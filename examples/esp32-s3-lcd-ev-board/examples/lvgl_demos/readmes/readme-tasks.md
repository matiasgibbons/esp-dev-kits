# Sistema ESP32-S3 LCD - Definición de Alcance y Arquitectura de Tareas

## Visión General del Sistema

Sistema embebido ESP32-S3 con display LCD para control de dispositivos, con conectividad múltiple (WiFi/BLE), actualizaciones OTA, comunicación con hardware externo y aplicación móvil.

---

## Arquitectura de Tareas Principales

### **NETWORK_TASK** (Conectividad WiFi y Servicios de Red)

#### 1. Conexión WiFi
- **a.** Estado (conectado/desconectado/escaneando)
- **b.** Conectar (automático/manual, rotación de redes)
- **c.** Desconectar
- **d.** Escaneo de redes disponibles
- **e.** Gestión de credenciales múltiples (maximo 3, 1 fallback hardcoded baja prioridad)

#### 2. Servicio SNTP
- **a.** Sincronización inicial de hora
- **b.** Re-sincronización periódica
- **c.** Manejo de timezone y DST

#### 3. Servicio Weather (API Open-Meteo)
- **a.** Búsqueda de geolocalización (país/ciudad → lat/lng)
- **b.** Actualización de datos climáticos (5 minutos, opcion desactivar)
- **c.** Cache local y validación de datos
- **d.** Throttling de requests

#### 4. OTA (Over-The-Air Updates)
- **a.** Ping a servidor de actualizaciones
- **b.** Verificación de versiones disponibles
- **c.** Descarga de firmware
- **d.** Validación e instalación
- **e.** Rollback en caso de error
- **f.** Manejo de comandos recibidos (bloquear sistema, forzar actualizacion, etc)

#### 5. Conexión con App Móvil (HTTP/WebSocket)
- **a.** Registro/asociación de dispositivo
- **b.** Envío de estado del sistema
- **c.** Recepción de configuración remota
- **d.** Recepción de comandos remotos
- **e.** Autenticación y seguridad

---

### **BLE_TASK** (Conectividad Bluetooth Low Energy)

#### 0. Conexión BLE
- **a.** Estado (advertising/conectado/desconectado)
- **b.** Conectar (pairing automático/manual)
- **c.** Desconectar
- **d.** Gestión de múltiples conexiones (solo una permitida)

#### 1. Conexión con App Móvil
- **a.** Asociación y pairing
- **b.** Envío de estado del dispositivo
- **c.** Recepción de datos de configuración
- **d.** Recepción de comandos de control
- **e.** Notificaciones push locales

#### 2. Beacon y Discovery (IDEA FUTURO)
- **a.** Modo beacon para localización
- **b.** Discovery de otros dispositivos cercanos
- **c.** Mesh networking básico

---

### **USART_TASK** (Comunicación con Hardware Externo)

#### 0. Comunicación Serial
- **a.** Envío de estado del sistema
- **b.** Recepción de estado del hardware
- **c.** Envío de comandos de control
- **d.** Validación de integridad de datos (CRC8/16)
- **e.** Manejo de timeouts y reintentos
- **f.** Protocolo de handshake

---

### **DEVICE_TASK** (Lógica Principal del Dispositivo)

#### 0. Estado Detenido (Standby)
- **a.** Monitoreo básico de hardware externo (GPIO o USART)
- **b.** Modo de bajo consumo
- **c.** Watchdog de sistema

#### 1. Estado Operativo (Running)
- **a.** Control activo de hardware (GPIO o USART)
- **b.** Procesamiento de comunicación USART
- **c.** Logging de actividad y eventos
- **d.** Algoritmos de control (PID, lógica de estado)
- **e.** Detección de anomalías
- **f.** Autodiagnóstico

#### 2. Estado Pausado (Paused)
- **a.** Mantenimiento de estado de hardware
- **b.** Preservación de contexto
- **c.** Preparación para reanudación

#### 3. Estado de Mantenimiento
- **a.** Calibración de sensores
- **b.** Autotest de componentes
- **c.** Limpieza de datos temporales

---

### **SYSTEM_TASK** (Gestión del Sistema)

#### 0. Gestión de Memoria y Almacenamiento
- **a.** Carga de configuración desde NVS/SPIFFS
- **b.** Guardado de configuración y datos
- **c.** Limpieza automática de archivos temporales
- **d.** Monitoreo de espacio disponible
- **e.** Backup y restauración

#### 1. Monitoreo del Sistema
- **a.** Supervisión de uso de memoria RAM
- **b.** Monitoreo de temperatura del SoC
- **c.** Verificación de voltajes de alimentación
- **d.** Estadísticas de tareas (CPU usage, stack usage)

#### 2. Logging y Diagnóstico
- **a.** Sistema de logs centralizado
- **b.** Rotación automática de archivos de log
- **c.** Envío de logs críticos vía red
- **d.** Métricas y telemetría del sistema

#### 3. Gestión de Energía (FUTURO)
- **a.** Control de modos de bajo consumo
- **b.** Gestión de wake-up sources
- **c.** Optimización de frecuencias de CPU
---

### **UI_TASK** (Interfaz de Usuario)

#### 0. Gestión de Display
- **a.** Actualización de pantallas y widgets
- **b.** Procesamiento de eventos de touch
- **c.** Gestión de brillo automático
- **d.** Screensaver y timeout

#### 1. Interacción con Usuario
- **a.** Navegación entre pantallas (EEZ Studio/Flow)
- **b.** Configuración local de parámetros
- **c.** Visualización de estado del sistema
- **d.** Alertas y notificaciones visuales

