/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "ui_bridge.h"
#include "app_wifi.h"
#include "esp_log.h"

// Incluir headers de EEZ Studio para acceder a objects
#include "ui_eez_test1/src/ui/screens.h"
#include "ui_eez_test1/src/ui/vars.h"
#include "ui_eez_test1/src/ui/ui.h"

static const char *TAG = "ui_bridge";

// Flag para indicar que hay actualizaciones pendientes
static bool wifi_list_needs_update = false;
static bool wifi_scan_in_progress = false;

// Tracking de pantalla actual
static int current_screen = -1;
static bool config_screen_entered = false;

// Forward declaration
static void wifi_selection_event_cb(lv_event_t * e);

void ui_bridge_init(void)
{
    ESP_LOGI(TAG, "UI Bridge initialized");
    wifi_list_needs_update = false;
    wifi_scan_in_progress = false;
}

void ui_bridge_start_wifi_scan(void)
{
    ESP_LOGI(TAG, "Starting WiFi scan...");
    wifi_scan_in_progress = true;
    
    // Enviar evento de escaneo al módulo WiFi
    esp_err_t ret = send_network_event(NET_EVENT_SCAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi scan: %s", esp_err_to_name(ret));
        wifi_scan_in_progress = false;
    }
}

void ui_bridge_update_wifi_list(void)
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
    
    // Llenar con las redes encontradas
    for (int i = 0; i < scan_info_result.ap_count; i++) {
        wifi_ap_record_t *ap = &scan_info_result.ap_info[i];
        
        // Crear texto con SSID y fuerza de señal
        char wifi_text[64];
        snprintf(wifi_text, sizeof(wifi_text), "%s (%d dBm)", 
                 (char*)ap->ssid, ap->rssi);
        
        // Agregar botón a la lista
        lv_obj_t *btn = lv_list_add_btn(objects.wifi_list, 
                                       LV_SYMBOL_WIFI, 
                                       wifi_text);
        
        if (btn != NULL) {
            // Guardar índice del AP en user_data
            lv_obj_set_user_data(btn, (void*)(intptr_t)i);
            
            // Agregar callback para selección
            lv_obj_add_event_cb(btn, wifi_selection_event_cb, 
                               LV_EVENT_CLICKED, NULL);
        }
    }
    
    app_wifi_unlock();
    wifi_scan_in_progress = false;
    wifi_list_needs_update = false;
    
    ESP_LOGI(TAG, "WiFi list updated with %d networks", scan_info_result.ap_count);
}

// Callback para cuando se selecciona una red WiFi
static void wifi_selection_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    int ap_index = (int)(intptr_t)lv_obj_get_user_data(btn);
    
    ui_bridge_select_wifi_network(ap_index);
}

void ui_bridge_select_wifi_network(int ap_index)
{
    if (ap_index < 0 || ap_index >= scan_info_result.ap_count) {
        ESP_LOGE(TAG, "Invalid AP index: %d", ap_index);
        return;
    }
    
    char* selected_ssid = (char*)scan_info_result.ap_info[ap_index].ssid;
    ESP_LOGI(TAG, "WiFi network selected: %s", selected_ssid);
    
    // TODO: Aquí puedes agregar lógica para:
    // 1. Mostrar un diálogo para ingresar contraseña
    // 2. Conectar automáticamente si es una red abierta
    // 3. Actualizar configuración de red
    
    // Por ahora, solo logeamos la selección
}

void ui_bridge_process_wifi_updates(void)
{
    // Detectar si estamos en la pantalla Config verificando si el objeto existe y está visible
    bool in_config_screen = false;
    
    if (objects.config != NULL) {
        lv_obj_t* active_screen = lv_scr_act();
        in_config_screen = (active_screen == objects.config);
    }
    
    // Si entramos a la pantalla de configuración
    if (in_config_screen && !config_screen_entered) {
        ESP_LOGI(TAG, "Entered Config screen - starting WiFi scan");
        config_screen_entered = true;
        ui_bridge_start_wifi_scan();
    } else if (!in_config_screen) {
        config_screen_entered = false;
    }
    
    // Verificar si el escaneo WiFi ha terminado
    if (wifi_scan_in_progress) {
        // Verificar estado del escaneo
        if (scan_info_result.scan_done == WIFI_SCAN_RENEW) {
            wifi_list_needs_update = true;
            app_wifi_state_set(WIFI_SCAN_IDLE); // Marcar como procesado
        }
    }
    
    // Actualizar lista si es necesario
    if (wifi_list_needs_update) {
        ui_bridge_update_wifi_list();
    }
}

void ui_bridge_on_config_screen_enter(void)
{
    ESP_LOGI(TAG, "Config screen entered manually - starting WiFi scan");
    ui_bridge_start_wifi_scan();
}
