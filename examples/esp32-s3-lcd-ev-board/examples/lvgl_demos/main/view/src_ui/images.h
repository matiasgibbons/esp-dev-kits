#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_tesla_gen;
extern const lv_img_dsc_t img_iso_texel;
extern const lv_img_dsc_t img_logo_tt;
extern const lv_img_dsc_t img_wh_clear_day;
extern const lv_img_dsc_t img_wh_clear_night;
extern const lv_img_dsc_t img_wh_cloudy;
extern const lv_img_dsc_t img_wh_drizzle;
extern const lv_img_dsc_t img_wh_fog;
extern const lv_img_dsc_t img_wh_partly_cloudy_day;
extern const lv_img_dsc_t img_wh_partly_cloudy_night;
extern const lv_img_dsc_t img_wh_rain;
extern const lv_img_dsc_t img_wh_sleet;
extern const lv_img_dsc_t img_wh_snow;
extern const lv_img_dsc_t img_wh_thunderstorms;
extern const lv_img_dsc_t img_wh_thunderstorms_rain;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[15];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/