/*
 * Persistent settings management using NVS
 */

#include "app_settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "app_settings";
static nvs_handle_t settings_nvs_handle;

esp_err_t app_settings_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ret = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &settings_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Settings system initialized");
    return ESP_OK;
}

void app_settings_get_defaults(app_settings_t *settings)
{
    memset(settings, 0, sizeof(app_settings_t));
    
    // WiFi defaults (empty - user must configure)
    settings->wifi_ssid[0] = '\0';
    settings->wifi_password[0] = '\0';
    
    // Weather defaults (Rosario, Argentina)
    settings->weather_lat = -32.9442;
    settings->weather_lon = -60.6505;
    strcpy(settings->weather_city, "Rosario, Argentina");
    
    // Display defaults
    settings->display_brightness = 80;  // 80%
    strcpy(settings->language, "es");   // Spanish
    
    // System defaults
    settings->last_known_good_config = true;
}

esp_err_t app_settings_load(app_settings_t *settings)
{
    size_t required_size = 0;
    esp_err_t ret;

    // Start with defaults
    app_settings_get_defaults(settings);

    // Load WiFi SSID
    required_size = sizeof(settings->wifi_ssid);
    ret = nvs_get_str(settings_nvs_handle, KEY_WIFI_SSID, settings->wifi_ssid, &required_size);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Error reading wifi_ssid: %s", esp_err_to_name(ret));
    }

    // Load WiFi Password
    required_size = sizeof(settings->wifi_password);
    ret = nvs_get_str(settings_nvs_handle, KEY_WIFI_PASSWORD, settings->wifi_password, &required_size);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Error reading wifi_password: %s", esp_err_to_name(ret));
    }

    // Load weather coordinates
    ret = nvs_get_blob(settings_nvs_handle, KEY_WEATHER_LAT, &settings->weather_lat, &required_size);
    if (ret == ESP_OK) {
        required_size = sizeof(settings->weather_lon);
        nvs_get_blob(settings_nvs_handle, KEY_WEATHER_LON, &settings->weather_lon, &required_size);
    }

    // Load weather city
    required_size = sizeof(settings->weather_city);
    ret = nvs_get_str(settings_nvs_handle, KEY_WEATHER_CITY, settings->weather_city, &required_size);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Error reading weather_city: %s", esp_err_to_name(ret));
    }

    // Load display brightness
    ret = nvs_get_u8(settings_nvs_handle, KEY_DISPLAY_BRIGHTNESS, &settings->display_brightness);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Error reading brightness: %s", esp_err_to_name(ret));
    }

    // Load language
    required_size = sizeof(settings->language);
    ret = nvs_get_str(settings_nvs_handle, KEY_LANGUAGE, settings->language, &required_size);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Error reading language: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "Settings loaded: WiFi=%s, Weather=%s (%.4f,%.4f), Brightness=%d, Lang=%s", 
             settings->wifi_ssid, settings->weather_city, 
             settings->weather_lat, settings->weather_lon,
             settings->display_brightness, settings->language);

    return ESP_OK;
}

esp_err_t app_settings_save(const app_settings_t *settings)
{
    esp_err_t ret = ESP_OK;

    // Save WiFi settings
    if (strlen(settings->wifi_ssid) > 0) {
        ret |= nvs_set_str(settings_nvs_handle, KEY_WIFI_SSID, settings->wifi_ssid);
        ret |= nvs_set_str(settings_nvs_handle, KEY_WIFI_PASSWORD, settings->wifi_password);
    }

    // Save weather settings
    ret |= nvs_set_blob(settings_nvs_handle, KEY_WEATHER_LAT, &settings->weather_lat, sizeof(float));
    ret |= nvs_set_blob(settings_nvs_handle, KEY_WEATHER_LON, &settings->weather_lon, sizeof(float));
    ret |= nvs_set_str(settings_nvs_handle, KEY_WEATHER_CITY, settings->weather_city);

    // Save display settings
    ret |= nvs_set_u8(settings_nvs_handle, KEY_DISPLAY_BRIGHTNESS, settings->display_brightness);
    ret |= nvs_set_str(settings_nvs_handle, KEY_LANGUAGE, settings->language);

    // Commit changes
    ret |= nvs_commit(settings_nvs_handle);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Settings saved successfully");
    } else {
        ESP_LOGE(TAG, "Error saving settings: %s", esp_err_to_name(ret));
    }

    return ret;
}

esp_err_t app_settings_save_wifi(const char* ssid, const char* password)
{
    esp_err_t ret = ESP_OK;
    
    ret |= nvs_set_str(settings_nvs_handle, KEY_WIFI_SSID, ssid);
    ret |= nvs_set_str(settings_nvs_handle, KEY_WIFI_PASSWORD, password);
    ret |= nvs_commit(settings_nvs_handle);

    ESP_LOGI(TAG, "WiFi credentials saved: %s", ssid);
    return ret;
}

esp_err_t app_settings_save_weather_location(float lat, float lon, const char* city)
{
    esp_err_t ret = ESP_OK;
    
    ret |= nvs_set_blob(settings_nvs_handle, KEY_WEATHER_LAT, &lat, sizeof(float));
    ret |= nvs_set_blob(settings_nvs_handle, KEY_WEATHER_LON, &lon, sizeof(float));
    ret |= nvs_set_str(settings_nvs_handle, KEY_WEATHER_CITY, city);
    ret |= nvs_commit(settings_nvs_handle);

    ESP_LOGI(TAG, "Weather location saved: %s (%.4f, %.4f)", city, lat, lon);
    return ret;
}

esp_err_t app_settings_save_brightness(uint8_t brightness)
{
    esp_err_t ret = nvs_set_u8(settings_nvs_handle, KEY_DISPLAY_BRIGHTNESS, brightness);
    ret |= nvs_commit(settings_nvs_handle);

    ESP_LOGI(TAG, "Display brightness saved: %d%%", brightness);
    return ret;
}
