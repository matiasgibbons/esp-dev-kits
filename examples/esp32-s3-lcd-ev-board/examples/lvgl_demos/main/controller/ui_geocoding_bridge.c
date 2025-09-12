/*
 * Geocoding and location search bridge for Pantalla ubicacion.
 * Implements complete workflow: Country -> City -> Search -> Results -> Select -> Persist -> Return
 */

#include "ui_geocoding_bridge.h"
#include "app_weather.h"
#include "esp_log.h"
#include <string.h>

// Incluir headers de EEZ Studio para acceder a objects
#include "screens.h"
#include "vars.h"
#include "ui.h"

static const char *TAG = "ui_geocoding_bridge";

// Forward declarations
static void geocoding_result_item_clicked(lv_event_t *e);

// State tracking for the geocoding workflow
static struct {
    geocoding_location_t search_results[5]; // Up to 5 search results
    int num_results;                  // Number of results found
    int selected_result_index;        // Index of selected result (-1 if none)
    bool search_in_progress;          // Search operation ongoing
} geocoding_state = {
    .num_results = 0,
    .selected_result_index = -1,
    .search_in_progress = false
};

// Buffer to hold the current listado_paises string
static char listado_paises_buf[1024];
static int listado_paises_built = 0;

// Buffer to hold geocoding results
static char geocoding_results_buf[1024];

// Static table of Latin American countries (Spanish display name + ISO code)
static const struct {
    const char *name;
    const char *code;
} paises[] = {
    { "Argentina", "AR" },
    { "Bolivia", "BO" },
    { "Brasil", "BR" },
    { "Chile", "CL" },
    { "Colombia", "CO" },
    { "Costa Rica", "CR" },
    { "Cuba", "CU" },
    { "Ecuador", "EC" },
    { "El Salvador", "SV" },
    { "Guatemala", "GT" },
    { "Honduras", "HN" },
    { "México", "MX" },
    { "Nicaragua", "NI" },
    { "Panamá", "PA" },
    { "Paraguay", "PY" },
    { "Perú", "PE" },
    { "Puerto Rico", "PR" },
    { "República Dominicana", "DO" },
    { "Uruguay", "UY" },
    { "Venezuela", "VE" }
};

// ============================================================================
// Country list management
// ============================================================================

static void build_listado_paises(void)
{
    char *p = listado_paises_buf;
    size_t rem = sizeof(listado_paises_buf);
    int written = 0;
    size_t n = sizeof(paises) / sizeof(paises[0]);

    for (size_t i = 0; i < n; ++i) {
        const char *name = paises[i].name;
        if (rem == 0) break;
        written = snprintf(p, rem, "%s", name);
        if (written < 0) break;
        size_t w = (size_t)written;
        if (w >= rem) { p[rem - 1] = '\0'; rem = 0; break; }
        p += w; rem -= w;
        if (i + 1 < n && rem > 1) { *p++ = '\n'; *p = '\0'; rem -= 1; }
    }

    listado_paises_built = 1;
}

const char *get_var_listado_paises()
{
    if (!listado_paises_built) build_listado_paises();
    return (const char *)listado_paises_buf;
}

void set_var_listado_paises(const char *value)
{
    if (value == NULL) {
        listado_paises_buf[0] = '\0';
        listado_paises_built = 1;
        return;
    }
    strncpy(listado_paises_buf, value, sizeof(listado_paises_buf) - 1);
    listado_paises_buf[sizeof(listado_paises_buf) - 1] = '\0';
    listado_paises_built = 1;
}

const char *get_var_wh_geocoding_results()
{
    return (const char *)geocoding_results_buf;
}

void set_var_wh_geocoding_results(const char *value)
{
    if (value == NULL) {
        geocoding_results_buf[0] = '\0';
        return;
    }
    strncpy(geocoding_results_buf, value, sizeof(geocoding_results_buf) - 1);
    geocoding_results_buf[sizeof(geocoding_results_buf) - 1] = '\0';
}

// ============================================================================
// Lista de resultados management 
// ============================================================================

static void clear_geocoding_results_list(void)
{
    if (objects.resultado_geocoding == NULL) {
        ESP_LOGW(TAG, "resultado_geocoding list object is NULL");
        return;
    }
    
    // Limpiar todos los elementos de la lista
    lv_obj_clean(objects.resultado_geocoding);
}

