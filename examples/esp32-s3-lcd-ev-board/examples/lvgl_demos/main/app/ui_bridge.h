/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include "lvgl.h"
#include "app_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializar el puente UI-WiFi
 */
void ui_bridge_init(void);

/**
 * @brief Actualizar la lista de redes WiFi en la UI
 * @note Debe llamarse cuando el escaneo WiFi esté completo
 */
void ui_bridge_update_wifi_list(void);

/**
 * @brief Disparar escaneo de redes WiFi
 */
void ui_bridge_start_wifi_scan(void);

/**
 * @brief Manejar selección de red WiFi
 * @param ap_index Índice del access point seleccionado
 */
void ui_bridge_select_wifi_network(int ap_index);

/**
 * @brief Verificar si hay actualizaciones pendientes de WiFi
 * @note Llamar desde el loop principal o tick
 */
void ui_bridge_process_wifi_updates(void);

/**
 * @brief Función para llamar cuando se entra a la pantalla de configuración
 * @note Puedes llamar esto desde una acción en EEZ Studio
 */
void ui_bridge_on_config_screen_enter(void);

#ifdef __cplusplus
}
#endif
