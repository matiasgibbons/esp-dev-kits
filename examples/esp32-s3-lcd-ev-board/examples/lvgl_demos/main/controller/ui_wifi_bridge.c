/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "ui_wifi_bridge.h"
#include "app_wifi.h"
#include "esp_log.h"
#include "event_system.h"

// Incluir headers de EEZ Studio para acceder a objects
#include "screens.h"
#include "vars.h"
#include "ui.h"

static const char *TAG = "ui_wifi_bridge";

// Estado mínimo UI (solo flags UI, no duplicar datos del modelo)
static bool wifi_list_needs_update = false;
static bool wifi_scan_in_progress = false; // indica que se solicitó un scan

// Forward declaration
static void wifi_selection_event_cb(lv_event_t * e);

// Helper interno para decidir si podemos mostrar la lista
static bool wifi_bridge_has_scan_results(void) {
    const scan_info_t *info = app_wifi_get_scan_results();
    return (info && info->ap_count > 0 && info->scan_done != WIFI_SCAN_IDLE);
}

void ui_wifi_bridge_init(void)
{
    ESP_LOGI(TAG, "UI Bridge initialized");
    wifi_list_needs_update = false;
    wifi_scan_in_progress = false;
}

void ui_wifi_bridge_request_scan(void)
{
    ESP_LOGI(TAG, "Solicitando scan WiFi (EVENT_WIFI_SCAN_REQUESTED)");
    wifi_scan_in_progress = true;
    esp_err_t ret = event_system_post_simple(EVENT_WIFI_SCAN_REQUESTED, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo postear EVENT_WIFI_SCAN_REQUESTED: %s", esp_err_to_name(ret));
        wifi_scan_in_progress = false;
    }
}

void ui_wifi_bridge_update_wifi_list(void)
{
    ESP_LOGI(TAG, "Updating WiFi list in UI...");
    
    // Verificar que el objeto wifi_list existe
    if (objects.wifi_list == NULL) {
        ESP_LOGW(TAG, "WiFi list object not available");
        return;
    }
    
    // Obtener acceso seguro a los datos de escaneo
    if (!app_wifi_lock(100)) {
        ESP_LOGW(TAG, "Could not lock WiFi data");
        return;
    }
    
    // Limpiar la lista anterior
    lv_obj_clean(objects.wifi_list);

    const scan_info_t *info = app_wifi_get_scan_results();
    if (!info) {
        app_wifi_unlock();
        ESP_LOGW(TAG, "Scan info NULL");
        wifi_scan_in_progress = false;
        return;
    }

    // Llenar con las redes encontradas
    for (int i = 0; i < info->ap_count; i++) {
        const wifi_ap_record_t *ap = &info->ap_info[i];
        char wifi_text[64];
        snprintf(wifi_text, sizeof(wifi_text), "%s (%d dBm)", (char*)ap->ssid, ap->rssi);
        lv_obj_t *btn = lv_list_add_btn(objects.wifi_list, LV_SYMBOL_WIFI, wifi_text);
        if (btn) {
            lv_obj_set_user_data(btn, (void*)(intptr_t)i);
            lv_obj_add_event_cb(btn, wifi_selection_event_cb, LV_EVENT_CLICKED, NULL);
        }
    }

    app_wifi_unlock();
    wifi_scan_in_progress = false;
    wifi_list_needs_update = false;
    ESP_LOGI(TAG, "WiFi list updated with %d networks", info->ap_count);
}

// Callback para cuando se selecciona una red WiFi
static void wifi_selection_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    int ap_index = (int)(intptr_t)lv_obj_get_user_data(btn);
    
    ui_wifi_bridge_select_network(ap_index);
}

void ui_wifi_bridge_select_network(int ap_index)
{
    const scan_info_t *info = app_wifi_get_scan_results();
    if (!info || ap_index < 0 || ap_index >= info->ap_count) {
        ESP_LOGE(TAG, "Invalid AP index: %d", ap_index);
        return;
    }

    char* selected_ssid = (char*)info->ap_info[ap_index].ssid;
    ESP_LOGI(TAG, "WiFi network selected: %s", selected_ssid);
    
    // TODO: Aquí puedes agregar lógica para:
    // 1. Mostrar un diálogo para ingresar contraseña
    // 2. Conectar automáticamente si es una red abierta
    // 3. Actualizar configuración de red
    
    // Por ahora, solo logeamos la selección
}

// Función para manejar la acción de actualizar lista WiFi desde EEZ Studio
void action_wifi_update_list(lv_event_t *e)
{
    ESP_LOGI(TAG, "WiFi update list action triggered from EEZ Studio");
    ui_wifi_bridge_request_scan();
}

// Función para manejar la acción de conectar WiFi desde EEZ Studio
void action_wifi_update_connect(lv_event_t *e)
{
    ESP_LOGI(TAG, "WiFi connect action triggered from EEZ Studio");
    // Implementar lógica de conexión según sea necesario
    // Por ahora solo log
}

void ui_wifi_bridge_process_wifi_updates(void)
{
    // Solo verificar si el escaneo WiFi ha terminado
    const scan_info_t *info = app_wifi_get_scan_results();
    if (wifi_scan_in_progress && info) {
        if (info->scan_done == WIFI_SCAN_RENEW) {
            wifi_list_needs_update = true;
            wifi_scan_in_progress = false; // Scan finalizado
            app_wifi_state_set(WIFI_SCAN_IDLE); // marcar procesado
        }
    }

    if (wifi_list_needs_update && wifi_bridge_has_scan_results()) {
        ui_wifi_bridge_update_wifi_list();
        wifi_list_needs_update = false;
    }
}


