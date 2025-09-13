/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "ui_weather_bridge.h"
#include "app_weather.h"
#include "event_system.h"
#include "esp_log.h"
#include <string.h>

// Incluir headers de EEZ Studio para acceder a objects
#include "screens.h"
#include "vars.h"
#include "ui.h"
#include "images.h"

static const char *TAG = "ui_weather_bridge";

// Estado mínimo necesario
static bool periodic_update_enabled = true;
static weather_location_t current_location = LOCATION_NUM_ROSARIO; // Única ubicación (usuario puede cambiar en settings)

// Throttling para evitar updates demasiado frecuentes
static uint32_t last_update_time = 0;
#define MIN_UPDATE_INTERVAL_MS (60000)  // Minimum 1 minute between updates

void ui_weather_bridge_init(void)
{
    ESP_LOGI(TAG, "UI Weather Bridge initialized");

    // Forzar primera actualización usando la API pública (gestiona helper interno)
    ui_weather_bridge_update_weather_info(current_location);
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
    ESP_LOGD(TAG, "Updating weather icon for code: %d", weather_code);
    
    if (objects.icono_clima_dinamico == NULL) {
        ESP_LOGW(TAG, "Weather icon object not available");
        return;
    }
    
    const void* icon = ui_weather_bridge_get_weather_icon(weather_code);
    lv_image_set_src(objects.icono_clima_dinamico, icon);
    
    ESP_LOGD(TAG, "Weather icon updated successfully");
}

void ui_weather_bridge_update_temperature(int32_t temperature)
{
    ESP_LOGD(TAG, "Updating temperature: %d°C", (int)temperature);
    
    if (objects.etiqueta_temperatura == NULL) {
        ESP_LOGW(TAG, "Temperature label object not available");
        return;
    }
    
    char temp_text[32];
    snprintf(temp_text, sizeof(temp_text), "Temperatura: %d°C", (int)temperature);
    lv_label_set_text(objects.etiqueta_temperatura, temp_text);
    
    ESP_LOGD(TAG, "Temperature updated successfully");
}

void ui_weather_bridge_update_humidity(int32_t humidity)
{
    ESP_LOGD(TAG, "Updating humidity: %d%%", (int)humidity);
    
    if (objects.etiqueta_humedad == NULL) {
        ESP_LOGW(TAG, "Humidity label object not available");
        return;
    }
    
    char humidity_text[32];
    snprintf(humidity_text, sizeof(humidity_text), "Humedad: %d%%", (int)humidity);
    lv_label_set_text(objects.etiqueta_humedad, humidity_text);
    
    ESP_LOGD(TAG, "Humidity updated successfully");
}

void ui_weather_bridge_update_condition(const char* condition)
{
    ESP_LOGD(TAG, "Updating weather condition: %s", condition ? condition : "N/A");
    
    if (objects.etiqueta_condicion_ambiental == NULL) {
        ESP_LOGW(TAG, "Weather condition label object not available");
        return;
    }
    
    char condition_text[96];
    if (condition && strlen(condition) > 0) {
        snprintf(condition_text, sizeof(condition_text), "Condicion: %s", condition);
    } else {
        snprintf(condition_text, sizeof(condition_text), "Condicion: N/A");
    }
    
    lv_label_set_text(objects.etiqueta_condicion_ambiental, condition_text);
    
    ESP_LOGD(TAG, "Weather condition updated successfully");
}

void ui_weather_bridge_update_measurement_time(const char* last_update)
{
    ESP_LOGD(TAG, "Updating measurement time: %s", last_update ? last_update : "N/A");
    
    if (objects.etiqueta_hora_medicion == NULL) {
        ESP_LOGW(TAG, "Measurement time label object not available");
        return;
    }
    
    char time_text[96];
    if (last_update && strlen(last_update) > 0) {
        snprintf(time_text, sizeof(time_text), "Actualizado: %s", last_update);
    } else {
        snprintf(time_text, sizeof(time_text), "Actualizado: N/A");
    }
    
    lv_label_set_text(objects.etiqueta_hora_medicion, time_text);
    
    ESP_LOGD(TAG, "Measurement time updated successfully");
}

