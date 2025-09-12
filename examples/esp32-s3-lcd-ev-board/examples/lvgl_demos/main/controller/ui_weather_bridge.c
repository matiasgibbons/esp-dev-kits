/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "ui_weather_bridge.h"
#include "app_weather.h"
#include "esp_log.h"
#include <string.h>

// Incluir headers de EEZ Studio para acceder a objects
#include "screens.h"
#include "vars.h"
#include "ui.h"
#include "images.h"

static const char *TAG = "ui_weather_bridge";

// Flags para indicar actualizaciones pendientes
static bool weather_needs_update = false;
static bool weather_icon_needs_update = false;
static bool weather_temp_needs_update = false;

// Tracking de pantalla/tab actual
static bool weather_tab_active = false;
static bool weather_tab_entered = false;

// Datos del clima actual
static weather_type_code_t current_weather_code = 0;
static int32_t current_temperature = 0;
static weather_location_t current_location = LOCATION_NUM_ROSARIO;

// Estado de actualización periódica (activado/desactivado)
static bool periodic_update_enabled = true;

void ui_weather_bridge_init(void)
{
    ESP_LOGI(TAG, "UI Weather Bridge initialized");
    weather_needs_update = false;
    weather_icon_needs_update = false;
    weather_temp_needs_update = false;
    weather_tab_active = false;
    weather_tab_entered = false;
}

const void* ui_weather_bridge_get_weather_icon(weather_type_code_t weather_code)
{
    // Mapeo de códigos WMO a iconos disponibles
    switch (weather_code) {
        case 0: // Clear sky
            return &img_wh_clear_day;
        case 1: case 2: case 3: // Partly cloudy
            return &img_wh_partly_cloudy_day;
        case 45: case 48: // Fog
            return &img_wh_fog;
        case 51: case 53: case 55: // Drizzle
            return &img_wh_drizzle;
        case 61: case 63: case 65: // Rain
            return &img_wh_rain;
        case 71: case 73: case 75: // Snow
            return &img_wh_snow;
        case 80: case 81: case 82: // Rain showers
            return &img_wh_rain;
        case 95: case 96: case 99: // Thunderstorm
            return &img_wh_thunderstorms;
        default:
            return &img_wh_cloudy; // Default cloudy icon
    }
}

void ui_weather_bridge_update_weather_icon(weather_type_code_t weather_code)
{
    ESP_LOGI(TAG, "Updating weather icon for code: %d", weather_code);
    
    // Verificar que el objeto icono_clima_dinamico existe
    if (objects.icono_clima_dinamico == NULL) {
        ESP_LOGW(TAG, "Weather icon object not available");
        return;
    }
    
    const void* icon = ui_weather_bridge_get_weather_icon(weather_code);
    lv_image_set_src(objects.icono_clima_dinamico, icon);
    
    current_weather_code = weather_code;
    weather_icon_needs_update = false;
    
    ESP_LOGI(TAG, "Weather icon updated successfully");
}

void ui_weather_bridge_update_temperature(int32_t temperature)
{
    ESP_LOGI(TAG, "Updating temperature: %d°C", (int)temperature);
    
    // Verificar que el objeto etiqueta_temperatura existe
    if (objects.etiqueta_temperatura == NULL) {
        ESP_LOGW(TAG, "Temperature label object not available");
        return;
    }
    
    // Formatear temperatura
    char temp_text[32];
    snprintf(temp_text, sizeof(temp_text), "Temperatura: %d°C", (int)temperature);
    
    lv_label_set_text(objects.etiqueta_temperatura, temp_text);
    
    current_temperature = temperature;
    weather_temp_needs_update = false;
    
    ESP_LOGI(TAG, "Temperature updated successfully");
}

void ui_weather_bridge_update_humidity(int32_t humidity)
{
    ESP_LOGI(TAG, "Updating humidity: %d%%", (int)humidity);
    
    // Verificar que el objeto etiqueta_humedad existe
    if (objects.etiqueta_humedad == NULL) {
        ESP_LOGW(TAG, "Humidity label object not available");
        return;
    }
    
    // Formatear humedad
    char humidity_text[32];
    snprintf(humidity_text, sizeof(humidity_text), "Humedad: %d%%", (int)humidity);
    
    lv_label_set_text(objects.etiqueta_humedad, humidity_text);
    
    ESP_LOGI(TAG, "Humidity updated successfully");
}

