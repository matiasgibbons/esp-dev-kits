# TASK_NETWORK - Alcance y Especificación

Este documento define el alcance, responsabilidades y subtareas de la **TASK_NETWORK** en el sistema ESP32-S3 LCD. Sirve como guía para el desarrollo, integración y mantenimiento de la lógica de conectividad y servicios de red.

---

## 1. Conexión WiFi

- **a.** Estado (conectado/desconectado/escaneando)
- **b.** Conectar (automático/manual, rotación de redes)
- **c.** Desconectar
- **d.** Escaneo de redes disponibles
- **e.** Gestión de credenciales múltiples (máximo 3, 1 fallback hardcoded baja prioridad)

---

## 2. Servicio SNTP

- **a.** Sincronización inicial de hora
- **b.** Re-sincronización periódica
- **c.** Manejo de timezone y DST

---

## 3. Servicio Weather (API Open-Meteo)

- **a.** Búsqueda de geolocalización (país/ciudad → lat/lng)
- **b.** Actualización de datos climáticos (cada 5 minutos, opción desactivar)
- **c.** Cache local y validación de datos
- **d.** Throttling de requests

---

## 4. OTA (Over-The-Air Updates)

- **a.** Ping a servidor de actualizaciones
- **b.** Verificación de versiones disponibles
- **c.** Descarga de firmware
- **d.** Validación e instalación
- **e.** Rollback en caso de error
- **f.** Manejo de comandos recibidos (bloquear sistema, forzar actualización, etc)

---

## 5. Conexión con App Móvil (HTTP/WebSocket)

- **a.** Registro/asociación de dispositivo
- **b.** Envío de estado del sistema
- **c.** Recepción de configuración remota
- **d.** Recepción de comandos remotos
- **e.** Autenticación y seguridad

---

## Consideraciones de Implementación

- **Sistema de eventos publish/subscribe:** Todos los cambios de estado y resultados relevantes deben publicarse como eventos tipados (`system_event_type_t`).
- **Sin llamadas directas entre tareas:** La comunicación se realiza exclusivamente mediante eventos.
- **Payload mínimo en eventos:** Los datos grandes deben residir en el modelo; los eventos solo llevan referencias o flags.
- **Logs y comentarios en español:** Usar `ESP_LOGI` para eventos importantes y `ESP_LOGD` para depuración.
- **Throttling:** Limitar la frecuencia de requests a servicios externos para evitar bloqueos y sobrecarga.
- **Validación de errores:** Validar cada retorno `esp_err_t` y loggear con contexto.
- **Seguridad:** Gestionar autenticación, certificados y cifrado para todas las comunicaciones externas.

---

## Ejemplo de Eventos Publicados

- `EVENT_WIFI_STATE_CHANGED`
- `EVENT_WIFI_SCAN_START`
- `EVENT_WIFI_SCAN_COMPLETE`
- `EVENT_WIFI_CONNECTED`
- `EVENT_WIFI_DISCONNECTED`
- `EVENT_SNTP_SYNC_START`
- `EVENT_SNTP_SYNC_COMPLETE`
- `EVENT_WEATHER_FETCH_START`
- `EVENT_WEATHER_DATA_READY`
- `EVENT_OTA_UPDATE_AVAILABLE`
- `EVENT_OTA_UPDATE_SUCCESS`
- `EVENT_APP_COMMAND_RECEIVED`

---

## Ejemplo de Eventos Suscritos

- `EVENT_WIFI_CONNECT_REQUEST`
- `EVENT_WIFI_DISCONNECT_REQUEST`
- `EVENT_WIFI_SCAN_REQUEST`
- `EVENT_SNTP_RESYNC_REQUEST`
- `EVENT_WEATHER_UPDATE_REQUESTED`
- `EVENT_OTA_UPDATE_REQUESTED`
- `EVENT_APP_COMMAND_SEND`
- `EVENT_SETTINGS_CHANGED`

---

## Notas

- La **TASK_NETWORK** es responsable de toda la conectividad y servicios de red, pero nunca accede directamente a la UI (LVGL).
- Todas las actualizaciones de estado y resultados deben ser comunicados a otras tareas mediante el sistema de eventos.
- El diseño desacoplado permite agregar o modificar servicios de red sin afectar el resto del sistema.

---