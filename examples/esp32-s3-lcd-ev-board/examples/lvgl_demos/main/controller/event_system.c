/*
 * Implementación del sistema de eventos central
 */

#include "event_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_system.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "event_system";

// Configuración del sistema
#define MAX_SUBSCRIBERS 50
#define EVENT_QUEUE_SIZE 100
#define MAX_EVENT_DATA_SIZE 256

// Cola principal de eventos
static QueueHandle_t main_event_queue = NULL;

// Array de suscripciones
static event_subscription_t subscriptions[MAX_SUBSCRIBERS];
static uint32_t num_subscriptions = 0;

// Mutex para proteger suscripciones
static SemaphoreHandle_t subscriptions_mutex = NULL;

// Estadísticas
static uint32_t events_posted_count = 0;
static uint32_t events_processed_count = 0;

// Nombres de eventos para debug
static const char* event_names[] = {
    "WIFI_SCAN_REQUESTED",
    "WIFI_SCAN_START",
    "WIFI_SCAN_COMPLETE", 
    "WIFI_CONNECTED",
    "WIFI_DISCONNECTED",
    "WIFI_CONNECTION_FAILED",
    "WEATHER_FETCH_START",
    "WEATHER_DATA_READY",
    "WEATHER_UPDATE_FAILED",
    "WEATHER_UPDATE_REQUESTED",
    "UI_ACTION_TRIGGERED",
    "UI_SCREEN_CHANGED",
    "UI_BUTTON_PRESSED",
    "UI_SETTING_CHANGED",
    "SETTINGS_CHANGED",
    "SETTINGS_SAVED",
    "SETTINGS_LOADED",
    "SYSTEM_LOW_MEMORY",
    "SYSTEM_ERROR",
    "SYSTEM_STARTUP_COMPLETE",
    "SYSTEM_SHUTDOWN_REQUEST",
    "SPIFFS_LOG_WRITTEN",
    "SPIFFS_CLEANUP_COMPLETE",
    "GEOCODING_SEARCH_REQUESTED",
    "GEOCODING_SEARCH_START",
    "GEOCODING_SEARCH_COMPLETE",
    "GEOCODING_SEARCH_FAILED",
    "SNTP_SYNC_START",
    "SNTP_SYNC_COMPLETE",
    "SNTP_SYNC_FAILED"
};

esp_err_t event_system_init(void)
{
    ESP_LOGI(TAG, "Initializing event system...");
    
    // Crear cola principal de eventos
    main_event_queue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(system_event_t));
    if (main_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create main event queue");
        return ESP_ERR_NO_MEM;
    }
    
    // Crear mutex para suscripciones
    subscriptions_mutex = xSemaphoreCreateMutex();
    if (subscriptions_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create subscriptions mutex");
        vQueueDelete(main_event_queue);
        return ESP_ERR_NO_MEM;
    }
    
    // Inicializar array de suscripciones
    memset(subscriptions, 0, sizeof(subscriptions));
    num_subscriptions = 0;
    
    // Reset estadísticas
    events_posted_count = 0;
    events_processed_count = 0;
    
    ESP_LOGI(TAG, "Event system initialized successfully");
    return ESP_OK;
}

esp_err_t event_system_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing event system...");
    
    if (main_event_queue) {
        vQueueDelete(main_event_queue);
        main_event_queue = NULL;
    }
    
    if (subscriptions_mutex) {
        vSemaphoreDelete(subscriptions_mutex);
        subscriptions_mutex = NULL;
    }
    
    num_subscriptions = 0;
    
    ESP_LOGI(TAG, "Event system deinitialized");
    return ESP_OK;
}