static void add_result_to_list(int index, const char *city_name, const char *country_code, float lat, float lon)
{
    if (objects.resultado_geocoding == NULL) {
        ESP_LOGW(TAG, "resultado_geocoding list object is NULL");
        return;
    }
    
    // Encontrar el nombre completo del país basado en el código
    const char *country_name = "País desconocido";
    for (int i = 0; i < sizeof(paises) / sizeof(paises[0]); i++) {
        if (strcmp(paises[i].code, country_code) == 0) {
            country_name = paises[i].name;
            break;
        }
    }
    
    char item_text[128];
    snprintf(item_text, sizeof(item_text), "%s, %s", city_name, country_name);
    
    // Agregar item a la lista sin ícono
    lv_obj_t *btn = lv_list_add_button(objects.resultado_geocoding, NULL, item_text);
    
    // Guardar el índice en el user_data del botón para poder identificarlo después
    lv_obj_set_user_data(btn, (void*)(intptr_t)index);
    
    // Agregar event handler para cuando se haga clic en el item
    lv_obj_add_event_cb(btn, geocoding_result_item_clicked, LV_EVENT_CLICKED, NULL);
}

static void show_message_in_list(const char *message)
{
    if (objects.resultado_geocoding == NULL) {
        ESP_LOGW(TAG, "resultado_geocoding list object is NULL");
        return;
    }
    
    // Limpiar lista
    clear_geocoding_results_list();
    
    // Agregar mensaje como un item no clickeable sin ícono
    lv_obj_t *btn = lv_list_add_button(objects.resultado_geocoding, NULL, message);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE); // Hacer que no sea clickeable
}

// Event handler para cuando se hace clic en un resultado
static void geocoding_result_item_clicked(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    int result_index = (int)(intptr_t)lv_obj_get_user_data(btn);
    
    ESP_LOGI(TAG, "User clicked on result index: %d", result_index);
    ui_geocoding_bridge_select_result(result_index);
}

// ============================================================================
// Helper functions
// ============================================================================

const char *get_codigo_pais_by_name(const char *name)
{
    if (!name) return NULL;
    size_t n = sizeof(paises) / sizeof(paises[0]);
    for (size_t i = 0; i < n; ++i) {
        if (strcmp(paises[i].name, name) == 0) return paises[i].code;
    }
    return NULL;
}

const char *get_codigo_pais_by_index(int index)
{
    if (index < 0) return NULL;
    size_t n = sizeof(paises) / sizeof(paises[0]);
    if ((size_t)index >= n) return NULL;
    return paises[index].code;
}

const char *get_nombre_pais_by_index(int index)
{
    if (index < 0) return NULL;
    size_t n = sizeof(paises) / sizeof(paises[0]);
    if ((size_t)index >= n) return NULL;
    return paises[index].name;
}

// ============================================================================
// Initialization
// ============================================================================

void ui_geocoding_bridge_init(void)
{
    ESP_LOGI(TAG, "UI Geocoding Bridge initialized");
    
    // Initialize buffers
    geocoding_results_buf[0] = '\0';
    
    // Initialize state
    geocoding_state.num_results = 0;
    geocoding_state.selected_result_index = -1;
    geocoding_state.search_in_progress = false;
    
    // Set initial message
    ui_geocoding_bridge_clear_results();
}

// ============================================================================
// Geocoding Workflow Implementation
// ============================================================================

// Step 3: Search execution
void ui_geocoding_bridge_search_location(const char* country_code, const char* city)
{
    ESP_LOGI(TAG, "Step 3: Searching for '%s' in country '%s'", city ? city : "NULL", country_code ? country_code : "NULL");
    
    if (!country_code || !city || strlen(city) == 0) {
        ESP_LOGW(TAG, "Invalid search parameters");
        ui_geocoding_bridge_show_error("Parámetros de búsqueda inválidos");
        return;
    }
    
    geocoding_state.search_in_progress = true;
    ui_geocoding_bridge_show_searching();
    
    // Call the real geocoding service
    esp_err_t ret = app_weather_geocoding_search_cities(country_code, city, 
                                                      geocoding_state.search_results, 
                                                      5, // max_results
                                                      &geocoding_state.num_results);
    
    geocoding_state.search_in_progress = false;
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Geocoding search completed with %d results", geocoding_state.num_results);
        ui_geocoding_bridge_show_results();
    } else {
        ESP_LOGW(TAG, "Geocoding search failed: %s", esp_err_to_name(ret));
        ui_geocoding_bridge_show_error("Error en la búsqueda. Verifique la conexión a internet.");
    }
}

