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
 * @brief Solicitar actualización del clima para ubicación personalizada
 * @param country_code Código del país (ej: "ES", "AR")
 * @param postal_code Código postal
 */
void ui_weather_bridge_request_custom_weather(const char* country_code, const char* postal_code);

/**
 * @brief Verificar y procesar actualizaciones pendientes del clima
 * @note Llamar desde el loop principal o tick
 */
void ui_weather_bridge_process_weather_updates(void);

/**
 * @brief Función para llamar cuando se entra a la pestaña de clima
 * @note Puedes llamar esto desde una acción en EEZ Studio
 */
void ui_weather_bridge_on_weather_tab_enter(void);

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
 * @brief Función para forzar actualización del clima (solo ubicaciones predefinidas)
 */
void ui_weather_bridge_force_weather_update(void);

/**
 * @brief Función para forzar actualización inmediata del clima
 */
void ui_weather_bridge_force_immediate_update(void);

#ifdef __cplusplus
}
#endif
