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
// Máximos heredados de settings.h (unificamos aquí)
#define APP_SETTINGS_MAX_WIFI_NETWORKS 3

typedef struct {
    char ssid[32];
    char password[64];
    uint8_t priority; // 0 = más alta
} app_settings_wifi_network_t;

typedef struct {
    // WiFi credenciales activas (para conexión inmediata)
    char wifi_ssid[64];
    char wifi_password[128];
    
    // Perfiles WiFi (fallback / servicio técnico)
    app_settings_wifi_network_t wifi_networks[APP_SETTINGS_MAX_WIFI_NETWORKS];
    uint8_t wifi_network_count;      // cantidad válida
    uint8_t wifi_current_network;    // índice activo

    // Clima / ubicación
    float weather_lat;
    float weather_lon;
    char weather_city[64];
    
    // Display / UI
    uint8_t display_brightness;  // 0-100
    char language[8];            // "es", "en", etc.
    bool demo_gui;               // heredado de sys_param_t
    bool need_hint;              // heredado de sys_param_t

    // Estado del sistema
    bool last_known_good_config;
} app_settings_t;

// Estructura para request de rotación de red (opcional futura)
typedef struct {
    uint8_t from_index;
    uint8_t to_index;
} app_settings_wifi_rotate_evt_t;

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

// Singleton global (lectura / escritura controlada)
app_settings_t* app_settings_get(void);
const app_settings_t* app_settings_get_const(void);

// Operaciones perfil WiFi unificado
esp_err_t app_settings_set_active_network(app_settings_t *settings, uint8_t index);
esp_err_t app_settings_rotate_next_network(app_settings_t *settings);
const char* app_settings_get_active_ssid(const app_settings_t *settings);
const char* app_settings_get_active_password(const app_settings_t *settings);

#ifdef __cplusplus
}
#endif