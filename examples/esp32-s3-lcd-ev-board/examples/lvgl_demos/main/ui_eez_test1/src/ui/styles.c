#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: Estado_Sesion_Iniciado
//

void init_style_estado_sesion_iniciado_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xfff32121));
};

lv_style_t *get_style_estado_sesion_iniciado_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_estado_sesion_iniciado_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_estado_sesion_iniciado(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_estado_sesion_iniciado_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_estado_sesion_iniciado(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_estado_sesion_iniciado_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: Estado_Sesion_Detenido
//

void init_style_estado_sesion_detenido_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff27f321));
};

lv_style_t *get_style_estado_sesion_detenido_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_estado_sesion_detenido_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_estado_sesion_detenido(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_estado_sesion_detenido_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_estado_sesion_detenido(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_estado_sesion_detenido_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_estado_sesion_iniciado,
        add_style_estado_sesion_detenido,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_estado_sesion_iniciado,
        remove_style_estado_sesion_detenido,
    };
    remove_style_funcs[styleIndex](obj);
}

