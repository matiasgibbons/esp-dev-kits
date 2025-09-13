/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include "lvgl.h"
#include "app_weather.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializar el puente UI-Weather
 */
void ui_weather_bridge_init(void);

/**
 * @brief Actualizar la información del clima en la UI
 * @param location Ubicación del clima a actualizar
 */
void ui_weather_bridge_update_weather_info(weather_location_t location);

/**
 * @brief Actualizar el icono del clima dinámicamente
 * @param weather_code Código del clima WMO
 */
void ui_weather_bridge_update_weather_icon(weather_type_code_t weather_code);

/**
 * @brief Actualizar temperatura mostrada en la UI
 * @param temperature Temperatura en grados Celsius
 */
void ui_weather_bridge_update_temperature(int32_t temperature);

/**
 * @brief Actualizar humedad mostrada en la UI
 * @param humidity Humedad relativa en porcentaje
 */
void ui_weather_bridge_update_humidity(int32_t humidity);

/**
 * @brief Actualizar condición ambiental mostrada en la UI
 * @param condition Descripción textual del clima
 */
void ui_weather_bridge_update_condition(const char* condition);

/**
 * @brief Actualizar hora de medición mostrada en la UI
 * @param last_update Timestamp de la última actualización
 */
void ui_weather_bridge_update_measurement_time(const char* last_update);

/**
 * @brief Actualizar ubicación mostrada en la UI
 * @param location Nombre de la ubicación
 */
void ui_weather_bridge_update_location(const char* location);

/**
 * @brief Verificar y procesar actualizaciones pendientes del clima
 * @note Llamar desde el loop principal o tick
 */
void ui_weather_bridge_process_weather_updates(void);

/**
 * @brief Obtener el icono del clima según el código WMO
 * @param weather_code Código del clima WMO
 * @return Puntero a la imagen del icono
 */
const void* ui_weather_bridge_get_weather_icon(weather_type_code_t weather_code);

/**
 * @brief Habilitar/deshabilitar actualización periódica del clima
 * @param enabled true para habilitar, false para deshabilitar
 */
void ui_weather_bridge_set_periodic_update(bool enabled);

/**
 * @brief Obtener el estado de actualización periódica
 * @return true si está habilitada, false si está deshabilitada
 */
bool ui_weather_bridge_get_periodic_update(void);

/**
 * @brief Función para forzar actualización inmediata del clima
 */
void ui_weather_bridge_force_immediate_update(void);

#ifdef __cplusplus
}
#endif
