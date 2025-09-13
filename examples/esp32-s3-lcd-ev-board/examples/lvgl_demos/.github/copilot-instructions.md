## Propósito

Guía corta y práctica para agentes de código (AI) que trabajan en este proyecto ESP32 + LVGL (UI generada con EEZ Studio). Usa este archivo para ser productivo rápidamente y evitar modificar artefactos generados.

## Visión general (qué es el proyecto)
Convenciones y patrones (concreto)

Agregar archivos fuente: colocar nuevos controllers en `main/controller/` o modelos en `main/model/`. El `main/CMakeLists.txt` los incluye automáticamente (GLOB_RECURSE para `view/src_ui` y GLOB para `controller`/`model`).

## Arquitectura Funcional (Metodología Extendida)

### Patrón en Capas: MVC + Bridge + Sistema de Eventos + Hilo Único de UI

1. View (Generada, EEZ Studio)  
  Ruta: `main/view/src_ui/` — Layout de la UI, objetos de widgets, stubs de acciones EEZ. NUNCA editar manualmente; regenerar con EEZ Studio si la UI cambia.
2. Bridge (`ui_*_bridge.c`)  
  Media entre acciones generadas y lógica del modelo. Se encarga de formateo/traducción de datos crudos del modelo a textos/iconos. Funciones expuestas: `ui_<feature>_bridge_init()`, `ui_<feature>_bridge_process_*_updates()`, handlers `ui_<feature>_action_*`.
3. Model (`app_*.c`)  
  Lógica pura: red (HTTP/WiFi), parsing, persistencia (SPIFFS), settings. Sin llamadas LVGL.
4. Event System (`event_system.c/.h`)  
  Centro publish/subscribe (cola + callbacks opcionales) con eventos tipados y prioridad.
5. Task Manager (`task_manager.c`)  
  Crea tareas: UI (única que toca LVGL), Weather, WiFi, System/dispatcher & monitoreo.

### Regla de Hilo Único de UI
Solo la tarea UI interactúa con LVGL. El resto de tareas publica eventos con:
* Sin payload (estado en el modelo, el bridge lo consulta), o
* Referencias/payload pequeño (evitar copias grandes de heap).  
No se necesita mutex para LVGL; el lock de display está encapsulado dentro del loop de la tarea UI (`bsp_display_lock/unlock`).

### Resumen del Sistema de Eventos
El enum `system_event_type_t` (ver `event_system.h`) incluye dominios: WIFI, WEATHER, UI, SETTINGS, SYSTEM, SPIFFS. Subconjunto de ejemplo:
```
EVENT_WEATHER_FETCH_START
EVENT_WEATHER_DATA_READY
EVENT_WEATHER_UPDATE_REQUESTED
EVENT_WIFI_SCAN_START
EVENT_WIFI_SCAN_COMPLETE
EVENT_SYSTEM_LOW_MEMORY
EVENT_SETTINGS_CHANGED
```
Priorities: `EVENT_PRIORITY_{LOW,NORMAL,HIGH,CRITICAL}`.

Core API:
```
event_system_init();
event_system_post(type, data, size, priority);
event_system_post_simple(type, data, size); // normal priority helper
event_system_subscribe_queue(type, queue);
event_system_subscribe_callback(type, handler);
event_system_process_events(max_events); // (used in system or dispatcher task)
```

Publishing Pattern (model/task):
```
event_system_post_simple(EVENT_WEATHER_FETCH_START, NULL, 0);
// fetch...
event_system_post(EVENT_WEATHER_DATA_READY, NULL, 0, EVENT_PRIORITY_HIGH);
```

Consumption Pattern (UI task):
```
while (xQueueReceive(ui_event_queue, &evt, 0) == pdTRUE) {
  switch(evt.type) {
   case EVENT_WEATHER_DATA_READY:
    ui_weather_bridge_process_weather_updates();
    break;
   case EVENT_WIFI_SCAN_COMPLETE:
    ui_wifi_bridge_process_wifi_updates();
    break;
   // ...
  }
}
```

### Flujos Típicos
Weather periódico:
```
Weather_Task -> EVENT_WEATHER_FETCH_START
Weather_Task (success) -> EVENT_WEATHER_DATA_READY
UI_Task (queue) -> ui_weather_bridge_process_weather_updates() -> pulls model state -> updates widgets
```
User refresh (acción usuario):
```
EEZ action -> ui_weather_action_refresh() -> EVENT_WEATHER_UPDATE_REQUESTED
Weather_Task sees request -> fetch -> EVENT_WEATHER_DATA_READY -> UI updates
```
WiFi scan:
```
WiFi_Task -> EVENT_WIFI_SCAN_START -> scan -> EVENT_WIFI_SCAN_COMPLETE -> UI_Task -> ui_wifi_bridge_process_wifi_updates()
```
Low memory (memoria baja):
```
System_Task detects -> EVENT_SYSTEM_LOW_MEMORY -> UI_Task may simplify UI / log
```

### Comentarios y Logs
* Usar `ESP_LOGI` para eventos importantes (transiciones de estado).
* Usar `ESP_LOGD` para trazas de depuración (información detallada).
* Comentar en español cada funcion o bucle.
* Agregar breve descripción de cada archivo indicando su utilidad, indicar:
  - Funcionalidad principal.
  - Que funcion se expone al codigo externo.
  - Que eventos se publican.

### Convenciones de Nombres
Events: `EVENT_<DOMAIN>_<STATE>` (ej: `EVENT_WEATHER_DATA_READY`).  
Funciones de modelo: `app_<domain>_<action>` (ej: `app_weather_get_info`).  
Funciones de actualización bridge: `ui_<domain>_bridge_update_*`.  
Action handlers (desde UI): `ui_<domain>_action_<verb>`.

