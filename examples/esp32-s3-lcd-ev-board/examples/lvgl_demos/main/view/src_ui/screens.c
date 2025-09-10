#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;

static void event_handler_cb_presentacion_presentacion(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_general_obj0(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 2, 0, e);
    }
}

static void event_handler_cb_general_obj1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 4, 0, e);
    }
}

static void event_handler_cb_trabajando_slider_potencia(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_slider_get_value(ta);
            assignIntegerProperty(flowState, 0, 3, value, "Failed to assign Value in Slider widget");
        }
    }
}

static void event_handler_cb_trabajando_boton_control_sesion(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 13, 0, e);
    }
}

static void event_handler_cb_config_actualizar_redes(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        action_wifi_update_list(e);
    }
}

static void event_handler_cb_config_cambiar_ubicacion(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 23, 0, e);
    }
}

static void event_handler_cb_config_obj2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 26, 0, e);
    }
}

static void event_handler_cb_ubicacion_resultado_geocoding(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 3, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_ubicacion_ciudad(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 7, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_ubicacion_boton_volver(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 8, 0, e);
    }
}

static void event_handler_cb_perfil_obj3(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
}

static void event_handler_cb_perfil_obj4(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

void create_screen_presentacion() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.presentacion = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_presentacion_presentacion, LV_EVENT_ALL, flowState);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 159, 82);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_logo_tt);
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 7, 77);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_iso_texel);
            lv_image_set_scale(obj, 200);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 153, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Presione para comenzar");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 0, 288);
            lv_obj_set_size(obj, 480, 192);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_presentacion();
}

void tick_screen_presentacion() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
}

void create_screen_general() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.general = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 0, 288);
            lv_obj_set_size(obj, 480, 192);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 74, 59);
            lv_obj_set_size(obj, 130, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_general_obj0, LV_EVENT_ALL, flowState);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Inicio Rapido");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 74, 207);
            lv_obj_set_size(obj, 130, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_general_obj1, LV_EVENT_ALL, flowState);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Configuracion");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 74, 133);
            lv_obj_set_size(obj, 130, 50);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Programas");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 480, 40);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffe8cb9a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Hora Local
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.hora_local = obj;
                    lv_obj_set_pos(obj, 410, -4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "12:00");
                }
            }
        }
    }
    
    tick_screen_general();
}

void tick_screen_general() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
}

void create_screen_trabajando() {
    void *flowState = getFlowState(0, 2);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.trabajando = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // SliderPotencia
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.slider_potencia = obj;
            lv_obj_set_pos(obj, 32, 246);
            lv_obj_set_size(obj, 416, 33);
            lv_obj_add_event_cb(obj, event_handler_cb_trabajando_slider_potencia, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 141, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Potencia");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 363, 101);
            lv_obj_set_size(obj, 100, 50);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // PotenciaValor
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.potencia_valor = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            lv_obj_set_pos(obj, 363, 30);
            lv_obj_set_size(obj, 100, 50);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // TiempoSesion
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.tiempo_sesion = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 0, 288);
            lv_obj_set_size(obj, 480, 192);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            lv_obj_set_pos(obj, 9, 23);
            lv_obj_set_size(obj, 192, LV_SIZE_CONTENT);
            lv_dropdown_set_options(obj, "Corporal\nFacial");
            lv_dropdown_set_dir(obj, LV_DIR_TOP);
            lv_dropdown_set_symbol(obj, LV_SYMBOL_UP);
            lv_dropdown_set_selected(obj, 0);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 10, 91);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Modo de emision");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 10, 7);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Canal de emision");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 353, 14);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Tiempo restante");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 381, 85);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Potencia");
        }
        {
            // Boton_Control_Sesion
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.boton_control_sesion = obj;
            lv_obj_set_pos(obj, 10, 170);
            lv_obj_set_size(obj, 136, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_trabajando_boton_control_sesion, LV_EVENT_ALL, flowState);
            add_style_estado_sesion_detenido(obj);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Label_Control_Sesion
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_control_sesion = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Detener");
                }
            }
        }
        {
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            lv_obj_set_pos(obj, 9, 107);
            lv_obj_set_size(obj, 197, LV_SIZE_CONTENT);
            lv_dropdown_set_options(obj, "Continuo\nIntermitente");
            lv_dropdown_set_dir(obj, LV_DIR_TOP);
            lv_dropdown_set_symbol(obj, LV_SYMBOL_UP);
            lv_dropdown_set_selected(obj, 0);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_trabajando();
}