esp_err_t event_system_post(system_event_type_t type, void *data, size_t data_size, event_priority_t priority)
{
    if (main_event_queue == NULL) {
        ESP_LOGE(TAG, "Event system not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (type >= EVENT_TYPE_MAX) {
        ESP_LOGE(TAG, "Invalid event type: %d", type);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (data_size > MAX_EVENT_DATA_SIZE) {
        ESP_LOGW(TAG, "Event data too large (%d bytes), truncating", data_size);
        data_size = MAX_EVENT_DATA_SIZE;
    }
    
    // Crear evento
    system_event_t event = {
        .type = type,
        .priority = priority,
        .data = NULL,
        .data_size = 0,
        .timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS,
        .source_task_id = (uint32_t)xTaskGetCurrentTaskHandle()
    };
    
    // Copiar datos si los hay
    if (data && data_size > 0) {
        void *event_data = malloc(data_size);
        if (event_data == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory for event data");
            return ESP_ERR_NO_MEM;
        }
        memcpy(event_data, data, data_size);
        event.data = event_data;
        event.data_size = data_size;
    }
    
    // Enviar a cola principal
    BaseType_t result;
    if (priority >= EVENT_PRIORITY_HIGH) {
        // Eventos de alta prioridad van al frente
        result = xQueueSendToFront(main_event_queue, &event, pdMS_TO_TICKS(100));
    } else {
        // Eventos normales van atrás
        result = xQueueSendToBack(main_event_queue, &event, pdMS_TO_TICKS(100));
    }
    
    if (result != pdTRUE) {
        ESP_LOGW(TAG, "Failed to post event %s to main queue", event_system_type_to_string(type));
        if (event.data) {
            free(event.data);
        }
        return ESP_ERR_TIMEOUT;
    }
    
    events_posted_count++;
    ESP_LOGD(TAG, "Posted event: %s (priority=%d)", event_system_type_to_string(type), priority);
    
    return ESP_OK;
}

esp_err_t event_system_post_simple(system_event_type_t type, void *data, size_t data_size)
{
    return event_system_post(type, data, data_size, EVENT_PRIORITY_NORMAL);
}

esp_err_t event_system_subscribe_queue(system_event_type_t event_type, QueueHandle_t queue)
{
    if (subscriptions_mutex == NULL || queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (event_type >= EVENT_TYPE_MAX) {
        ESP_LOGE(TAG, "Invalid event type for subscription: %d", event_type);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(subscriptions_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take subscriptions mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    // Buscar slot libre
    if (num_subscriptions >= MAX_SUBSCRIBERS) {
        xSemaphoreGive(subscriptions_mutex);
        ESP_LOGE(TAG, "Maximum subscribers reached");
        return ESP_ERR_NO_MEM;
    }
    
    // Verificar si ya existe
    for (uint32_t i = 0; i < num_subscriptions; i++) {
        if (subscriptions[i].event_type == event_type && subscriptions[i].queue == queue) {
            xSemaphoreGive(subscriptions_mutex);
            ESP_LOGW(TAG, "Already subscribed to event %s", event_system_type_to_string(event_type));
            return ESP_OK;
        }
    }
    
    // Agregar nueva suscripción
    subscriptions[num_subscriptions] = (event_subscription_t) {
        .event_type = event_type,
        .queue = queue,
        .callback = NULL,
        .is_active = true
    };
    num_subscriptions++;
    
    xSemaphoreGive(subscriptions_mutex);
    
    ESP_LOGI(TAG, "Subscribed to event: %s (total subscriptions: %u)", 
             event_system_type_to_string(event_type), num_subscriptions);
    
    return ESP_OK;
}

esp_err_t event_system_subscribe_callback(system_event_type_t event_type, event_handler_t callback)
{
    if (subscriptions_mutex == NULL || callback == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (event_type >= EVENT_TYPE_MAX) {
        ESP_LOGE(TAG, "Invalid event type for subscription: %d", event_type);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(subscriptions_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take subscriptions mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    if (num_subscriptions >= MAX_SUBSCRIBERS) {
        xSemaphoreGive(subscriptions_mutex);
        ESP_LOGE(TAG, "Maximum subscribers reached");
        return ESP_ERR_NO_MEM;
    }
    
    // Agregar nueva suscripción callback
    subscriptions[num_subscriptions] = (event_subscription_t) {
        .event_type = event_type,
        .queue = NULL,
        .callback = callback,
        .is_active = true
    };
    num_subscriptions++;
    
    xSemaphoreGive(subscriptions_mutex);
    
    ESP_LOGI(TAG, "Subscribed callback to event: %s", event_system_type_to_string(event_type));
    
    return ESP_OK;
}

esp_err_t event_system_unsubscribe(system_event_type_t event_type, QueueHandle_t queue)
{
    if (subscriptions_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(subscriptions_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    // Buscar y remover suscripción
    for (uint32_t i = 0; i < num_subscriptions; i++) {
        if (subscriptions[i].event_type == event_type && subscriptions[i].queue == queue) {
            // Mover el último elemento a esta posición
            if (i < num_subscriptions - 1) {
                subscriptions[i] = subscriptions[num_subscriptions - 1];
            }
            num_subscriptions--;
            
            xSemaphoreGive(subscriptions_mutex);
            ESP_LOGI(TAG, "Unsubscribed from event: %s", event_system_type_to_string(event_type));
            return ESP_OK;
        }
    }
    
    xSemaphoreGive(subscriptions_mutex);
    ESP_LOGW(TAG, "Subscription not found for event: %s", event_system_type_to_string(event_type));
    return ESP_ERR_NOT_FOUND;
}

uint32_t event_system_process_events(uint32_t max_events)
{
    if (main_event_queue == NULL) {
        return 0;
    }
    
    uint32_t processed = 0;
    system_event_t event;
    
    while (processed < max_events && xQueueReceive(main_event_queue, &event, 0) == pdTRUE) {
        ESP_LOGD(TAG, "Processing event: %s", event_system_type_to_string(event.type));
        
        // Distribuir a suscriptores
        if (xSemaphoreTake(subscriptions_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            for (uint32_t i = 0; i < num_subscriptions; i++) {
                if (subscriptions[i].event_type == event.type && subscriptions[i].is_active) {
                    if (subscriptions[i].queue != NULL) {
                        // Enviar a cola
                        if (xQueueSend(subscriptions[i].queue, &event, 0) != pdTRUE) {
                            ESP_LOGD(TAG, "Queue full for event %s", event_system_type_to_string(event.type));
                        }
                    } else if (subscriptions[i].callback != NULL) {
                        // Llamar callback
                        subscriptions[i].callback(&event);
                    }
                }
            }
            xSemaphoreGive(subscriptions_mutex);
        }
        
        // Liberar datos del evento
        if (event.data) {
            free(event.data);
        }
        
        processed++;
        events_processed_count++;
    }
    
    if (processed > 0) {
        ESP_LOGD(TAG, "Processed %u events", processed);
    }
    
    return processed;
}

esp_err_t event_system_get_stats(uint32_t *events_posted, uint32_t *events_processed, uint32_t *queue_size)
{
    if (main_event_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (events_posted) *events_posted = events_posted_count;
    if (events_processed) *events_processed = events_processed_count;
    if (queue_size) *queue_size = uxQueueMessagesWaiting(main_event_queue);
    
    return ESP_OK;
}

const char* event_system_type_to_string(system_event_type_t event_type)
{
    if (event_type >= EVENT_TYPE_MAX) {
        return "UNKNOWN_EVENT";
    }
    return event_names[event_type];
}

uint32_t event_system_cleanup_old_events(uint32_t max_age_ms)
{
    if (main_event_queue == NULL) {
        return 0;
    }
    
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t cleaned = 0;
    system_event_t event;
    
    // Esta es una implementación simple - en producción sería más eficiente
    QueueHandle_t temp_queue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(system_event_t));
    if (temp_queue == NULL) {
        return 0;
    }
    
    // Mover eventos válidos a cola temporal
    while (xQueueReceive(main_event_queue, &event, 0) == pdTRUE) {
        if ((current_time - event.timestamp) <= max_age_ms) {
            xQueueSend(temp_queue, &event, 0);
        } else {
            if (event.data) {
                free(event.data);
            }
            cleaned++;
        }
    }
    
    // Mover eventos de vuelta
    while (xQueueReceive(temp_queue, &event, 0) == pdTRUE) {
        xQueueSend(main_event_queue, &event, 0);
    }
    
    vQueueDelete(temp_queue);
    
    if (cleaned > 0) {
        ESP_LOGI(TAG, "Cleaned %u old events", cleaned);
    }
    
    return cleaned;
}