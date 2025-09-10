#include "images.h"

const ext_img_desc_t images[15] = {
    { "TeslaGen", &img_tesla_gen },
    { "IsoTEXEL", &img_iso_texel },
    { "Logo-TT", &img_logo_tt },
    { "wh_clear-day", &img_wh_clear_day },
    { "wh_clear-night", &img_wh_clear_night },
    { "wh_cloudy", &img_wh_cloudy },
    { "wh_drizzle", &img_wh_drizzle },
    { "wh_fog", &img_wh_fog },
    { "wh_partly-cloudy-day", &img_wh_partly_cloudy_day },
    { "wh_partly-cloudy-night", &img_wh_partly_cloudy_night },
    { "wh_rain", &img_wh_rain },
    { "wh_sleet", &img_wh_sleet },
    { "wh_snow", &img_wh_snow },
    { "wh_thunderstorms", &img_wh_thunderstorms },
    { "wh_thunderstorms-rain", &img_wh_thunderstorms_rain },
};