void ui_weather_bridge_update_condition(const char* condition)
{
    ESP_LOGI(TAG, "Updating weather condition: %s", condition ? condition : "N/A");
    
    // Verificar que el objeto etiqueta_condicion_ambiental existe
    if (objects.etiqueta_condicion_ambiental == NULL) {
        ESP_LOGW(TAG, "Weather condition label object not available");
        return;
    }
    
    // Formatear condición ambiental
    char condition_text[96];
    if (condition && strlen(condition) > 0) {
        snprintf(condition_text, sizeof(condition_text), "Condicion: %s", condition);
    } else {
        snprintf(condition_text, sizeof(condition_text), "Condicion: N/A");
    }
    
    lv_label_set_text(objects.etiqueta_condicion_ambiental, condition_text);
    
    ESP_LOGI(TAG, "Weather condition updated successfully");
}

void ui_weather_bridge_update_measurement_time(const char* last_update)
{
    ESP_LOGI(TAG, "Updating measurement time: %s", last_update ? last_update : "N/A");
    
    // Verificar que el objeto etiqueta_hora_medicion existe
    if (objects.etiqueta_hora_medicion == NULL) {
        ESP_LOGW(TAG, "Measurement time label object not available");
        return;
    }
    
    // Formatear hora de medición
    char time_text[96];
    if (last_update && strlen(last_update) > 0) {
        snprintf(time_text, sizeof(time_text), "Actualizado: %s", last_update);
    } else {
        snprintf(time_text, sizeof(time_text), "Actualizado: N/A");
    }
    
    lv_label_set_text(objects.etiqueta_hora_medicion, time_text);
    
    ESP_LOGI(TAG, "Measurement time updated successfully");
}

void ui_weather_bridge_update_location(const char* location)
{
    ESP_LOGI(TAG, "Updating location: %s", location ? location : "N/A");
    
    // Verificar que el objeto label_ubicacion existe
    if (objects.label_ubicacion == NULL) {
        ESP_LOGW(TAG, "Location label object not available");
        return;
    }
    
    // Formatear ubicación
    char location_text[96];
    if (location && strlen(location) > 0) {
        snprintf(location_text, sizeof(location_text), "Ubicacion: %s", location);
    } else {
        snprintf(location_text, sizeof(location_text), "Ubicacion: N/A");
    }
    
    lv_label_set_text(objects.label_ubicacion, location_text);
    
    ESP_LOGI(TAG, "Location updated successfully");
}

void ui_weather_bridge_update_weather_info(weather_location_t location)
{
    ESP_LOGI(TAG, "Updating weather info for location: %d", location);
    
    // Reset the update flag immediately to prevent infinite loops
    weather_needs_update = false;
    
    // Obtener información del clima
    weather_info_t *weather_info = app_weather_get_info(location);
    if (weather_info == NULL || !weather_info->is_valid) {
        ESP_LOGW(TAG, "No valid weather data for location %d", location);
        return;
    }
    
    // Actualizar todas las etiquetas del clima
    ui_weather_bridge_update_temperature(weather_info->current_temp);
    ui_weather_bridge_update_humidity(weather_info->current_humidity);
    ui_weather_bridge_update_condition(weather_info->current_text);
    ui_weather_bridge_update_measurement_time(weather_info->last_update);
    ui_weather_bridge_update_location(weather_info->location);
    
    // Actualizar icono
    ui_weather_bridge_update_weather_icon(weather_info->current_code);
    
    current_location = location;
    
    ESP_LOGI(TAG, "Weather info updated: %s, %d°C, %d%%, %s", 
             weather_info->location, 
             weather_info->current_temp,
             weather_info->current_humidity,
             weather_info->current_text);
}

void ui_weather_bridge_request_custom_weather(const char* country_code, const char* postal_code)
{
    if (!country_code || !postal_code) {
        ESP_LOGE(TAG, "Invalid parameters for custom weather request");
        return;
    }
    
    ESP_LOGI(TAG, "Requesting custom weather for %s-%s", country_code, postal_code);
    
    // Solicitar clima usando geocodificación
    esp_err_t ret = app_weather_request_geocoded(country_code, postal_code);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to request custom weather: %s", esp_err_to_name(ret));
    } else {
        // Marcar que necesitamos actualizar con ubicación personalizada
        current_location = LOCATION_NUM_CUSTOM;
        weather_needs_update = true;
    }
}