void ui_weather_bridge_update_location(const char* location)
{
    ESP_LOGD(TAG, "Updating location: %s", location ? location : "N/A");
    
    if (objects.label_ubicacion == NULL) {
        ESP_LOGW(TAG, "Location label object not available");
        return;
    }
    
    char location_text[96];
    if (location && strlen(location) > 0) {
        snprintf(location_text, sizeof(location_text), "Ubicacion: %s", location);
    } else {
        snprintf(location_text, sizeof(location_text), "Ubicacion: N/A");
    }
    
    lv_label_set_text(objects.label_ubicacion, location_text);
    
    ESP_LOGD(TAG, "Location updated successfully");
}

// Helper interno: refresca todos los widgets con la info de clima
static void ui_weather_bridge_update_weather_display(const weather_info_t* weather_info)
{
    if (weather_info == NULL) {
        ESP_LOGW(TAG, "NULL weather info provided");
        return;
    }

    if (!weather_info->is_valid) {
        ESP_LOGW(TAG, "Weather data is not valid");
        
        // Mostrar estado "sin datos" en UI
        if (objects.etiqueta_temperatura) {
            lv_label_set_text(objects.etiqueta_temperatura, "Temperatura: --°C");
        }
        if (objects.etiqueta_humedad) {
            lv_label_set_text(objects.etiqueta_humedad, "Humedad: --%");
        }
        if (objects.etiqueta_condicion_ambiental) {
            lv_label_set_text(objects.etiqueta_condicion_ambiental, "Condicion: Sin datos");
        }
        return;
    }
    
    ESP_LOGI(TAG, "Updating weather display: %s, %d°C, %d%%, %s", 
             weather_info->location, 
             weather_info->current_temp,
             weather_info->current_humidity,
             weather_info->current_text);
    
    // Actualizar todas las etiquetas del clima
    ui_weather_bridge_update_temperature(weather_info->current_temp);
    ui_weather_bridge_update_humidity(weather_info->current_humidity);
    ui_weather_bridge_update_condition(weather_info->current_text);
    ui_weather_bridge_update_measurement_time(weather_info->last_update);
    ui_weather_bridge_update_location(weather_info->location);
    ui_weather_bridge_update_weather_icon(weather_info->current_code);
    
    ESP_LOGI(TAG, "Weather display updated successfully");
}

void ui_weather_bridge_process_weather_updates(void)
{
    if (!periodic_update_enabled) {
        ESP_LOGD(TAG, "Periodic updates disabled, skipping");
        return;
    }

    // Obtener datos actuales del modelo
    weather_info_t *info = app_weather_get_info(current_location);
    if (info == NULL || !info->is_valid) {
        ESP_LOGD(TAG, "No valid weather data available for updates");
        return;
    }
    
    // Check throttling
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (last_update_time != 0 && (current_time - last_update_time) < MIN_UPDATE_INTERVAL_MS) {
        ESP_LOGD(TAG, "Weather update throttled, only %u ms since last update", 
                 current_time - last_update_time);
        return;
    }
    
    ESP_LOGD(TAG, "Processing periodic weather UI update");
    ui_weather_bridge_update_weather_display(info);
    last_update_time = current_time;
}

// Implementación requerida por header: actualizar info dado location
void ui_weather_bridge_update_weather_info(weather_location_t location)
{
    current_location = location; // actualizar ubicación activa
    weather_info_t *info = app_weather_get_info(location);
    if (info == NULL) {
        ESP_LOGW(TAG, "Weather info pointer NULL for location %d", location);
        return;
    }
    ui_weather_bridge_update_weather_display(info);
}

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

bool ui_weather_bridge_get_periodic_update(void)
{
    return periodic_update_enabled;
}

void ui_weather_bridge_force_immediate_update(void)
{
    ESP_LOGI(TAG, "Forcing immediate weather update via event system");
    
    // Enviar evento para que weather_task haga fetch inmediato (usar tipo existente)
    event_system_post(EVENT_WEATHER_UPDATE_REQUESTED, NULL, 0, EVENT_PRIORITY_HIGH);
}

// EEZ Studio Action Handlers - Conecta acciones UI con lógica
void ui_weather_action_refresh(lv_event_t *e)
{
    ESP_LOGI(TAG, "Weather refresh action triggered by user");
    ui_weather_bridge_force_immediate_update();
}

void ui_weather_action_toggle_updates(lv_event_t *e)
{
    bool current_state = ui_weather_bridge_get_periodic_update();
    ui_weather_bridge_set_periodic_update(!current_state);
    ESP_LOGI(TAG, "Weather updates toggled to: %s", !current_state ? "enabled" : "disabled");
}
