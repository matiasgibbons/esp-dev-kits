/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "task_manager.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

// Incluir BSP
#include "bsp/esp-bsp.h"

// Incluir los bridges de UI
#include "ui.h"
#include "ui_weather_bridge.h"
#include "ui_wifi_bridge.h"

static const char *TAG = "task_manager";

// Handles de las tareas
static TaskHandle_t ui_task_handle = NULL;
static TaskHandle_t weather_task_handle = NULL;
static TaskHandle_t wifi_task_handle = NULL;
static TaskHandle_t system_task_handle = NULL;

// Mutex para sincronización del display
static SemaphoreHandle_t display_mutex = NULL;

// Variables de control
static bool tasks_suspended = false;

/**
 * @brief Tarea principal de UI - maneja eventos y actualiza pantallas
 */
static void ui_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UI Task started on core %d", xPortGetCoreID());
    
    while (1) {
        // Tomar mutex del display
        if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            bsp_display_lock(0);
            ui_tick();  // Procesar eventos de UI
            bsp_display_unlock();
            xSemaphoreGive(display_mutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

/**
 * @brief Tarea de actualizaciones del clima
 */
static void weather_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Weather Task started on core %d", xPortGetCoreID());
    
    while (1) {
        // Tomar mutex del display
        if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            bsp_display_lock(0);
            ui_weather_bridge_process_weather_updates();
            bsp_display_unlock();
            xSemaphoreGive(display_mutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(WEATHER_UPDATE_INTERVAL));
    }
}

/**
 * @brief Tarea de actualizaciones de WiFi
 */
static void wifi_task(void *pvParameters)
{
    ESP_LOGI(TAG, "WiFi Task started on core %d", xPortGetCoreID());
    
    while (1) {
        // Tomar mutex del display
        if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            bsp_display_lock(0);
            ui_bridge_process_wifi_updates();
            bsp_display_unlock();
            xSemaphoreGive(display_mutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL));
    }
}

/**
 * @brief Tarea del sistema - monitoreo y mantenimiento
 */
static void system_task(void *pvParameters)
{
    ESP_LOGI(TAG, "System Task started on core %d", xPortGetCoreID());
    
    uint32_t counter = 0;
    
    while (1) {
        // Cada 10 ciclos (100 segundos), mostrar info de tareas
        if (++counter >= 10) {
            counter = 0;
            task_manager_print_task_info();
        }
        
        vTaskDelay(pdMS_TO_TICKS(SYSTEM_UPDATE_INTERVAL));
    }
}

esp_err_t task_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Task Manager...");
    
    // Crear mutex para el display
    display_mutex = xSemaphoreCreateMutex();
    if (display_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create display mutex");
        return ESP_FAIL;
    }
    
    // Crear tarea de UI (core 1, alta prioridad)
    BaseType_t ret = xTaskCreatePinnedToCore(
        ui_task,
        "UI_Task",
        TASK_STACK_UI,
        NULL,
        TASK_PRIORITY_UI,
        &ui_task_handle,
        1  // Core 1
    );
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UI task");
        return ESP_FAIL;
    }
    
    // Crear tarea de Weather (core 0, prioridad media)
    ret = xTaskCreatePinnedToCore(
        weather_task,
        "Weather_Task",
        TASK_STACK_WEATHER,
        NULL,
        TASK_PRIORITY_WEATHER,
        &weather_task_handle,
        0  // Core 0
    );
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Weather task");
        return ESP_FAIL;
    }
    
    // Crear tarea de WiFi (core 0, prioridad media)
    ret = xTaskCreatePinnedToCore(
        wifi_task,
        "WiFi_Task",
        TASK_STACK_WIFI,
        NULL,
        TASK_PRIORITY_WIFI,
        &wifi_task_handle,
        0  // Core 0
    );
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create WiFi task");
        return ESP_FAIL;
    }
    
    // Crear tarea del sistema (core 0, prioridad baja)
    ret = xTaskCreatePinnedToCore(
        system_task,
        "System_Task",
        TASK_STACK_SYSTEM,
        NULL,
        TASK_PRIORITY_SYSTEM,
        &system_task_handle,
        0  // Core 0
    );
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create System task");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "All tasks created successfully");
    return ESP_OK;
}

