#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: Boton Defecto
//

void init_style_boton_defecto_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff2196f3));
};

lv_style_t *get_style_boton_defecto_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_boton_defecto_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_boton_defecto(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_boton_defecto_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_boton_defecto(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_boton_defecto_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: Boton Rojo
//

void init_style_boton_rojo_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xfff32121));
};

lv_style_t *get_style_boton_rojo_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_boton_rojo_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_boton_rojo(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_boton_rojo_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_boton_rojo(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_boton_rojo_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: Boton Verde
//

void init_style_boton_verde_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff27f321));
};

lv_style_t *get_style_boton_verde_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_boton_verde_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_boton_verde(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_boton_verde_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_boton_verde(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_boton_verde_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: KB-Code
//

void add_style_kb_code(lv_obj_t *obj) {
    (void)obj;
};

void remove_style_kb_code(lv_obj_t *obj) {
    (void)obj;
};

//
// Style: Label <h1>
//

void init_style_label__h1__MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &lv_font_montserrat_26);
    lv_style_set_align(style, LV_ALIGN_CENTER);
};

lv_style_t *get_style_label__h1__MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_label__h1__MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_label__h1_(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_label__h1__MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_label__h1_(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_label__h1__MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: Label <h2>
//

void init_style_label__h2__MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &lv_font_montserrat_14);
    lv_style_set_align(style, LV_ALIGN_CENTER);
};

lv_style_t *get_style_label__h2__MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_label__h2__MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_label__h2_(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_label__h2__MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_label__h2_(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_label__h2__MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: f24
//

void init_style_f24_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &lv_font_montserrat_24);
};

lv_style_t *get_style_f24_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_f24_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_f24(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_f24_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_f24(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_f24_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_boton_defecto,
        add_style_boton_rojo,
        add_style_boton_verde,
        add_style_kb_code,
        add_style_label__h1_,
        add_style_label__h2_,
        add_style_f24,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_boton_defecto,
        remove_style_boton_rojo,
        remove_style_boton_verde,
        remove_style_kb_code,
        remove_style_label__h1_,
        remove_style_label__h2_,
        remove_style_f24,
    };
    remove_style_funcs[styleIndex](obj);
}

