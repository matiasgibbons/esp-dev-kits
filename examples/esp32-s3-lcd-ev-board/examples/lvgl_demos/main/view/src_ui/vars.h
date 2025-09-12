#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_POTENCIA = 0,
    FLOW_GLOBAL_VARIABLE_TIEMPO_SESION = 1,
    FLOW_GLOBAL_VARIABLE_ESTADO_SESION = 2,
    FLOW_GLOBAL_VARIABLE_WEATHER_CODE = 3,
    FLOW_GLOBAL_VARIABLE_WH_GEOCODING_CITY = 4
};

// Native global variables

extern const char *get_var_listado_paises();
extern void set_var_listado_paises(const char *value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/