void tick_screen_trabajando() {
    void *flowState = getFlowState(0, 2);
    (void)flowState;
    {
        int32_t new_val = evalIntegerProperty(flowState, 0, 3, "Failed to evaluate Value in Slider widget");
        int32_t cur_val = lv_slider_get_value(objects.slider_potencia);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.slider_potencia;
            lv_slider_set_value(objects.slider_potencia, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 4, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.potencia_valor);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.potencia_valor;
            lv_label_set_text(objects.potencia_valor, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 6, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.tiempo_sesion);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.tiempo_sesion;
            lv_label_set_text(objects.tiempo_sesion, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_config() {
    void *flowState = getFlowState(0, 3);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.config = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 0, 288);
            lv_obj_set_size(obj, 480, 192);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_tabview_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 480, 288);
            lv_tabview_set_tab_bar_position(obj, LV_DIR_LEFT);
            lv_tabview_set_tab_bar_size(obj, 80);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "Actividad");
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // TablaActividad
                            lv_obj_t *obj = lv_table_create(parent_obj);
                            objects.tabla_actividad = obj;
                            lv_obj_set_pos(obj, 4, 4);
                            lv_obj_set_size(obj, 359, 246);
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "WiFi");
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Actualizar Redes
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.actualizar_redes = obj;
                            lv_obj_set_pos(obj, 114, 206);
                            lv_obj_set_size(obj, 100, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_config_actualizar_redes, LV_EVENT_ALL, flowState);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "Actualizar");
                                }
                            }
                        }
                        {
                            // Conectar Red
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.conectar_red = obj;
                            lv_obj_set_pos(obj, 230, 206);
                            lv_obj_set_size(obj, 100, 50);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "Conectar");
                                }
                            }
                        }
                        {
                            // wifi_list
                            lv_obj_t *obj = lv_list_create(parent_obj);
                            objects.wifi_list = obj;
                            lv_obj_set_pos(obj, 38, -6);
                            lv_obj_set_size(obj, 292, 197);
                        }
                        {
                            lv_obj_t *obj = lv_switch_create(parent_obj);
                            lv_obj_set_pos(obj, 26, 206);
                            lv_obj_set_size(obj, 50, 25);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 2, 231);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Habilitar WIFI");
                        }
                    }
                }
                {
                    // Tab_Clima
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "Clima");
                    objects.tab_clima = obj;
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Actualizacion Clima
                            lv_obj_t *obj = lv_switch_create(parent_obj);
                            objects.actualizacion_clima = obj;
                            lv_obj_set_pos(obj, 54, 167);
                            lv_obj_set_size(obj, 50, 25);
                        }
                        {
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.obj10 = obj;
                            lv_obj_set_pos(obj, 26, 14);
                            lv_obj_set_size(obj, 128, 128);
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffb4f4f1), LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // Icono Clima Dinamico
                                    lv_obj_t *obj = lv_image_create(parent_obj);
                                    objects.icono_clima_dinamico = obj;
                                    lv_obj_set_pos(obj, -18, -18);
                                    lv_obj_set_size(obj, 128, 128);
                                    lv_image_set_src(obj, &img_wh_clear_day);
                                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_MOMENTUM);
                                }
                            }
                        }
                        {
                            // Etiqueta Temperatura
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.etiqueta_temperatura = obj;
                            lv_obj_set_pos(obj, 215, 60);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Temperatura");
                        }
                        {
                            // Label Actualizacion Clima
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_actualizacion_clima = obj;
                            lv_obj_set_pos(obj, 29, 192);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Mostrar Clima");
                        }
                        {
                            // Etiqueta Hora Medicion
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.etiqueta_hora_medicion = obj;
                            lv_obj_set_pos(obj, 212, 28);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Hora Medicion");
                        }
                        {
                            // Etiqueta Humedad
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.etiqueta_humedad = obj;
                            lv_obj_set_pos(obj, 225, 93);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Humedad");
                        }
                        {
                            // Etiqueta Condicion Ambiental
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.etiqueta_condicion_ambiental = obj;
                            lv_obj_set_pos(obj, 188, 126);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Condicion Ambiental");
                        }
                        {
                            // Label Ubicacion
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_ubicacion = obj;
                            lv_obj_set_pos(obj, 225, -2);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Ubicacion");
                        }
                        {
                            // Cambiar Ubicacion
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.cambiar_ubicacion = obj;
                            lv_obj_set_pos(obj, 215, 192);
                            lv_obj_set_size(obj, 100, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_config_cambiar_ubicacion, LV_EVENT_ALL, flowState);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "Cambiar\nUbicacion");
                                }
                            }
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "Volver");
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.obj2 = obj;
                            lv_obj_set_pos(obj, 115, 173);
                            lv_obj_set_size(obj, 100, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_config_obj2, LV_EVENT_ALL, flowState);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "Salir");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_config();
}

void tick_screen_config() {
    void *flowState = getFlowState(0, 3);
    (void)flowState;
}

