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
    lv_obj_t *ubicacion;
    lv_obj_t *perfil;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *boton_control_sesion;
    lv_obj_t *actualizar_redes;
    lv_obj_t *cambiar_ubicacion;
    lv_obj_t *obj2;
    lv_obj_t *boton_volver;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *hora_local;
    lv_obj_t *slider_potencia;
    lv_obj_t *potencia_valor;
    lv_obj_t *tiempo_sesion;
    lv_obj_t *obj8;
    lv_obj_t *label_control_sesion;
    lv_obj_t *obj9;
    lv_obj_t *tabla_actividad;
    lv_obj_t *conectar_red;
    lv_obj_t *wifi_list;
    lv_obj_t *tab_clima;
    lv_obj_t *actualizacion_clima;
    lv_obj_t *obj10;
    lv_obj_t *icono_clima_dinamico;
    lv_obj_t *etiqueta_temperatura;
    lv_obj_t *label_actualizacion_clima;
    lv_obj_t *etiqueta_hora_medicion;
    lv_obj_t *etiqueta_humedad;
    lv_obj_t *etiqueta_condicion_ambiental;
    lv_obj_t *label_ubicacion;
    lv_obj_t *teclado_ciudad;
    lv_obj_t *obj11;
    lv_obj_t *resultado_geocoding;
    lv_obj_t *etiqueta_ciudad;
    lv_obj_t *etiqueta_pais;
    lv_obj_t *pais;
    lv_obj_t *ciudad;
    lv_obj_t *boton_buscar;
    lv_obj_t *ta1;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_PRESENTACION = 1,
    SCREEN_ID_GENERAL = 2,
    SCREEN_ID_TRABAJANDO = 3,
    SCREEN_ID_CONFIG = 4,
    SCREEN_ID_UBICACION = 5,
    SCREEN_ID_PERFIL = 6,
};

void create_screen_presentacion();
void tick_screen_presentacion();

void create_screen_general();
void tick_screen_general();

void create_screen_trabajando();
void tick_screen_trabajando();

void create_screen_config();
void tick_screen_config();

void create_screen_ubicacion();
void tick_screen_ubicacion();

void create_screen_perfil();
void tick_screen_perfil();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/