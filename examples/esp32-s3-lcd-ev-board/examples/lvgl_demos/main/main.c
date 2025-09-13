/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

// -----------------------------------------------------------------------------
// Punto de entrada principal: inicializa persistencia, UI, tareas y networking
// Arquitectura: MVC + Bridges + Sistema de Eventos (una sola tarea LVGL)
// -----------------------------------------------------------------------------

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include <time.h>

#include "ui.h"
#include "app_wifi.h"
#include "app_weather.h"
#include "app_settings.h"  // Sistema unificado de configuraciones (reemplaza antiguo settings.h)
#include "app_spiffs.h"     // Persistencia de logs y archivos

#include "ui_weather_bridge.h"
#include "ui_wifi_bridge.h"
#include "task_manager.h"

static char *TAG = "app_main";

/* Print log about SRAM and PSRAM memory */
#define LOG_MEM_INFO    (1)

void app_main(void)
{
    ESP_LOGI(TAG, "=== Texel Equipment - Sistema Iniciando ===");

    // 1. Persistencia / settings (NVS + SPIFFS)
    ESP_LOGI(TAG, "Inicializando NVS (settings)...");
    ESP_ERROR_CHECK(app_settings_init());
    app_settings_t *settings = app_settings_get();
    // Carga directa de NVS sobre estructura temporal y copia completa al singleton
    app_settings_t temp_loaded;
    ESP_ERROR_CHECK(app_settings_load(&temp_loaded));
    // Copiar campos relevantes al singleton (manteniendo perfiles predefinidos TEXEL si estaban vacíos)
    if (temp_loaded.wifi_ssid[0] != '\0') {
        strncpy(settings->wifi_ssid, temp_loaded.wifi_ssid, sizeof(settings->wifi_ssid)-1);
        strncpy(settings->wifi_password, temp_loaded.wifi_password, sizeof(settings->wifi_password)-1);
        if (settings->wifi_networks[0].ssid[0] == '\0') {
            strncpy(settings->wifi_networks[0].ssid, temp_loaded.wifi_ssid, sizeof(settings->wifi_networks[0].ssid)-1);
            strncpy(settings->wifi_networks[0].password, temp_loaded.wifi_password, sizeof(settings->wifi_networks[0].password)-1);
        }
    }
    settings->weather_lat = temp_loaded.weather_lat;
    settings->weather_lon = temp_loaded.weather_lon;
    strncpy(settings->weather_city, temp_loaded.weather_city, sizeof(settings->weather_city)-1);
    settings->display_brightness = temp_loaded.display_brightness;
    strncpy(settings->language, temp_loaded.language, sizeof(settings->language)-1);
    ESP_LOGI(TAG, "Settings cargados: WiFi=%s, Ciudad=%s", settings->wifi_ssid, settings->weather_city);

    ESP_LOGI(TAG, "Inicializando SPIFFS...");
    ESP_ERROR_CHECK(app_spiffs_init());

    // 2. UI / Display antes de levantar tareas que emiten eventos para evitar carreras
    bsp_i2c_init();
    lv_disp_t *disp = bsp_display_start();

#if CONFIG_BSP_DISPLAY_LVGL_AVOID_TEAR
    ESP_LOGI(TAG, "Avoid lcd tearing effect");
#if CONFIG_BSP_DISPLAY_LVGL_FULL_REFRESH
    ESP_LOGI(TAG, "LVGL full-refresh");
#elif CONFIG_BSP_DISPLAY_LVGL_DIRECT_MODE
    ESP_LOGI(TAG, "LVGL direct-mode");
#endif
#endif

    ESP_LOGI(TAG, "Inicializando UI (EEZ + Bridges)");
    /**
     * To avoid errors caused by multiple tasks simultaneously accessing LVGL,
     * should acquire a lock before operating on LVGL.
     */
    bsp_display_lock(0);

    /* Inicializar UI generada y bridges */
    ui_init();
    ui_wifi_bridge_init();
    ui_weather_bridge_init();

    /* Aplicar brillo configurado (placeholder: si existe API en BSP, usarla) */
    // bsp_display_set_brightness(settings->display_brightness); // TODO: implementar si BSP lo soporta

    /* Release the lock */
    bsp_display_unlock();

    // 3. Task Manager (crea colas y suscripciones de eventos)
    ESP_LOGI(TAG, "Iniciando task manager...");
    ESP_ERROR_CHECK(task_manager_init());

    // 4. Módulos de dominio (publicarán eventos ya con sistema listo)
    ESP_LOGI(TAG, "Inicializando módulo weather...");
    ESP_ERROR_CHECK(app_weather_init());

    // 5. Networking: aplicar fallback TEXEL si no hay credenciales
    if (settings->wifi_ssid[0] == '\0') {
        ESP_LOGW(TAG, "No hay credenciales WiFi guardadas; activando fallback TEXEL automáticamente");
        ESP_ERROR_CHECK(app_settings_set_active_network(settings, 2)); // índice TEXEL
        ESP_LOGI(TAG, "Fallback WiFi activo: %s", settings->wifi_ssid);
    }
    ESP_LOGI(TAG, "Arrancando WiFi network task...");
    app_network_start();

    ESP_LOGI(TAG, "=== Sistema inicializado exitosamente ===");
    ESP_LOGI(TAG, "Persistencia: SPIFFS OK / NVS OK");
    ESP_LOGI(TAG, "Main task en modo monitor.");
    
    // 7. Registro de inicio en logs de actividad
    activity_log_entry_t startup_log = {
        .timestamp = time(NULL),
        .session_type = "Sistema",
        .emission_mode = "Inicio",
        .duration_minutes = 0,
        .power_level = 0,
        .notes = "Sistema iniciado correctamente"
    };
    app_spiffs_log_activity(&startup_log);
    
    while (1) {
    // Monitoreo cada 60s
    vTaskDelay(pdMS_TO_TICKS(60000));
    ESP_LOGI(TAG, "Heartbeat - Free heap: %u bytes", esp_get_free_heap_size());
#if LOG_MEM_INFO
    // Log de memoria cada 5 minutos (contador de minutos)
    static int min_counter = 0;
    if (++min_counter >= 5) {
        min_counter = 0;
        static char buffer[128];
        sprintf(buffer, "   Biggest /     Free /    Total\n"
            "\t  SRAM : [%8d / %8d / %8d]\n"
            "\t PSRAM : [%8d / %8d / %8d]",
            heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
            heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
            heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
            heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
            heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
            heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
        ESP_LOGI("MEM", "%s", buffer);
    }
#endif
    }
}
