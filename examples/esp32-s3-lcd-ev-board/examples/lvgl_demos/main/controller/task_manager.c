/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "task_manager.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_task_wdt.h"
#include "event_system.h"

// Incluir BSP
#include "bsp/esp-bsp.h"

// Incluir los bridges de UI
#include "ui.h"
#include "ui_weather_bridge.h"
#include "ui_wifi_bridge.h"
#include "ui_geocoding_bridge.h"

// Incluir módulos de datos
#include "app_sntp.h"
#include "app_wifi.h"

static const char *TAG = "task_manager";

// Handles de las tareas
static TaskHandle_t ui_task_handle = NULL;
static TaskHandle_t weather_task_handle = NULL; // también procesa geocoding
static TaskHandle_t wifi_task_handle = NULL;
static TaskHandle_t system_task_handle = NULL;

// Colas de eventos por tarea
static QueueHandle_t ui_event_queue = NULL;
static QueueHandle_t geocoding_event_queue = NULL; // cola para peticiones geocoding + weather triggers
static QueueHandle_t wifi_event_queue = NULL; // cola para peticiones WiFi

// Variables de control
static bool tasks_suspended = false;

/**
 * @brief UI Task - ÚNICA que accede a LVGL
 */
static void ui_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UI Task started on core %d (EVENT-DRIVEN LVGL)", xPortGetCoreID());
    
    esp_task_wdt_add(NULL);
    
    system_event_t event;
    uint32_t memory_check_counter = 0;
    
    while (1) {
        esp_task_wdt_reset();
        
        // Procesar eventos de otras tareas (no bloqueante)
        while (xQueueReceive(ui_event_queue, &event, 0) == pdTRUE) {
            ESP_LOGD(TAG, "Processing UI event: %s", event_system_type_to_string(event.type));
            
            bsp_display_lock(0);
            
            switch (event.type) {
                case EVENT_WIFI_SCAN_COMPLETE:
                    ui_wifi_bridge_process_wifi_updates();
                    break;
                case EVENT_WIFI_CONNECTED:
                    ESP_LOGI(TAG, "UI: WiFi conectado");
                    // TODO: ui_wifi_bridge_update_status();
                    break;
                case EVENT_WIFI_DISCONNECTED:
                    ESP_LOGI(TAG, "UI: WiFi desconectado");
                    break;
                case EVENT_WIFI_CONNECTION_FAILED:
                    ESP_LOGW(TAG, "UI: WiFi conexión fallida (máximos reintentos)");
                    break;
                    
                case EVENT_WEATHER_DATA_READY:
                    ui_weather_bridge_process_weather_updates();
                    break;
                    
                case EVENT_WEATHER_UPDATE_REQUESTED:
                    ui_weather_bridge_force_immediate_update();
                    ui_weather_bridge_process_weather_updates();
                    break;
                case EVENT_GEOCODING_SEARCH_START:
                case EVENT_GEOCODING_SEARCH_COMPLETE:
                case EVENT_GEOCODING_SEARCH_FAILED:
                    ui_geocoding_bridge_process_updates(&event);
                    break;
                    
                case EVENT_SETTINGS_CHANGED:
                    // ui_bridge_process_settings_updates(); // TODO: implementar
                    ESP_LOGI(TAG, "Settings changed - UI update needed");
                    break;
                    
                case EVENT_SNTP_SYNC_START:
                    ESP_LOGI(TAG, "UI: SNTP sincronización iniciada");
                    break;
                case EVENT_SNTP_SYNC_COMPLETE:
                    ESP_LOGI(TAG, "UI: SNTP sincronización completada");
                    break;
                case EVENT_SNTP_SYNC_FAILED:
                    ESP_LOGW(TAG, "UI: SNTP sincronización falló");
                    break;
                    
                case EVENT_SYSTEM_LOW_MEMORY:
                    if (event.data && event.data_size == sizeof(uint32_t)) {
                        uint32_t free_heap = *((uint32_t*)event.data);
                        ESP_LOGW(TAG, "Low memory warning: %u bytes free", free_heap);
                        // Forzar limpieza de memoria LVGL
                        lv_obj_invalidate(lv_scr_act());
                    }
                    break;
                    
                default:
                    ESP_LOGD(TAG, "Unhandled UI event: %s", event_system_type_to_string(event.type));
                    break;
            }
            
            bsp_display_unlock();
            
            // Liberar datos del evento si los hay
            if (event.data) {
                free(event.data);
            }
        }
        
        // Check LVGL memory state every 500 cycles (5 seconds)
        if (++memory_check_counter >= 500) {
            memory_check_counter = 0;
            
            lv_mem_monitor_t monitor;
            lv_mem_monitor(&monitor);
            
            if (monitor.free_size < (monitor.total_size / 10)) {  // Less than 10% free
                ESP_LOGW(TAG, "LVGL memory critical! Free: %u KB, Total: %u KB, Fragmentation: %u%%", 
                         monitor.free_size / 1024, monitor.total_size / 1024,
                         (monitor.frag_pct));
                         
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
        
        // Procesamiento regular de UI (sin mutex, somos la única tarea)
        bsp_display_lock(0);
        
        lv_mem_monitor_t pre_monitor;
        lv_mem_monitor(&pre_monitor);
        
        ui_tick();  // Eventos y renderizado principal
        
        lv_mem_monitor_t post_monitor;
        lv_mem_monitor(&post_monitor);
        
        if (post_monitor.free_size < 1024) {  // Less than 1KB free
            ESP_LOGE(TAG, "LVGL memory exhausted! Attempting recovery...");
            lv_obj_invalidate(lv_scr_act());
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        bsp_display_unlock();
        
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

/**
 * @brief Tarea de datos del clima - NO accede a LVGL directamente
 */
static void weather_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Weather/Geocoding Task started on core %d (DATA ONLY - EVENT DRIVEN)", xPortGetCoreID());

    esp_task_wdt_add(NULL);
    system_event_t evt;
    TickType_t last_weather_fetch = 0;

    while (1) {
        esp_task_wdt_reset();

        // Procesar eventos en cola principal (geocoding requests)
    while (geocoding_event_queue && xQueueReceive(geocoding_event_queue, &evt, 0) == pdTRUE) {
            ESP_LOGD(TAG, "Weather task received event: %s", event_system_type_to_string(evt.type));
            if (evt.type == EVENT_GEOCODING_SEARCH_REQUESTED) {
                // Payload con parámetros
                if (evt.data && evt.data_size == sizeof(geocoding_search_request_t)) {
                    geocoding_search_request_t req;
                    memcpy(&req, evt.data, sizeof(req));
                    ESP_LOGI(TAG, "Geocoding request: %s, %s", req.country_code, req.city);
                    event_system_post_simple(EVENT_GEOCODING_SEARCH_START, NULL, 0);

                    geocoding_location_t temp_results[5];
                    int num_results = 0;
                    esp_err_t gret = app_weather_geocoding_search_cities(req.country_code, req.city, temp_results, 5, &num_results);
                    if (gret == ESP_OK) {
                        geocoding_search_result_evt_t result_evt = { .num_results = num_results };
                        if (num_results > 5) num_results = 5;
                        memcpy(result_evt.results, temp_results, sizeof(geocoding_location_t) * num_results);
                        event_system_post(EVENT_GEOCODING_SEARCH_COMPLETE, &result_evt, sizeof(result_evt), EVENT_PRIORITY_HIGH);
                    } else {
                        event_system_post_simple(EVENT_GEOCODING_SEARCH_FAILED, NULL, 0);
                    }
                }
            } else if (evt.type == EVENT_WIFI_CONNECTED) {
                ESP_LOGI(TAG, "Weather task: WiFi connected -> iniciando SNTP async");
                // Iniciar SNTP de forma no-bloqueante
                app_sntp_init_async();
                // El weather fetch se disparará cuando llegue EVENT_SNTP_SYNC_COMPLETE
            } else if (evt.type == EVENT_SNTP_SYNC_COMPLETE) {
                ESP_LOGI(TAG, "Weather task: SNTP sincronizado -> disparar fetch inicial clima");
                last_weather_fetch = xTaskGetTickCount();
                event_system_post_simple(EVENT_WEATHER_FETCH_START, NULL, 0);
                esp_err_t wret = app_weather_request(LOCATION_NUM_ROSARIO);
                if (wret == ESP_OK) {
                    event_system_post_simple(EVENT_WEATHER_DATA_READY, NULL, 0);
                } else {
                    ESP_LOGW(TAG, "Weather fetch failed after SNTP: %s", esp_err_to_name(wret));
                    event_system_post_simple(EVENT_WEATHER_UPDATE_FAILED, NULL, 0);
                }
            } else if (evt.type == EVENT_WEATHER_UPDATE_REQUESTED) {
                ESP_LOGI(TAG, "Weather task: actualización solicitada por usuario/UI");
                event_system_post_simple(EVENT_WEATHER_FETCH_START, NULL, 0);
                esp_err_t wret = app_weather_request(LOCATION_NUM_ROSARIO);
                if (wret == ESP_OK) {
                    event_system_post_simple(EVENT_WEATHER_DATA_READY, NULL, 0);
                } else {
                    ESP_LOGW(TAG, "Weather fetch failed on manual update: %s", esp_err_to_name(wret));
                    event_system_post_simple(EVENT_WEATHER_UPDATE_FAILED, NULL, 0);
                }
            }
            if (evt.data) free(evt.data);
        }

        // Fetch periódico de clima (throttled, solo si WiFi conectado)
        TickType_t now = xTaskGetTickCount();
        if ((now - last_weather_fetch) > pdMS_TO_TICKS(WEATHER_UPDATE_INTERVAL)) {
            if (app_wifi_is_connected()) {
                last_weather_fetch = now;
                event_system_post_simple(EVENT_WEATHER_FETCH_START, NULL, 0);
                esp_err_t wret = app_weather_request(LOCATION_NUM_ROSARIO);
                if (wret == ESP_OK) {
                    event_system_post_simple(EVENT_WEATHER_DATA_READY, NULL, 0);
                } else {
                    ESP_LOGW(TAG, "Weather fetch failed (periodic): %s", esp_err_to_name(wret));
                    event_system_post_simple(EVENT_WEATHER_UPDATE_FAILED, NULL, 0);
                }
            } else {
                ESP_LOGI(TAG, "Weather periodic fetch skipped: WiFi not connected");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief Tarea de WiFi - NO accede a LVGL directamente
 */
static void wifi_task(void *pvParameters)
{
    ESP_LOGI(TAG, "WiFi Task started on core %d (DATA ONLY - EVENT DRIVEN)", xPortGetCoreID());

    esp_task_wdt_add(NULL);
    bool pending_scan = false;

    system_event_t evt;

    while (1) {
        esp_task_wdt_reset();

        // Procesar eventos de petición
        while (wifi_event_queue && xQueueReceive(wifi_event_queue, &evt, 0) == pdTRUE) {
            if (evt.type == EVENT_WIFI_SCAN_REQUESTED) {
                ESP_LOGI(TAG, "WiFi task: scan request received");
                pending_scan = true;
            }
            if (evt.data) free(evt.data);
        }

        if (pending_scan) {
            pending_scan = false;
            ESP_LOGD(TAG, "WiFi task: starting scan...");
            event_system_post_simple(EVENT_WIFI_SCAN_START, NULL, 0);
            // TODO: llamar a app_wifi_scan() real cuando esté disponible
            bool scan_success = true; // Simular éxito
            if (scan_success) {
                ESP_LOGD(TAG, "WiFi task: scan complete");
                event_system_post_simple(EVENT_WIFI_SCAN_COMPLETE, NULL, 0);
            } else {
                ESP_LOGW(TAG, "WiFi task: scan failed");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200)); // pequeño delay para watchdog y baja latencia a requests
    }
}

/**
 * @brief Tarea del sistema - monitoreo, mantenimiento y dispatcher de eventos
 */
static void system_task(void *pvParameters)
{
    ESP_LOGI(TAG, "System Task started on core %d (SYSTEM MONITOR + EVENT DISPATCHER)", xPortGetCoreID());
    
    uint32_t counter = 0;
    
    while (1) {
        // Procesar eventos del sistema (dispatcher central)
        uint32_t processed = event_system_process_events(10); // Máximo 10 eventos por ciclo
        if (processed > 0) {
            ESP_LOGD(TAG, "Processed %u events", processed);
        }
        
        // Cada 10 ciclos (100 segundos), mostrar info de tareas
        if (++counter >= 10) {
            counter = 0;
            task_manager_print_task_info();
            
            // Verificar memoria del sistema
            uint32_t free_heap = esp_get_free_heap_size();
            uint32_t min_free = esp_get_minimum_free_heap_size();
            
            ESP_LOGI(TAG, "System health: Free heap=%u, Min free=%u", free_heap, min_free);
            
            // Publicar evento de memoria baja si es necesario
            if (free_heap < 50000) { // Threshold de 50KB
                event_system_post(EVENT_SYSTEM_LOW_MEMORY, &free_heap, sizeof(free_heap), EVENT_PRIORITY_HIGH);
            }
            
            // Obtener estadísticas del sistema de eventos
            uint32_t events_posted, events_processed, queue_size;
            if (event_system_get_stats(&events_posted, &events_processed, &queue_size) == ESP_OK) {
                ESP_LOGI(TAG, "Event stats: Posted=%u, Processed=%u, Queue=%u", 
                         events_posted, events_processed, queue_size);
            }
            
            // Limpiar eventos antiguos (mayores a 30 segundos)
            uint32_t cleaned = event_system_cleanup_old_events(30000);
            if (cleaned > 0) {
                ESP_LOGI(TAG, "Cleaned %u old events", cleaned);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(SYSTEM_UPDATE_INTERVAL));
    }
}

esp_err_t task_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Event-Driven Task Manager...");
    
    // Inicializar sistema de eventos primero
    ESP_ERROR_CHECK(event_system_init());
    
    // Crear cola de eventos para UI task
    ui_event_queue = xQueueCreate(20, sizeof(system_event_t));
    if (ui_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create UI event queue");
        return ESP_FAIL;
    }

    // Crear cola de eventos para WiFi task (peticiones de scan)
    wifi_event_queue = xQueueCreate(10, sizeof(system_event_t));
    // Crear cola de eventos para Geocoding
    geocoding_event_queue = xQueueCreate(10, sizeof(system_event_t));
    if (geocoding_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create Geocoding event queue");
        return ESP_FAIL;
    }
    if (wifi_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi event queue");
        return ESP_FAIL;
    }
    
    // Suscribir UI task a eventos relevantes
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WIFI_SCAN_COMPLETE, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WEATHER_DATA_READY, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WEATHER_UPDATE_REQUESTED, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_SETTINGS_CHANGED, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_SYSTEM_LOW_MEMORY, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_GEOCODING_SEARCH_START, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_GEOCODING_SEARCH_COMPLETE, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_GEOCODING_SEARCH_FAILED, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_SNTP_SYNC_START, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_SNTP_SYNC_COMPLETE, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_SNTP_SYNC_FAILED, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WIFI_CONNECTED, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WIFI_DISCONNECTED, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WIFI_CONNECTION_FAILED, ui_event_queue));
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_GEOCODING_SEARCH_REQUESTED, geocoding_event_queue));
    // Permitir que la tarea de weather reciba notificación cuando hay WiFi para disparar fetch inicial
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WIFI_CONNECTED, geocoding_event_queue));
    // Permitir que la tarea de weather procese requests manuales de actualización (además de UI)
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WEATHER_UPDATE_REQUESTED, geocoding_event_queue));
    // Permitir que la tarea de weather reaccione cuando SNTP esté listo
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_SNTP_SYNC_COMPLETE, geocoding_event_queue));
    // Suscripción WiFi task a evento REQUESTED
    ESP_ERROR_CHECK(event_system_subscribe_queue(EVENT_WIFI_SCAN_REQUESTED, wifi_event_queue));
    
    // Crear tarea de UI (core 1, alta prioridad) - ÚNICA que accede LVGL
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
    
    // Crear tareas de datos (NO acceden a LVGL)
    ret = xTaskCreatePinnedToCore(
        weather_task,
        "Weather_Data",
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
    
    ret = xTaskCreatePinnedToCore(
        wifi_task,
        "WiFi_Data", 
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
    
    // Publicar evento de sistema iniciado
    event_system_post_simple(EVENT_SYSTEM_STARTUP_COMPLETE, NULL, 0);
    
    ESP_LOGI(TAG, "Event-driven task manager initialized successfully");
    return ESP_OK;
}

SemaphoreHandle_t task_manager_get_display_mutex(void)
{
    // Ya no hay mutex - la UI task maneja todo acceso a LVGL
    return NULL;
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
    ESP_LOGI(TAG, "Forcing immediate weather update via event system");
    
    // Publicar evento de actualización solicitada
    esp_err_t ret = event_system_post(EVENT_WEATHER_UPDATE_REQUESTED, NULL, 0, EVENT_PRIORITY_HIGH);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to post weather update request event: %s", esp_err_to_name(ret));
    }
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
