/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "app_weather.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "app_weather";

#define MAX_HTTP_RECV_BUFFER            (2048)
#define _UA_                            "ESP32-S3-Weather-Client"

// Open-Meteo API base URL (free, no API key required!)
#define OPENMETEO_BASE_URL              "http://api.open-meteo.com/v1/forecast"

// Coordinates for major cities
typedef struct {
    const char* name;
    const char* lat;
    const char* lon;
} city_coords_t;

static const city_coords_t cities[] = {
    {"Shanghai", "31.2304", "121.4737"},
    {"Beijing", "39.9042", "116.4074"},
    {"Shenzhen", "22.5431", "114.0579"}
};

// Weather data storage
static weather_info_t weather_data[LOCATION_NUM_MAX];

// HTTP event handler
esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer = NULL;
    static int output_len = 0;
    
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // Store received data
        if (output_buffer == NULL) {
            output_buffer = (char *) malloc(MAX_HTTP_RECV_BUFFER);
            output_len = 0;
        }
        if (output_len + evt->data_len < MAX_HTTP_RECV_BUFFER - 1) {
            memcpy(output_buffer + output_len, evt->data, evt->data_len);
            output_len += evt->data_len;
            output_buffer[output_len] = '\0';
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL) {
            ESP_LOGI(TAG, "Received weather data: %s", output_buffer);
            // Parse the JSON response here
            app_weather_parse_openmeteo(output_buffer, (weather_location_t)(evt->user_data));
            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
        }
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        if (output_buffer != NULL) {
            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

// Parse Open-Meteo JSON response
esp_err_t app_weather_parse_openmeteo(const char *json_string, weather_location_t location)
{
    if (location >= LOCATION_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid location: %d", location);
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_ERR_INVALID_ARG;
    }

    weather_info_t *info = &weather_data[location];
    memset(info, 0, sizeof(weather_info_t));

    // Get current weather data
    cJSON *current = cJSON_GetObjectItem(json, "current");
    if (current != NULL) {
        cJSON *temp = cJSON_GetObjectItem(current, "temperature_2m");
        if (temp != NULL && cJSON_IsNumber(temp)) {
            info->current_temp = (int32_t)temp->valuedouble;
        }

        cJSON *humidity = cJSON_GetObjectItem(current, "relative_humidity_2m");
        if (humidity != NULL && cJSON_IsNumber(humidity)) {
            info->current_humidity = (int32_t)humidity->valuedouble;
        }

        cJSON *weather_code = cJSON_GetObjectItem(current, "weather_code");
        if (weather_code != NULL && cJSON_IsNumber(weather_code)) {
            info->current_code = (weather_type_code_t)weather_code->valueint;
        }

        // Convert weather code to text description
        app_weather_code_to_text(info->current_code, info->current_text, sizeof(info->current_text));
    }

    // Set location name and mark as valid
    strncpy(info->location, cities[location].name, sizeof(info->location) - 1);
    info->is_valid = true;
    
    ESP_LOGI(TAG, "Weather updated for %s: %d°C, %s", 
             info->location, info->current_temp, info->current_text);

    cJSON_Delete(json);
    return ESP_OK;
}

// Convert weather code to text description
void app_weather_code_to_text(weather_type_code_t code, char *text, size_t text_size)
{
    // WMO Weather interpretation codes
    switch (code) {
        case 0:
            strncpy(text, "Clear sky", text_size - 1);
            break;
        case 1: case 2: case 3:
            strncpy(text, "Partly cloudy", text_size - 1);
            break;
        case 45: case 48:
            strncpy(text, "Fog", text_size - 1);
            break;
        case 51: case 53: case 55:
            strncpy(text, "Drizzle", text_size - 1);
            break;
        case 61: case 63: case 65:
            strncpy(text, "Rain", text_size - 1);
            break;
        case 71: case 73: case 75:
            strncpy(text, "Snow", text_size - 1);
            break;
        case 80: case 81: case 82:
            strncpy(text, "Rain showers", text_size - 1);
            break;
        case 95: case 96: case 99:
            strncpy(text, "Thunderstorm", text_size - 1);
            break;
        default:
            strncpy(text, "Unknown", text_size - 1);
            break;
    }
    text[text_size - 1] = '\0';
}

// Fetch weather data from Open-Meteo API
esp_err_t app_weather_request(weather_location_t location)
{
    if (location >= LOCATION_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid location: %d", location);
        return ESP_ERR_INVALID_ARG;
    }

    char url[512];
    snprintf(url, sizeof(url), 
             "%s?latitude=%s&longitude=%s&current=temperature_2m,relative_humidity_2m,weather_code&timezone=auto",
             OPENMETEO_BASE_URL,
             cities[location].lat,
             cities[location].lon);

    ESP_LOGI(TAG, "Requesting weather for %s: %s", cities[location].name, url);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .user_data = (void*)location,
        .is_async = false,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    } else {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP Status = %d", status_code);
    }

    esp_http_client_cleanup(client);
    return err;
}

// Get weather info for a location
weather_info_t *app_weather_get_info(weather_location_t location)
{
    if (location >= LOCATION_NUM_MAX) {
        return NULL;
    }
    return &weather_data[location];
}

// Initialize weather module
esp_err_t app_weather_init(void)
{
    ESP_LOGI(TAG, "Initializing weather module with Open-Meteo API");
    memset(weather_data, 0, sizeof(weather_data));
    return ESP_OK;
}
