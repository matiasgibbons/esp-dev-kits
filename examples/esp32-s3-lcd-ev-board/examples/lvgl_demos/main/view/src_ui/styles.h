#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: Boton Defecto
lv_style_t *get_style_boton_defecto_MAIN_DEFAULT();
void add_style_boton_defecto(lv_obj_t *obj);
void remove_style_boton_defecto(lv_obj_t *obj);

// Style: Boton Rojo
lv_style_t *get_style_boton_rojo_MAIN_DEFAULT();
void add_style_boton_rojo(lv_obj_t *obj);
void remove_style_boton_rojo(lv_obj_t *obj);

// Style: Boton Verde
lv_style_t *get_style_boton_verde_MAIN_DEFAULT();
void add_style_boton_verde(lv_obj_t *obj);
void remove_style_boton_verde(lv_obj_t *obj);

// Style: KB-Code
void add_style_kb_code(lv_obj_t *obj);
void remove_style_kb_code(lv_obj_t *obj);

// Style: Label <h1>
lv_style_t *get_style_label__h1__MAIN_DEFAULT();
void add_style_label__h1_(lv_obj_t *obj);
void remove_style_label__h1_(lv_obj_t *obj);

// Style: Label <h2>
lv_style_t *get_style_label__h2__MAIN_DEFAULT();
void add_style_label__h2_(lv_obj_t *obj);
void remove_style_label__h2_(lv_obj_t *obj);

// Style: f24
lv_style_t *get_style_f24_MAIN_DEFAULT();
void add_style_f24(lv_obj_t *obj);
void remove_style_f24(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/