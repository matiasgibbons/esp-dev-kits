#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *presentacion;
    lv_obj_t *general;
    lv_obj_t *trabajando;
    lv_obj_t *config;
    lv_obj_t *perfil;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *boton_control_sesion;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *slider_potencia;
    lv_obj_t *potencia_valor;
    lv_obj_t *tiempo_sesion;
    lv_obj_t *obj7;
    lv_obj_t *label_control_sesion;
    lv_obj_t *obj8;
    lv_obj_t *tabla_actividad;
    lv_obj_t *wifi_list;
    lv_obj_t *ta1;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_PRESENTACION = 1,
    SCREEN_ID_GENERAL = 2,
    SCREEN_ID_TRABAJANDO = 3,
    SCREEN_ID_CONFIG = 4,
    SCREEN_ID_PERFIL = 5,
};

void create_screen_presentacion();
void tick_screen_presentacion();

void create_screen_general();
void tick_screen_general();

void create_screen_trabajando();
void tick_screen_trabajando();

void create_screen_config();
void tick_screen_config();

void create_screen_perfil();
void tick_screen_perfil();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/