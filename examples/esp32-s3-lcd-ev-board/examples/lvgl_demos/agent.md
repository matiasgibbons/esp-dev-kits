# Arquitectura MVC para ESP32 LVGL + EEZ Studio

## Estructura

**VISTA:** screens.c (autogenerado por EEZ Studio) → carpeta `\main\view\src_ui\`  
**CONTROLADOR:** ui_*_bridge.c → carpeta `\main\controller\`  
**MODELO:** app_*.c → carpeta `\main\model\`  

## Flujo de Trabajo

### En EEZ Studio:
1. Usuario humano diseña pantallas UI visualmente
2. Agregar **Acciones** para eventos de usuario:
   - `action_pais_onchange`
   - `action_codigo_postal_onchange`

3. EEZ autogenera `screens.c` con manejadores de eventos

> **Importante:** Nunca modificar archivos de la carpeta `\main\view\src_ui\`. Si falta algún control, evento, label o botón, solicitar al usuario que lo agregue desde EEZ Studio.

### Flujo de Conexión:
```
screens.c → event_handler_cb_* → action_*_onchange → ui_*_bridge.c → app_*.c
```

### Patrón Bridge (Controller):
- **ui_wifi_bridge.c**: Controla WiFi UI ↔ app_wifi.c
- **ui_weather_bridge.c**: Controla Clima UI ↔ app_weather.c
- Cada bridge implementa las **acciones declaradas en EEZ Studio**

El agente no debe crear archivos auxiliares sin consentimiento del usuario humano.
El sistema se crea en idioma español.

**Comentarios:**
- Siempre en español
- Preferentemente in-line o como título de bloque