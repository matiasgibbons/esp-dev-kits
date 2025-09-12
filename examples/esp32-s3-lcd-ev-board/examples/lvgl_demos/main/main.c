/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"

#include "ui.h"

#include "settings.h"
#include "app_wifi.h"
#include "app_weather.h"

// Nuevos sistemas de persistencia
#include "app_settings.h"
#include "app_spiffs.h"

#include "ui_weather_bridge.h"
#include "ui_wifi_bridge.h"
#include "task_manager.h"

static char *TAG = "app_main";

/* Print log about SRAM and PSRAM memory */
#define LOG_MEM_INFO    (1)

void app_main(void)
{
    ESP_LOGI(TAG, "=== Texel Equipment - Sistema Iniciando ===");
    
    // 1. Inicializar sistemas de persistencia
    ESP_LOGI(TAG, "Initializing NVS settings system...");
    ESP_ERROR_CHECK(app_settings_init());
    
    ESP_LOGI(TAG, "Initializing SPIFFS file system...");
    ESP_ERROR_CHECK(app_spiffs_init());
    
    // 2. Cargar configuraciones del usuario
    app_settings_t user_settings;
    ESP_ERROR_CHECK(app_settings_load(&user_settings));
    ESP_LOGI(TAG, "User settings loaded: WiFi=%s, Location=%s", 
             user_settings.wifi_ssid, user_settings.weather_city);
    
    // 3. Inicializar configuraciones heredadas y módulos principales
    ESP_LOGI(TAG, "Initializing legacy settings...");
    ESP_ERROR_CHECK(settings_init());
    
    ESP_LOGI(TAG, "Initializing weather module...");
    ESP_ERROR_CHECK(app_weather_init());
    
    // 4. Configurar WiFi con credenciales guardadas (si existen)
    ESP_LOGI(TAG, "Starting WiFi network...");
    app_network_start();
    
    // TODO: Aplicar configuraciones cargadas (WiFi, ubicación, etc.)
    if (strlen(user_settings.wifi_ssid) > 0) {
        ESP_LOGI(TAG, "Applying saved WiFi configuration: %s", user_settings.wifi_ssid);
        // Aquí se podría auto-conectar al WiFi guardado
    }
    
    // 5. Inicializar hardware y display
    
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

    ESP_LOGI(TAG, "Display LVGL demo");
    /**
     * To avoid errors caused by multiple tasks simultaneously accessing LVGL,
     * should acquire a lock before operating on LVGL.
     */
    bsp_display_lock(0);

    /* Initialize your custom EEZ Studio UI */
    ui_init();
    
    /* Initialize UI Bridge for WiFi integration */
    ui_bridge_init();
    
    /* Initialize UI Bridge for Weather integration */
    ui_weather_bridge_init();

    /* Release the lock */
    bsp_display_unlock();

    /* Initialize and start the task manager */
    ESP_LOGI(TAG, "Starting task manager...");
    ESP_ERROR_CHECK(task_manager_init());
    
    // 6. Programar limpieza periódica de logs antiguos
    ESP_LOGI(TAG, "Scheduling periodic maintenance tasks...");
    // La limpieza se ejecutará cada 24 horas mediante el task manager
    
    /* Main task becomes idle - all work is now done by dedicated tasks */
    ESP_LOGI(TAG, "=== Sistema inicializado exitosamente ===");
    ESP_LOGI(TAG, "SPIFFS: Logs de actividad, configuraciones y respaldos disponibles");
    ESP_LOGI(TAG, "NVS: Configuraciones persistentes cargadas");
    ESP_LOGI(TAG, "Sistema listo para uso. Main task en modo monitor.");
    
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
        // El main task ahora solo hace monitoreo básico
        vTaskDelay(pdMS_TO_TICKS(60000));  // Delay 1 minuto
        
        // Log periódico del estado del sistema
        ESP_LOGI(TAG, "System alive - Free heap: %u bytes", esp_get_free_heap_size());
        
#if LOG_MEM_INFO
        /* Log memory info every 5 seconds (500 * 10ms) */
        static int mem_log_counter = 0;
        if (++mem_log_counter >= 500) {
            mem_log_counter = 0;
            static char buffer[128];    /* Make sure buffer is enough for `sprintf` */
            /**
             * It's not recommended to frequently use functions like `heap_caps_get_free_size()` to obtain memory information
             * in practical applications, especially when the application extensively uses `malloc()` to dynamically allocate
             * a significant number of memory blocks. The frequent interrupt disabling may potentially lead to issues with other functionalities.
             */
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
        
        vTaskDelay(pdMS_TO_TICKS(10));  /* Delay 10ms between updates */
    }
}
