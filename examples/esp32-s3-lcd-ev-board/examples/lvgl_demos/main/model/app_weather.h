/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// Definición de ubicaciones: solo Rosario (fallback) y una ubicación elegida por el usuario vía geocodificación
typedef enum {
    LOCATION_NUM_ROSARIO = 0,    // Rosario, Santa Fe (por defecto)
    LOCATION_NUM_USER,           // Ubicación elegida por el usuario (geocoding)
    LOCATION_NUM_MAX,
} weather_location_t;

// Geocoding structure para la ubicación elegida por el usuario
typedef struct {
    char country_code[4];   // e.g., "AR", "US", "ES"
    char city_name[64];     // Nombre de la ciudad
    float latitude;
    float longitude;
    bool is_valid;
} geocoding_location_t;

// Payload para request de geocoding (búsqueda ciudades)
typedef struct {
    char country_code[3]; // 2 + null
    char city[48];        // truncado seguro
} geocoding_search_request_t;

// Payload para evento de resultados de geocoding
typedef struct {
    int num_results;
    geocoding_location_t results[5];
} geocoding_search_result_evt_t;

// Weather type codes (WMO Weather interpretation codes)
typedef enum {
    WEATHER_CODE_CLEAR_SKY = 0,
    WEATHER_CODE_MAINLY_CLEAR = 1,
    WEATHER_CODE_PARTLY_CLOUDY = 2,
    WEATHER_CODE_OVERCAST = 3,
    WEATHER_CODE_FOG = 45,
    WEATHER_CODE_DEPOSITING_RIME_FOG = 48,
    WEATHER_CODE_LIGHT_DRIZZLE = 51,
    WEATHER_CODE_MODERATE_DRIZZLE = 53,
    WEATHER_CODE_DENSE_DRIZZLE = 55,
    WEATHER_CODE_LIGHT_RAIN = 61,
    WEATHER_CODE_MODERATE_RAIN = 63,
    WEATHER_CODE_HEAVY_RAIN = 65,
    WEATHER_CODE_LIGHT_SNOW = 71,
    WEATHER_CODE_MODERATE_SNOW = 73,
    WEATHER_CODE_HEAVY_SNOW = 75,
    WEATHER_CODE_LIGHT_RAIN_SHOWERS = 80,
    WEATHER_CODE_MODERATE_RAIN_SHOWERS = 81,
    WEATHER_CODE_VIOLENT_RAIN_SHOWERS = 82,
    WEATHER_CODE_THUNDERSTORM = 95,
    WEATHER_CODE_THUNDERSTORM_SLIGHT_HAIL = 96,
    WEATHER_CODE_THUNDERSTORM_HEAVY_HAIL = 99,
} weather_type_code_t;

typedef struct {
    char *temp;
} weather_hourly_info_t;

// Weather information structure
typedef struct {
    char location[64];
    char last_update[64];
    int32_t current_temp;
    int32_t current_humidity;
    weather_type_code_t current_code;
    char current_text[64];
    bool is_valid;
} weather_info_t;

// Function declarations
/**
 * @brief Initialize weather module
 * @return esp_err_t Initialization status
 */
esp_err_t app_weather_init(void);

/**
 * @brief Request weather data for a location
 * @param location Location to get weather for
 * @return esp_err_t Request status
 */
esp_err_t app_weather_request(weather_location_t location);

/**
 * @brief Search coordinates for cities using geocoding API
 * @param country_code Country code (e.g., "AR", "US", "ES")
 * @param result Output geocoding result
 * @return esp_err_t Geocoding status
 */
esp_err_t app_weather_geocoding_search_cities(const char* country_code, const char* city_name, 
                                             geocoding_location_t* results, int max_results, int* num_results);

/**
 * @brief Get weather info for a location
 * @param location Location to get weather for
 * @return weather_info_t* Pointer to weather info or NULL if invalid
 */
weather_info_t *app_weather_get_info(weather_location_t location);

/**
 * @brief Obtener ubicación geocodificada del usuario (si existe)
 * @return geocoding_location_t* Puntero a la ubicación o NULL si no está configurada
 */
geocoding_location_t *app_weather_get_custom_location(void);

/**
 * @brief Configurar ubicación del usuario desde resultado de geocoding
 * @param location Estructura geocodificada válida
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_weather_set_custom_location(const geocoding_location_t* location);

/**
 * @brief Parse Open-Meteo JSON response
 * @param json_string JSON response string
 * @param location Location index
 * @return esp_err_t Parse status
 */
esp_err_t app_weather_parse_openmeteo(const char *json_string, weather_location_t location);

/**
 * @brief Convert weather code to text description
 * @param code Weather code
 * @param text Output text buffer
 * @param text_size Size of text buffer
 */
void app_weather_code_to_text(weather_type_code_t code, char *text, size_t text_size);

#ifdef __cplusplus
}
#endif
