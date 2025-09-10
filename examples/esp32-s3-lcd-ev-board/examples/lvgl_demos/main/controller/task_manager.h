/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// Prioridades de tareas
#define TASK_PRIORITY_UI        10  // Alta prioridad para UI responsiva
#define TASK_PRIORITY_WEATHER   5   // Prioridad media para weather
#define TASK_PRIORITY_WIFI      5   // Prioridad media para WiFi
#define TASK_PRIORITY_SYSTEM    3   // Prioridad baja para tareas del sistema

// Tamaños de stack
#define TASK_STACK_UI          4096  // Mantener - suficiente con 51% libre
#define TASK_STACK_WEATHER     4096  // Incrementar de 3072 - tenía solo 6.5% libre
#define TASK_STACK_WIFI        8192  // Incrementar significativamente de 2048 - tenía solo 1.5% libre  
#define TASK_STACK_SYSTEM      3072  // Incrementar de 2048 - tenía solo 2% libre

// Intervalos de actualización (en ms)
#define UI_UPDATE_INTERVAL     10
#define WEATHER_UPDATE_INTERVAL 5000
#define WIFI_UPDATE_INTERVAL   1000
#define SYSTEM_UPDATE_INTERVAL 10000

/**
 * @brief Inicializar el gestor de tareas y crear todas las tareas del sistema
 */
esp_err_t task_manager_init(void);

/**
 * @brief Obtener el mutex del display para sincronización
 */
SemaphoreHandle_t task_manager_get_display_mutex(void);

/**
 * @brief Suspender todas las tareas excepto la UI (para operaciones críticas)
 */
void task_manager_suspend_background_tasks(void);

/**
 * @brief Reanudar todas las tareas suspendidas
 */
void task_manager_resume_background_tasks(void);

/**
 * @brief Mostrar información de todas las tareas (para debug)
 */
void task_manager_print_task_info(void);

/**
 * @brief Forzar actualización inmediata de weather (útil para botones de UI)
 */
void task_manager_force_weather_update(void);

/**
 * @brief Obtener estadísticas de uso de memoria de las tareas
 */
void task_manager_get_memory_stats(uint32_t *free_heap, uint32_t *min_free_heap, uint32_t *ui_stack_free, uint32_t *weather_stack_free);

#ifdef __cplusplus
}
#endif
