#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: Estado_Sesion_Iniciado
lv_style_t *get_style_estado_sesion_iniciado_MAIN_DEFAULT();
void add_style_estado_sesion_iniciado(lv_obj_t *obj);
void remove_style_estado_sesion_iniciado(lv_obj_t *obj);

// Style: Estado_Sesion_Detenido
lv_style_t *get_style_estado_sesion_detenido_MAIN_DEFAULT();
void add_style_estado_sesion_detenido(lv_obj_t *obj);
void remove_style_estado_sesion_detenido(lv_obj_t *obj);

// Style: KB-Code
void add_style_kb_code(lv_obj_t *obj);
void remove_style_kb_code(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/