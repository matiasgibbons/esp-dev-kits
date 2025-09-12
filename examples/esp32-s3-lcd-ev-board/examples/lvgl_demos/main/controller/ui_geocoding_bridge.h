/*
 * Header for geocoding/location search bridge functionality.
 */

#ifndef UI_GEOCODING_BRIDGE_H
#define UI_GEOCODING_BRIDGE_H

#include <stdint.h>
#include "lvgl.h"
#include "app_weather.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialization
void ui_geocoding_bridge_init(void);

// Geocoding workflow functions
void ui_geocoding_bridge_search_location(const char* country_code, const char* city);
void ui_geocoding_bridge_show_results(void);
void ui_geocoding_bridge_select_result(int result_index);

// Helper display functions
void ui_geocoding_bridge_clear_results(void);
void ui_geocoding_bridge_show_searching(void);
void ui_geocoding_bridge_show_error(const char* error_message);

// Helper functions for country data
const char *get_codigo_pais_by_name(const char *name);
const char *get_codigo_pais_by_index(int index);
const char *get_nombre_pais_by_index(int index);

// EEZ Studio action implementations
void action_wh_find_geocoding(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif // UI_GEOCODING_BRIDGE_H