// Step 4: Display results
void ui_geocoding_bridge_show_results(void)
{
    ESP_LOGI(TAG, "Step 4: Showing %d search results", geocoding_state.num_results);
    
    // Limpiar la lista
    clear_geocoding_results_list();
    
    if (geocoding_state.num_results == 0) {
        show_message_in_list("No se encontraron resultados.\nIntente con otra ciudad.");
    } else {
        // Agregar cada resultado como un item de la lista
        for (int i = 0; i < geocoding_state.num_results; i++) {
            ESP_LOGI(TAG, "Adding result %d: %s", i, geocoding_state.search_results[i].city_name);
            add_result_to_list(i, 
                              geocoding_state.search_results[i].city_name,
                              geocoding_state.search_results[i].country_code,
                              geocoding_state.search_results[i].latitude,
                              geocoding_state.search_results[i].longitude);
        }
    }
}

// Step 5: Result selection and persistence
void ui_geocoding_bridge_select_result(int result_index)
{
    ESP_LOGI(TAG, "Step 5: Selecting result index: %d", result_index);
    
    if (result_index < 0 || result_index >= geocoding_state.num_results) {
        ESP_LOGW(TAG, "Invalid result index: %d", result_index);
        ui_geocoding_bridge_show_error("Índice de resultado inválido");
        return;
    }
    
    geocoding_state.selected_result_index = result_index;
    
    // Configurar la ubicación customizada con el resultado seleccionado
    esp_err_t ret = app_weather_set_custom_location(&geocoding_state.search_results[result_index]);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set custom location: %s", esp_err_to_name(ret));
        ui_geocoding_bridge_show_error("Error al configurar la ubicación");
        return;
    }
    
    // Solicitar clima para la ubicación customizada
    ESP_LOGI(TAG, "Requesting weather for custom location: %s", 
             geocoding_state.search_results[result_index].city_name);
             
    ret = app_weather_request(LOCATION_NUM_CUSTOM);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Weather request sent for custom location");
    } else {
        ESP_LOGW(TAG, "Failed to request weather for custom location: %s", esp_err_to_name(ret));
    }
    
    ESP_LOGI(TAG, "Weather request sent for location: %s (%.4f, %.4f)", 
             geocoding_state.search_results[result_index].city_name, 
             geocoding_state.search_results[result_index].latitude,
             geocoding_state.search_results[result_index].longitude);
}

// Helper functions
void ui_geocoding_bridge_clear_results(void)
{
    show_message_in_list("Flujo de búsqueda:\n\n1. Seleccione un país\n2. Ingrese una ciudad\n3. Presione el botón Buscar\n\nComience seleccionando un país...");
}

void ui_geocoding_bridge_show_searching(void)
{
    show_message_in_list("Buscando ubicaciones...\n\nPor favor espere...");
}

void ui_geocoding_bridge_show_error(const char* error_message)
{
    char error_text[256];
    snprintf(error_text, sizeof(error_text), "Error: %s\n\nIntente nuevamente.", error_message ? error_message : "Error desconocido");
    show_message_in_list(error_text);
}

// ============================================================================
// EEZ Studio action implementations
// ============================================================================

void action_wh_find_geocoding(lv_event_t * e)
{
    ESP_LOGI(TAG, "EEZ Action: Search button pressed");
    
    // Get current country and city
    if (objects.pais == NULL || objects.ciudad == NULL) {
        ESP_LOGW(TAG, "UI objects not available");
        ui_geocoding_bridge_show_error("Error en la interfaz de usuario");
        return;
    }
    
    uint16_t country_index = lv_dropdown_get_selected(objects.pais);
    const char* country_code = get_codigo_pais_by_index((int)country_index);
    const char* city_text = lv_textarea_get_text(objects.ciudad);
    
    if (!country_code) {
        ui_geocoding_bridge_show_error("Seleccione un país válido");
        return;
    }
    
    if (!city_text || strlen(city_text) == 0) {
        ui_geocoding_bridge_show_error("Ingrese el nombre de una ciudad");
        return;
    }
    
    ui_geocoding_bridge_search_location(country_code, city_text);
}
