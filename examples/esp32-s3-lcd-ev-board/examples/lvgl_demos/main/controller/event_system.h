/*
 * Sistema de eventos central para coordinación entre tareas
 * Implementa patrón Publisher-Subscriber con colas FreeRTOS
 */

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tipos de eventos del sistema
typedef enum {
    // Eventos de WiFi
    EVENT_WIFI_SCAN_REQUESTED,
    EVENT_WIFI_SCAN_START,
    EVENT_WIFI_SCAN_COMPLETE,
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_DISCONNECTED,
    EVENT_WIFI_CONNECTION_FAILED,
    
    // Eventos de Weather
    EVENT_WEATHER_FETCH_START,
    EVENT_WEATHER_DATA_READY,
    EVENT_WEATHER_UPDATE_FAILED,
    EVENT_WEATHER_UPDATE_REQUESTED,
    
    // Eventos de UI
    EVENT_UI_ACTION_TRIGGERED,
    EVENT_UI_SCREEN_CHANGED,
    EVENT_UI_BUTTON_PRESSED,
    EVENT_UI_SETTING_CHANGED,
    
    // Eventos de Settings
    EVENT_SETTINGS_CHANGED,
    EVENT_SETTINGS_SAVED,
    EVENT_SETTINGS_LOADED,
    
    // Eventos del Sistema
    EVENT_SYSTEM_LOW_MEMORY,
    EVENT_SYSTEM_ERROR,
    EVENT_SYSTEM_STARTUP_COMPLETE,
    EVENT_SYSTEM_SHUTDOWN_REQUEST,

    // Eventos de Geocoding
    EVENT_GEOCODING_SEARCH_REQUESTED,
    EVENT_GEOCODING_SEARCH_START,
    EVENT_GEOCODING_SEARCH_COMPLETE,
    EVENT_GEOCODING_SEARCH_FAILED,
    
    // Eventos de SPIFFS
    EVENT_SPIFFS_LOG_WRITTEN,
    EVENT_SPIFFS_CLEANUP_COMPLETE,
    
    // Eventos de SNTP/Time
    EVENT_SNTP_SYNC_START,
    EVENT_SNTP_SYNC_COMPLETE,
    EVENT_SNTP_SYNC_FAILED,
    
    EVENT_TYPE_MAX  // Debe ser el último
} system_event_type_t;

// Prioridades de eventos
typedef enum {
    EVENT_PRIORITY_LOW = 0,
    EVENT_PRIORITY_NORMAL = 1,
    EVENT_PRIORITY_HIGH = 2,
    EVENT_PRIORITY_CRITICAL = 3
} event_priority_t;

// Estructura de evento
typedef struct {
    system_event_type_t type;
    event_priority_t priority;
    void *data;
    size_t data_size;
    uint32_t timestamp;
    uint32_t source_task_id;
} system_event_t;

// Callback para manejo de eventos
typedef void (*event_handler_t)(const system_event_t *event);

// Estructura para suscripción a eventos
typedef struct {
    system_event_type_t event_type;
    QueueHandle_t queue;
    event_handler_t callback;
    bool is_active;
} event_subscription_t;

/**
 * @brief Inicializar el sistema de eventos
 * @return esp_err_t
 */
esp_err_t event_system_init(void);

/**
 * @brief Deinicializar el sistema de eventos
 * @return esp_err_t
 */
esp_err_t event_system_deinit(void);

/**
 * @brief Publicar un evento en el sistema
 * @param type Tipo de evento
 * @param data Datos del evento (puede ser NULL)
 * @param data_size Tamaño de los datos
 * @param priority Prioridad del evento
 * @return esp_err_t
 */
esp_err_t event_system_post(system_event_type_t type, void *data, size_t data_size, event_priority_t priority);

/**
 * @brief Publicar un evento con prioridad normal (helper)
 * @param type Tipo de evento
 * @param data Datos del evento
 * @param data_size Tamaño de los datos
 * @return esp_err_t
 */
esp_err_t event_system_post_simple(system_event_type_t type, void *data, size_t data_size);

/**
 * @brief Suscribirse a un tipo de evento con cola
 * @param event_type Tipo de evento
 * @param queue Cola donde recibir eventos
 * @return esp_err_t
 */
esp_err_t event_system_subscribe_queue(system_event_type_t event_type, QueueHandle_t queue);

/**
 * @brief Suscribirse a un tipo de evento con callback
 * @param event_type Tipo de evento
 * @param callback Función callback
 * @return esp_err_t
 */
esp_err_t event_system_subscribe_callback(system_event_type_t event_type, event_handler_t callback);

/**
 * @brief Desuscribirse de un tipo de evento
 * @param event_type Tipo de evento
 * @param queue Cola a desuscribir (NULL si es callback)
 * @return esp_err_t
 */
esp_err_t event_system_unsubscribe(system_event_type_t event_type, QueueHandle_t queue);

/**
 * @brief Procesar eventos pendientes (llamar desde dispatcher task)
 * @param max_events Máximo número de eventos a procesar en esta llamada
 * @return Número de eventos procesados
 */
uint32_t event_system_process_events(uint32_t max_events);

/**
 * @brief Obtener estadísticas del sistema de eventos
 * @param events_posted Total de eventos publicados
 * @param events_processed Total de eventos procesados
 * @param queue_size Tamaño actual de la cola principal
 * @return esp_err_t
 */
esp_err_t event_system_get_stats(uint32_t *events_posted, uint32_t *events_processed, uint32_t *queue_size);

/**
 * @brief Convertir tipo de evento a string (para debug)
 * @param event_type Tipo de evento
 * @return Nombre del evento como string
 */
const char* event_system_type_to_string(system_event_type_t event_type);

/**
 * @brief Limpiar eventos antiguos de la cola (garbage collection)
 * @param max_age_ms Edad máxima en milisegundos
 * @return Número de eventos limpiados
 */
uint32_t event_system_cleanup_old_events(uint32_t max_age_ms);

#ifdef __cplusplus
}
#endif

#endif // EVENT_SYSTEM_H