SemaphoreHandle_t task_manager_get_display_mutex(void)
{
    return display_mutex;
}

void task_manager_suspend_background_tasks(void)
{
    if (!tasks_suspended) {
        ESP_LOGI(TAG, "Suspending background tasks");
        
        if (weather_task_handle) vTaskSuspend(weather_task_handle);
        if (wifi_task_handle) vTaskSuspend(wifi_task_handle);
        if (system_task_handle) vTaskSuspend(system_task_handle);
        
        tasks_suspended = true;
    }
}

void task_manager_resume_background_tasks(void)
{
    if (tasks_suspended) {
        ESP_LOGI(TAG, "Resuming background tasks");
        
        if (weather_task_handle) vTaskResume(weather_task_handle);
        if (wifi_task_handle) vTaskResume(wifi_task_handle);
        if (system_task_handle) vTaskResume(system_task_handle);
        
        tasks_suspended = false;
    }
}

void task_manager_print_task_info(void)
{
    ESP_LOGI(TAG, "=== Task Information ===");
    
    // Información de memoria
    ESP_LOGI(TAG, "Free heap: %u bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Min free heap: %u bytes", esp_get_minimum_free_heap_size());
    
    // Stack usage de cada tarea
    if (ui_task_handle) {
        UBaseType_t ui_stack = uxTaskGetStackHighWaterMark(ui_task_handle);
        ESP_LOGI(TAG, "UI Task - Stack free: %u bytes", ui_stack * sizeof(StackType_t));
    }
    
    if (weather_task_handle) {
        UBaseType_t weather_stack = uxTaskGetStackHighWaterMark(weather_task_handle);
        ESP_LOGI(TAG, "Weather Task - Stack free: %u bytes", weather_stack * sizeof(StackType_t));
    }
    
    if (wifi_task_handle) {
        UBaseType_t wifi_stack = uxTaskGetStackHighWaterMark(wifi_task_handle);
        ESP_LOGI(TAG, "WiFi Task - Stack free: %u bytes", wifi_stack * sizeof(StackType_t));
    }
    
    if (system_task_handle) {
        UBaseType_t system_stack = uxTaskGetStackHighWaterMark(system_task_handle);
        ESP_LOGI(TAG, "System Task - Stack free: %u bytes", system_stack * sizeof(StackType_t));
    }
    
    // Lista completa de tareas (opcional, solo si hay mucha memoria)
#if CONFIG_FREERTOS_USE_TRACE_FACILITY
    static char task_list_buffer[2048];
    vTaskList(task_list_buffer);
    ESP_LOGI(TAG, "Task List:\n%s", task_list_buffer);
#endif
}

void task_manager_force_weather_update(void)
{
    ESP_LOGI(TAG, "Forcing immediate weather update from UI");
    // Forzar actualización inmediata de weather bypassing throttling
    ui_weather_bridge_force_immediate_update();
}

void task_manager_get_memory_stats(uint32_t *free_heap, uint32_t *min_free_heap, uint32_t *ui_stack_free, uint32_t *weather_stack_free)
{
    if (free_heap) *free_heap = esp_get_free_heap_size();
    if (min_free_heap) *min_free_heap = esp_get_minimum_free_heap_size();
    
    if (ui_stack_free && ui_task_handle) {
        *ui_stack_free = uxTaskGetStackHighWaterMark(ui_task_handle) * sizeof(StackType_t);
    }
    
    if (weather_stack_free && weather_task_handle) {
        *weather_stack_free = uxTaskGetStackHighWaterMark(weather_task_handle) * sizeof(StackType_t);
    }
}
