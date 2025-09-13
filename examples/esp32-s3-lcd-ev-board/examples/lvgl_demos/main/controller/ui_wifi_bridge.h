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
void ui_wifi_bridge_init(void);

/**
 * @brief Actualizar la lista de redes WiFi en la UI
 * @note Debe llamarse cuando el escaneo WiFi esté completo
 */
void ui_wifi_bridge_update_wifi_list(void);

/**
 * @brief Disparar escaneo de redes WiFi
 */
void ui_wifi_bridge_request_scan(void);

/**
 * @brief Manejar selección de red WiFi
 * @param ap_index Índice del access point seleccionado
 */
void ui_wifi_bridge_select_network(int ap_index);

/**
 * @brief Verificar si hay actualizaciones pendientes de WiFi
 * @note Llamar desde el loop principal o tick
 */
void ui_wifi_bridge_process_wifi_updates(void);

/**
 * @brief Manejador para la acción de actualizar lista WiFi desde EEZ Studio
 * @note Esta función debe ser llamada desde la acción action_wifi_update_list
 * @param e Evento LVGL
 */
void action_wifi_update_list(lv_event_t *e);

/**
 * @brief Manejador para la acción de conectar WiFi desde EEZ Studio
 * @note Esta función debe ser llamada desde la acción action_wifi_update_connect
 * @param e Evento LVGL
 */
void action_wifi_update_connect(lv_event_t *e);

#ifdef __cplusplus
}
#endif