void create_screen_ubicacion() {
    void *flowState = getFlowState(0, 4);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.ubicacion = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Teclado Ciudad
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.teclado_ciudad = obj;
            lv_obj_set_pos(obj, 0, -192);
            lv_obj_set_size(obj, 480, 174);
            lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_TEXT_UPPER);
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 0, 288);
            lv_obj_set_size(obj, 480, 192);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Resultado Geocoding
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.resultado_geocoding = obj;
            lv_obj_set_pos(obj, 169, 0);
            lv_obj_set_size(obj, 205, 114);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "Resultados...");
            lv_textarea_set_one_line(obj, false);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_event_cb(obj, event_handler_cb_ubicacion_resultado_geocoding, LV_EVENT_ALL, flowState);
        }
        {
            // Etiqueta Ciudad
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.etiqueta_ciudad = obj;
            lv_obj_set_pos(obj, 60, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Ciudad");
        }
        {
            // Etiqueta Pais
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.etiqueta_pais = obj;
            lv_obj_set_pos(obj, 71, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Pais");
        }
        {
            // Pais
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.pais = obj;
            lv_obj_set_pos(obj, 10, 16);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_dropdown_set_options(obj, "");
            lv_dropdown_set_selected(obj, 0);
        }
        {
            // Ciudad
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.ciudad = obj;
            lv_obj_set_pos(obj, 10, 74);
            lv_obj_set_size(obj, 150, 40);
            lv_textarea_set_max_length(obj, 20);
            lv_textarea_set_placeholder_text(obj, "Ingrese ciudad...");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_event_cb(obj, event_handler_cb_ubicacion_ciudad, LV_EVENT_ALL, flowState);
        }
        {
            // Boton Volver
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.boton_volver = obj;
            lv_obj_set_pos(obj, 377, 63);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_ubicacion_boton_volver, LV_EVENT_ALL, flowState);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Volver");
                }
            }
        }
        {
            // Boton Buscar
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.boton_buscar = obj;
            lv_obj_set_pos(obj, 377, 7);
            lv_obj_set_size(obj, 100, 50);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Buscar");
                }
            }
        }
    }
    lv_keyboard_set_textarea(objects.teclado_ciudad, objects.ciudad);
    
    tick_screen_ubicacion();
}

void tick_screen_ubicacion() {
    void *flowState = getFlowState(0, 4);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 3, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.resultado_geocoding);
        uint32_t max_length = lv_textarea_get_max_length(objects.resultado_geocoding);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.resultado_geocoding;
            lv_textarea_set_text(objects.resultado_geocoding, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalStringArrayPropertyAndJoin(flowState, 6, 3, "Failed to evaluate Options in Dropdown widget", "\n");
        const char *cur_val = lv_dropdown_get_options(objects.pais);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.pais;
            lv_dropdown_set_options(objects.pais, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 7, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.ciudad);
        uint32_t max_length = lv_textarea_get_max_length(objects.ciudad);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.ciudad;
            lv_textarea_set_text(objects.ciudad, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_perfil() {
    void *flowState = getFlowState(0, 5);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.perfil = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 0, 138);
            lv_obj_set_size(obj, 480, 150);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // TA1
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.ta1 = obj;
            lv_obj_set_pos(obj, 17, 17);
            lv_obj_set_size(obj, 150, 70);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_one_line(obj, false);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 229, 17);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_perfil_obj3, LV_EVENT_ALL, flowState);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Iniciar");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 361, 17);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_perfil_obj4, LV_EVENT_ALL, flowState);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Iniciar");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 0, 288);
            lv_obj_set_size(obj, 480, 192);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    lv_keyboard_set_textarea(objects.obj13, objects.ta1);
    
    tick_screen_perfil();
}

void tick_screen_perfil() {
    void *flowState = getFlowState(0, 5);
    (void)flowState;
}


extern void add_style(lv_obj_t *obj, int32_t styleIndex);
extern void remove_style(lv_obj_t *obj, int32_t styleIndex);

static const char *screen_names[] = { "Presentacion", "General", "Trabajando", "Config", "Ubicacion", "Perfil" };
static const char *object_names[] = { "presentacion", "general", "trabajando", "config", "ubicacion", "perfil", "obj0", "obj1", "boton_control_sesion", "actualizar_redes", "cambiar_ubicacion", "obj2", "boton_volver", "obj3", "obj4", "obj5", "obj6", "obj7", "hora_local", "slider_potencia", "potencia_valor", "tiempo_sesion", "obj8", "label_control_sesion", "obj9", "tabla_actividad", "conectar_red", "wifi_list", "tab_clima", "actualizacion_clima", "obj10", "icono_clima_dinamico", "etiqueta_temperatura", "label_actualizacion_clima", "etiqueta_hora_medicion", "etiqueta_humedad", "etiqueta_condicion_ambiental", "label_ubicacion", "teclado_ciudad", "obj11", "resultado_geocoding", "etiqueta_ciudad", "etiqueta_pais", "pais", "ciudad", "boton_buscar", "ta1", "obj12", "obj13" };
static const char *style_names[] = { "Estado_Sesion_Iniciado", "Estado_Sesion_Detenido", "KB-Code" };


typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_presentacion,
    tick_screen_general,
    tick_screen_trabajando,
    tick_screen_config,
    tick_screen_ubicacion,
    tick_screen_perfil,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    eez_flow_init_styles(add_style, remove_style);
    
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    eez_flow_init_style_names(style_names, sizeof(style_names) / sizeof(const char *));
    
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_presentacion();
    create_screen_general();
    create_screen_trabajando();
    create_screen_config();
    create_screen_ubicacion();
    create_screen_perfil();
}
