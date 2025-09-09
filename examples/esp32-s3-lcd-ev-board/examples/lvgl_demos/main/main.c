/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "bsp/esp-bsp.h"
#include "ui_eez_test1/src/ui/ui.h"
#include "app/settings.h"
#include "app/app_wifi.h"
#include "app/app_weather.h"
#include "app/ui_bridge.h"

static char *TAG = "app_main";

/* Print log about SRAM and PSRAM memory */
#define LOG_MEM_INFO    (0)

void app_main(void)
{
    // Inicializar configuraciones y WiFi
    ESP_LOGI(TAG, "Initializing settings...");
    ESP_ERROR_CHECK(settings_init());
    
    ESP_LOGI(TAG, "Initializing weather module...");
    ESP_ERROR_CHECK(app_weather_init());
    
    ESP_LOGI(TAG, "Starting WiFi network...");
    app_network_start();
    
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

    /* Release the lock */
    bsp_display_unlock();

    /* Main UI loop - call ui_tick() periodically to handle events */
    while (1) {
        bsp_display_lock(0);
        ui_tick();  /* Process UI events and update screens */
        
        /* Process WiFi UI updates */
        ui_bridge_process_wifi_updates();
        
        bsp_display_unlock();
        
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