Nota: La UI_TASK está basada en el código generado por EEZ Studio y utiliza LVGL para la gestión de pantallas y widgets. Toda la lógica de presentación y actualización de la interfaz se realiza exclusivamente en esta tarea, siguiendo el patrón de hilo único de UI.

---

## Subtareas y Funcionalidades Críticas

### **Comunicación Inter-Tarea**
- Sistema de eventos centralizado (ya implementado)
- Sincronización entre tareas críticas
- Manejo de prioridades y deadlines

### **Robustez y Confiabilidad**
- Sistema de watchdog multinivel
- Recuperación automática de fallos
- Validación de integridad de datos
- Mecanismos de retry y fallback

### **Configuración y Personalización**
- Sistema de configuración por perfiles
- Importación/exportación de configuraciones
- Configuración remota vía app
- Modo de configuración inicial (first boot)

### **Seguridad**
- Gestión de certificados TLS/SSL
- Encriptación de datos sensibles
- Autenticación de dispositivos
- Gestión de tokens de acceso

---

## Flujo de Comunicación Entre Tareas

```
[UI_TASK] ←→ [EVENT_SYSTEM] ←→ [DEVICE_TASK]
                    ↕
[NETWORK_TASK] ←→ [SYSTEM_TASK] ←→ [USART_TASK]
                    ↕
                [BLE_TASK]
```

---

## Consideraciones de Implementación

### **Prioridades de Tareas**
- **UI_TASK**: Prioridad 10 (máxima) - Responsividad crítica
- **DEVICE_TASK**: Prioridad 8 - Control en tiempo real
- **USART_TASK**: Prioridad 7 - Comunicación hardware
- **SYSTEM_TASK**: Prioridad 6 - Monitoreo y gestión
- **NETWORK_TASK**: Prioridad 5 - Conectividad WiFi
- **BLE_TASK**: Prioridad 4 - Conectividad alternativa

### **Asignación de Cores**
- **Core 1**: UI_TASK (exclusivo para LVGL)
- **Core 0**: Resto de tareas (balanceado por FreeRTOS)

### **Comunicación**
- **Sistema de eventos publish/subscribe** con colas dedicadas
- **Eventos tipados** con payload mínimo
- **Sin llamadas directas** entre tareas

### **Persistencia**
- **NVS**: Configuración del sistema y credenciales
- **SPIFFS**: Logs, cache de datos y archivos temporales

### **Conectividad**
- **WiFi**: Conectividad primaria para servicios de red
- **BLE**: Conectividad secundaria y configuración local

### **Actualizaciones**
- **OTA con validación** y rollback automático
- **Descarga en background** sin interrumpir operación

---

## Estados del Sistema

### **Estado de Inicialización**
1. Carga de configuración desde NVS
2. Inicialización de hardware y periféricos
3. Arranque de tareas en orden de prioridad
4. Conexión automática a redes conocidas o fallback

### **Estado Operativo Normal**
1. Todas las tareas ejecutándose según programación
2. Conectividad establecida (WiFi/BLE)
3. Comunicación activa con hardware externo
4. UI responsiva y actualizada

### **Estado de Mantenimiento**
1. Suspensión de operaciones normales
2. Ejecución de rutinas de diagnóstico
3. Limpieza y optimización de datos
4. Calibración de sensores

### **Estado de Emergencia**
1. Suspensión de tareas no críticas
2. Preservación de datos críticos
3. Notificación de estado via todos los canales
4. Preparación para reinicio controlado

### **Estado de Bajo Consumo** (FUTURO)
1. Reducción de frecuencias de CPU
2. Suspensión de tareas no críticas
3. Monitoreo mínimo de sistema
4. Wake-up por eventos externos

---

## Métricas y Monitoreo

### **Recursos del Sistema**
- Uso de memoria RAM (heap libre, fragmentación)
- Uso de almacenamiento (NVS, SPIFFS)
- Uso de CPU por tarea
- Temperatura del SoC

### **Conectividad**
- Estado de conexiones (WiFi, BLE)
- Calidad de señal y latencia
- Tasa de éxito de requests de red
- Errores de comunicación

### **Operación del Dispositivo**
- Tiempo de operación continua
- Ciclos de trabajo completados
- Errores de hardware detectados
- Eventos de mantenimiento

### **Interfaz de Usuario**
- Tiempo de respuesta de UI
- Interacciones por sesión
- Pantallas más utilizadas
- Errores de navegación

---

## Plan de Implementación

### **Fase 1: Core System**
- EVENT_SYSTEM (✅ Implementado)
- UI_TASK básica (✅ Implementado)
- SYSTEM_TASK básica (✅ Implementado)

### **Fase 2: Connectivity**
- NETWORK_TASK (🔄 En progreso)
  - WiFi básico (✅ Implementado)
  - SNTP (✅ Implementado)  
  - Weather (✅ Implementado)
- BLE_TASK básica

### **Fase 3: Device Control**
- DEVICE_TASK
- USART_TASK
- Estados básicos del dispositivo

### **Fase 4: Advanced Features**
- OTA updates
- App móvil connectivity
- Configuración remota
- Diagnósticos avanzados

### **Fase 5: Optimization**
- Métricas avanzadas
- Seguridad mejorada
- Performance tuning
- Bajo consumo

---

Este documento define el alcance completo del sistema y sirve como guía para el desarrollo y mantenimiento de la arquitectura de tareas.