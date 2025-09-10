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

// Open-Meteo API base URLs
#define OPENMETEO_BASE_URL              "http://api.open-meteo.com/v1/forecast"
#define OPENMETEO_GEOCODING_URL         "http://geocoding-api.open-meteo.com/v1/search"

// Coordinates for major cities
typedef struct {
    const char* name;
    const char* lat;
    const char* lon;
} city_coords_t;

static const city_coords_t cities[] = {
    {"Rosario", "-32.9468", "-60.6393"},           // Rosario, Santa Fe
    {"Buenos Aires", "-34.6118", "-58.3960"},      // Buenos Aires, CABA  
    {"Córdoba", "-31.4135", "-64.1811"}            // Córdoba, Córdoba
};

// Weather data storage
static weather_info_t weather_data[LOCATION_NUM_MAX];
static geocoding_location_t custom_location;

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
    if (location == LOCATION_NUM_CUSTOM && custom_location.is_valid) {
        snprintf(info->location, sizeof(info->location), "%s (%s-%s)", 
                 custom_location.city_name, custom_location.country_code, custom_location.postal_code);
    } else {
        strncpy(info->location, cities[location].name, sizeof(info->location) - 1);
    }
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
    const char* city_name;
    const char* lat;
    const char* lon;

    if (location == LOCATION_NUM_CUSTOM && custom_location.is_valid) {
        // Use custom geocoded location
        city_name = custom_location.city_name;
        snprintf(url, sizeof(url), 
                 "%s?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m,weather_code&timezone=auto",
                 OPENMETEO_BASE_URL,
                 custom_location.latitude,
                 custom_location.longitude);
    } else {
        // Use predefined cities
        city_name = cities[location].name;
        lat = cities[location].lat;
        lon = cities[location].lon;
        snprintf(url, sizeof(url), 
                 "%s?latitude=%s&longitude=%s&current=temperature_2m,relative_humidity_2m,weather_code&timezone=auto",
                 OPENMETEO_BASE_URL, lat, lon);
    }

    ESP_LOGI(TAG, "Requesting weather for %s: %s", city_name, url);

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

// Get current custom geocoded location info
geocoding_location_t *app_weather_get_custom_location(void)
{
    return custom_location.is_valid ? &custom_location : NULL;
}

// Initialize weather module
esp_err_t app_weather_init(void)
{
    ESP_LOGI(TAG, "Initializing weather module with Open-Meteo API");
    memset(weather_data, 0, sizeof(weather_data));
    memset(&custom_location, 0, sizeof(custom_location));
    return ESP_OK;
}

// Parse geocoding JSON response
esp_err_t app_weather_parse_geocoding(const char *json_string, geocoding_location_t* result)
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse geocoding JSON");
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *results = cJSON_GetObjectItem(json, "results");
    if (results == NULL || !cJSON_IsArray(results)) {
        ESP_LOGE(TAG, "No results in geocoding response");
        cJSON_Delete(json);
        return ESP_ERR_NOT_FOUND;
    }

    int array_size = cJSON_GetArraySize(results);
    if (array_size == 0) {
        ESP_LOGE(TAG, "Empty results array in geocoding response");
        cJSON_Delete(json);
        return ESP_ERR_NOT_FOUND;
    }

    // Get first result
    cJSON *first_result = cJSON_GetArrayItem(results, 0);
    if (first_result == NULL) {
        ESP_LOGE(TAG, "Failed to get first geocoding result");
        cJSON_Delete(json);
        return ESP_ERR_INVALID_ARG;
    }

    // Extract data
    cJSON *name = cJSON_GetObjectItem(first_result, "name");
    cJSON *latitude = cJSON_GetObjectItem(first_result, "latitude");
    cJSON *longitude = cJSON_GetObjectItem(first_result, "longitude");

    if (name && cJSON_IsString(name) && latitude && cJSON_IsNumber(latitude) && longitude && cJSON_IsNumber(longitude)) {
        strncpy(result->city_name, name->valuestring, sizeof(result->city_name) - 1);
        result->latitude = (float)latitude->valuedouble;
        result->longitude = (float)longitude->valuedouble;
        result->is_valid = true;
        
        ESP_LOGI(TAG, "Geocoding found: %s (%.4f, %.4f)", 
                 result->city_name, result->latitude, result->longitude);
    } else {
        ESP_LOGE(TAG, "Invalid geocoding result format");
        cJSON_Delete(json);
        return ESP_ERR_INVALID_ARG;
    }

    cJSON_Delete(json);
    return ESP_OK;
}

// HTTP event handler for geocoding
esp_err_t http_geocoding_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer = NULL;
    static int output_len = 0;
    
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR (geocoding)");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED (geocoding)");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT (geocoding)");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER (geocoding), key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA (geocoding), len=%d", evt->data_len);
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
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH (geocoding)");
        if (output_buffer != NULL) {
            ESP_LOGI(TAG, "Received geocoding data: %s", output_buffer);
            // Parse the JSON response
            geocoding_location_t* result = (geocoding_location_t*)evt->user_data;
            app_weather_parse_geocoding(output_buffer, result);
            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
        }
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED (geocoding)");
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

// Search coordinates for postal code using geocoding API
esp_err_t app_weather_geocoding_search(const char* country_code, const char* postal_code, geocoding_location_t* result)
{
    if (!country_code || !postal_code || !result) {
        ESP_LOGE(TAG, "Invalid parameters for geocoding search");
        return ESP_ERR_INVALID_ARG;
    }

    // Store input parameters
    strncpy(result->country_code, country_code, sizeof(result->country_code) - 1);
    strncpy(result->postal_code, postal_code, sizeof(result->postal_code) - 1);
    result->is_valid = false;

    char url[512];
    snprintf(url, sizeof(url), 
             "%s?name=%s&countryCode=%s&count=1&language=es&format=json",
             OPENMETEO_GEOCODING_URL,
             postal_code,
             country_code);

    ESP_LOGI(TAG, "Geocoding search for %s-%s: %s", country_code, postal_code, url);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_geocoding_event_handler,
        .user_data = result,
        .is_async = false,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client for geocoding");
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Geocoding HTTP request failed: %s", esp_err_to_name(err));
    } else {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Geocoding HTTP Status = %d", status_code);
    }

    esp_http_client_cleanup(client);
    return err;
}

// Request weather data for custom location using geocoding
esp_err_t app_weather_request_geocoded(const char* country_code, const char* postal_code)
{
    if (!country_code || !postal_code) {
        ESP_LOGE(TAG, "Invalid parameters for geocoded weather request");
        return ESP_ERR_INVALID_ARG;
    }

    // First, get coordinates using geocoding
    esp_err_t err = app_weather_geocoding_search(country_code, postal_code, &custom_location);
    if (err != ESP_OK || !custom_location.is_valid) {
        ESP_LOGE(TAG, "Failed to geocode location %s-%s", country_code, postal_code);
        return err;
    }

    // Now request weather using the found coordinates
    char url[512];
    snprintf(url, sizeof(url), 
             "%s?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m,weather_code&timezone=auto",
             OPENMETEO_BASE_URL,
             custom_location.latitude,
             custom_location.longitude);

    ESP_LOGI(TAG, "Requesting weather for %s (%s-%s): %s", 
             custom_location.city_name, country_code, postal_code, url);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .user_data = (void*)LOCATION_NUM_CUSTOM,
        .is_async = false,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client for weather");
        return ESP_ERR_NO_MEM;
    }

    err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Weather HTTP request failed: %s", esp_err_to_name(err));
    } else {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Weather HTTP Status = %d", status_code);
    }

    esp_http_client_cleanup(client);
    return err;
}
