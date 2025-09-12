/*
 * Persistent settings management using NVS
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// NVS namespace for app settings
#define SETTINGS_NAMESPACE "app_settings"

// Settings keys
#define KEY_WIFI_SSID           "wifi_ssid"
#define KEY_WIFI_PASSWORD       "wifi_pass"
#define KEY_WEATHER_LAT         "weather_lat"
#define KEY_WEATHER_LON         "weather_lon"
#define KEY_WEATHER_CITY        "weather_city"
#define KEY_DISPLAY_BRIGHTNESS  "brightness"
#define KEY_LANGUAGE            "language"
#define KEY_LAST_GOOD_CONFIG    "last_config"

// Settings structure
typedef struct {
    char wifi_ssid[64];
    char wifi_password[128];
    float weather_lat;
    float weather_lon;
    char weather_city[64];
    uint8_t display_brightness;  // 0-100
    char language[8];            // "es", "en", etc.
    bool last_known_good_config;
} app_settings_t;

/**
 * @brief Initialize settings system
 * @return esp_err_t
 */
esp_err_t app_settings_init(void);

/**
 * @brief Load all settings from NVS
 * @param settings Pointer to settings structure to fill
 * @return esp_err_t
 */
esp_err_t app_settings_load(app_settings_t *settings);

/**
 * @brief Save all settings to NVS
 * @param settings Pointer to settings structure to save
 * @return esp_err_t
 */
esp_err_t app_settings_save(const app_settings_t *settings);

/**
 * @brief Save WiFi credentials
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return esp_err_t
 */
esp_err_t app_settings_save_wifi(const char* ssid, const char* password);

/**
 * @brief Save weather location
 * @param lat Latitude
 * @param lon Longitude
 * @param city City name
 * @return esp_err_t
 */
esp_err_t app_settings_save_weather_location(float lat, float lon, const char* city);

/**
 * @brief Save display brightness
 * @param brightness Brightness level (0-100)
 * @return esp_err_t
 */
esp_err_t app_settings_save_brightness(uint8_t brightness);

/**
 * @brief Get default settings
 * @param settings Pointer to settings structure to fill with defaults
 */
void app_settings_get_defaults(app_settings_t *settings);

#ifdef __cplusplus
}
#endif