void ui_weather_bridge_process_weather_updates(void)
{
    // Detectar si estamos en el tab de clima verificando si el objeto existe y está visible
    bool in_weather_tab = false;
    
    if (objects.tab_clima != NULL) {
        lv_obj_t* active_screen = lv_scr_act();
        // Verificar si estamos en la pantalla config y el tab clima está activo
        in_weather_tab = (active_screen == objects.config) && 
                        (lv_obj_has_state(objects.tab_clima, LV_STATE_CHECKED) || 
                         lv_obj_get_parent(objects.tab_clima) != NULL);
    }
    
    // Si entramos al tab de clima
    if (in_weather_tab && !weather_tab_entered) {
        ESP_LOGI(TAG, "Entered Weather tab - requesting weather update");
        weather_tab_entered = true;
        ui_weather_bridge_on_weather_tab_enter();
    } else if (!in_weather_tab) {
        weather_tab_entered = false;
    }
    
    weather_tab_active = in_weather_tab;
    
    // Procesar actualizaciones pendientes solo si estamos en el tab activo
    if (weather_tab_active) {
        if (weather_needs_update) {
            ui_weather_bridge_update_weather_info(current_location);
        }
        
        if (weather_icon_needs_update) {
            ui_weather_bridge_update_weather_icon(current_weather_code);
        }
        
        if (weather_temp_needs_update) {
            ui_weather_bridge_update_temperature(current_temperature);
        }
    }
}

void ui_weather_bridge_on_weather_tab_enter(void)
{
    ESP_LOGI(TAG, "Weather tab entered manually - requesting weather update");
    
    // Verificar si hay datos válidos de ubicación personalizada
    geocoding_location_t *custom_loc = app_weather_get_custom_location();
    if (custom_loc != NULL && custom_loc->is_valid) {
        // Actualizar clima para ubicación personalizada
        current_location = LOCATION_NUM_CUSTOM;
        weather_needs_update = true;
    } else {
        // Actualizar clima para ubicación por defecto (Rosario)
        current_location = LOCATION_NUM_ROSARIO;
        
        // Solicitar datos del clima si no los tenemos
        weather_info_t *weather_info = app_weather_get_info(current_location);
        if (weather_info == NULL || !weather_info->is_valid) {
            esp_err_t ret = app_weather_request(current_location);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to request weather: %s", esp_err_to_name(ret));
            }
        }
        weather_needs_update = true;
    }
}

// Función auxiliar para manejar cambios en el código postal desde la UI
void ui_weather_bridge_on_postal_code_change(const char* postal_code)
{
    if (!postal_code || strlen(postal_code) == 0) {
        ESP_LOGW(TAG, "Empty postal code");
        return;
    }
    
    // Obtener el país seleccionado del dropdown
    if (objects.pais == NULL) {
        ESP_LOGW(TAG, "Country dropdown not available");
        return;
    }
    
    // Por simplicidad, asumimos España como país por defecto
    // En una implementación más completa, obtendrías esto del dropdown
    const char* country_code = "ES";
    
    ESP_LOGI(TAG, "Postal code changed to: %s", postal_code);
    ui_weather_bridge_request_custom_weather(country_code, postal_code);
}

// Función para habilitar/deshabilitar actualización periódica
void ui_weather_bridge_set_periodic_update(bool enabled)
{
    periodic_update_enabled = enabled;
    ESP_LOGI(TAG, "Periodic weather update %s", enabled ? "enabled" : "disabled");
    
    // Actualizar el switch de la UI si está disponible
    if (objects.actualizacion_clima != NULL) {
        if (lv_obj_get_state(objects.actualizacion_clima) != enabled) {
            if (enabled) {
                lv_obj_add_state(objects.actualizacion_clima, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.actualizacion_clima, LV_STATE_CHECKED);
            }
        }
    }
}

// Función para obtener el estado de actualización periódica
bool ui_weather_bridge_get_periodic_update(void)
{
    return periodic_update_enabled;
}

// Función para forzar actualización del clima (solo para ubicaciones predefinidas)
void ui_weather_bridge_force_weather_update(void)
{
    ESP_LOGI(TAG, "Forcing weather update for predefined location");
    
    // Solo actualizar ubicaciones predefinidas, no manejar ubicaciones personalizadas
    if (current_location != LOCATION_NUM_CUSTOM) {
        esp_err_t ret = app_weather_request(current_location);
        if (ret == ESP_OK) {
            weather_needs_update = true;
        }
    } else {
        ESP_LOGW(TAG, "Cannot force update for custom location - use geocoding bridge instead");
    }
}

void ui_weather_bridge_force_immediate_update(void)
{
    ESP_LOGI(TAG, "Forcing immediate weather update");
    
    // Forzar las flags de actualización
    weather_needs_update = true;
    weather_icon_needs_update = true;
    weather_temp_needs_update = true;
}