### Añadir un Nuevo Feature (Checklist)
1. Model: crear `app_<feature>.c/.h` (sin LVGL).  
2. Bridge: crear `ui_<feature>_bridge.c` con `init`, `process_*_updates` y handlers de acciones.  
3. Events: añadir valores al enum `system_event_type_t` (antes de `EVENT_TYPE_MAX`), agregar nombre en la tabla de `event_system.c`.  
4. Suscribir la cola de la UI a los eventos necesarios en `task_manager_init()`.  
5. Publicar eventos desde la tarea productora o el modelo.  
6. Implementar en el bridge `process_*` para leer estado del modelo y actualizar widgets.  
7. Mantener formateo de UI (labels, strings) en el bridge (localización en español).  
8. Asegurar cero llamadas LVGL fuera de la tarea UI.

### Buenas Prácticas
* Preferir publicar eventos frente a llamadas directas cross-task que puedan bloquear.  
* Dividir loops de fetch (weather/WiFi) en trozos con delays cortos (<1s) para mantener feliz al watchdog.  
* Mantener payload de eventos pequeño; datos grandes residir en singletons del modelo.  
* Log de transiciones con `ESP_LOGI` y trazas frecuentes con `ESP_LOGD`.  
* Aplicar throttling a refrescos UI si la frecuencia de datos es mayor que la utilidad (ej: 1/min para clima).  
* Validar cada retorno `esp_err_t` y loggear con contexto.  
* Crear "directorios" en SPIFFS sabiendo que es plano; tratar ENOTSUP como informativo.

### Errores Comunes
* Llamar API LVGL desde tarea que no es UI (prohibido).  
* Publicar evento no definido (debe estar en `event_system.h` y tabla de nombres).  
* HTTP bloqueante grande dentro de UI task (llevar a modelo).  
* Redibujos completos de clima duplicados en loop cerrado (respetar throttling).  
* Nuevo feature sin suscripción (UI no reacciona).  

### Depuración Rápida de Eventos
1. Confirmar log de `event_system_post` exitoso.  
2. Revisar estadísticas con `event_system_get_stats()`.  
3. Verificar suscripción de la cola UI (retorno de `event_system_subscribe_queue`).  
4. Inspeccionar switch de la UI task procesa el evento.  
5. Bridge obtiene datos válidos del modelo (flags).  
6. Objetos UI existen (no NULL).  
7. Agregar `ESP_LOGD` temporal dentro de updates si falta algo.

### Plantilla de Evento (Nuevo)
1. Agregar enum: `EVENT_SENSOR_DATA_READY` en `event_system.h`.  
2. Agregar string en el array de nombres de `event_system.c`.  
3. Suscribir en `task_manager_init()`.  
4. Postear desde data task: `event_system_post_simple(EVENT_SENSOR_DATA_READY, NULL, 0);`  
5. En UI task: case `EVENT_SENSOR_DATA_READY`: `ui_sensor_bridge_process_sensor_updates();`

### Flujo Ejemplo Mínimo (Nuevo Sensor)
```
sensor_task -> EVENT_SENSOR_DATA_READY
UI_Task -> ui_sensor_bridge_process_sensor_updates() -> reads app_sensor_get_info() -> updates labels/icons
```

### Notas de Memoria & Watchdog
* UI task corre periódicamente `lv_mem_monitor` (ver `task_manager.c`) — log si free < 1KB.  
* Data tasks alimentan watchdog en cada loop.  
* Operaciones largas: fraccionar con `vTaskDelay(pdMS_TO_TICKS(200))`.  
* Timeout HTTP weather reducido (5s). Evitar bloqueos >5s.

### Cuándo Regenerar la UI
* Nueva pantalla / widget / acción necesaria.  
* Renombrar IDs de acciones.  
Luego: regenerar con EEZ Studio → NO editar a mano `view/src_ui/`.

---

Esta sección extendida formaliza la arquitectura event-driven para mantener cambios futuros consistentes y libres de condiciones de carrera.


## Información Clave (Resumen Rápido)
* Proyecto ESP-IDF (target: esp32s3) con UI LVGL generada por EEZ Studio.
* Patrón arquitectónico: MVC + Bridge + Sistema de Eventos + Única tarea UI.
* Flujo datos: `screens.c` (EEZ) -> handlers -> `ui_*_bridge.c` -> `app_*.c` -> (estado) -> bridge procesa -> LVGL (solo UI task).
* Archivos base: `main/main.c`, `task_manager.c`, `event_system.c`, `app_weather.c`, `ui_weather_bridge.c`.
* NO editar `main/view/src_ui/` manualmente.
* Strings de UI en español dentro de bridges (mantener localización).
* Sólo UI task usa LVGL (lock interno con `bsp_display_lock`).
* Añadir features siguiendo checklist de la sección de arquitectura.

### Ejemplos Rápidos
Respeto de lock LVGL:
```
bsp_display_lock(0);
ui_tick();
ui_weather_bridge_process_weather_updates();
bsp_display_unlock();
```
Agregar controller:
```
// Crear archivo: main/controller/ui_sensor_bridge.c
// Implementar init + process + action handlers
```

### No Hacer
* Editar archivos en `view/src_ui/`.
* Llamar funciones LVGL fuera de la tarea UI.
* Introducir eventos sin agregarlos al enum + tabla nombres.
* Bloquear con HTTP largo dentro de UI task.

### Debug UI Inicial
1. Revisar logs en `monitor`.
2. Confirmar evento publicado (log en `event_system_post`).
3. Ver que UI task reciba y procese (switch case).
4. Verificar punteros LVGL no NULL.
5. Añadir `ESP_LOGD` en bridge si falta actualización.

Si algo sigue poco claro, ajustar UI en EEZ Studio en lugar de editar código generado.
