/*
 * Geocoding and location search bridge for Pantalla ubicacion.
 * Implements complete workflow: Country -> City -> Search -> Results -> Select -> Persist -> Return
 */

#include "ui_geocoding_bridge.h"
#include "app_weather.h"
#include "event_system.h"
#include "esp_log.h"
#include <string.h>

// Incluir headers de EEZ Studio para acceder a objects
#include "screens.h"
#include "vars.h"
#include "ui.h"

static const char *TAG = "ui_geocoding_bridge";

// Forward declarations
static void geocoding_result_item_clicked(lv_event_t *e);
static void ui_geocoding_bridge_show_searching(void);
static void ui_geocoding_bridge_process_result_event(const geocoding_search_result_evt_t *res);

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

// ---------------------------------------------------------------------------
// Lista de países: única fuente de la verdad (nombres + código ISO)
// ---------------------------------------------------------------------------
// Se eliminó la duplicación: a partir de este array se construye el string
// de opciones para el dropdown (nombres separados por '\n').
// Solo necesitamos los códigos para la búsqueda. Los nombres visibles viven en listado_paises_options.
static const char * const paises_codes[] = {
    "AR", "BO", "BR", "CL", "CO",
    "CR", "CU", "EC", "SV", "GT",
    "HN", "MX", "NI", "PA", "PY",
    "PE", "PR", "DO", "UY", "VE"
};

// String constante con las opciones del dropdown (nombres separados por '\n').
// No se construye en runtime.
static const char listado_paises_options[] =
    "Argentina\n"
    "Bolivia\n"
    "Brasil\n"
    "Chile\n"
    "Colombia\n"
    "Costa Rica\n"
    "Cuba\n"
    "Ecuador\n"
    "El Salvador\n"
    "Guatemala\n"
    "Honduras\n"
    "México\n"
    "Nicaragua\n"
    "Panamá\n"
    "Paraguay\n"
    "Perú\n"
    "Puerto Rico\n"
    "República Dominicana\n"
    "Uruguay\n"
    "Venezuela"; // sin salto final

// Buffer de resultados geocoding
static char geocoding_results_buf[1024];

// ============================================================================
// Country list access (versión simplificada)
// ============================================================================

const char *get_var_listado_paises(void) {
    return listado_paises_options;
}

// Para mantener compatibilidad con el código generado por EEZ Studio.
// En este diseño estático no se requiere sobrescribir la lista.
// Si alguien intenta hacerlo, simplemente se ignora y se loggea (nivel debug).
void set_var_listado_paises(const char *value) {
    // Ignorado por diseño (lista fija). Se deja para compatibilidad.
    if (value) {
        ESP_LOGD(TAG, "set_var_listado_paises() ignorado: lista fija");
    }
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
    
    char item_text[128];
    // Mostramos solo código de país (nombres no necesarios según nuevo criterio)
    snprintf(item_text, sizeof(item_text), "%s, %s", city_name, country_code ? country_code : "??");
    
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

// Mostrar estado de búsqueda en progreso
static void ui_geocoding_bridge_show_searching(void)
{
    geocoding_state.num_results = 0; // reset parcial
    show_message_in_list("Buscando...\n\nPor favor espere.");
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

const char *get_codigo_pais_by_index(int index)
{
    if (index < 0) return NULL;
    size_t n = sizeof(paises_codes)/sizeof(paises_codes[0]);
    if ((size_t)index >= n) return NULL;
    return paises_codes[index];
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
    
}

// ============================================================================
// Geocoding Workflow Implementation
// ============================================================================

// Step 3: Search execution
void ui_geocoding_bridge_search_location(const char* country_code, const char* city)
{
    ESP_LOGI(TAG, "Geocoding: request '%s' in '%s'", city ? city : "NULL", country_code ? country_code : "NULL");

    if (geocoding_state.search_in_progress) {
        ESP_LOGW(TAG, "Search already in progress - ignoring new request");
        return;
    }

    if (!country_code || !city || strlen(city) == 0) {
        ui_geocoding_bridge_show_error("Parámetros de búsqueda inválidos");
        return;
    }

    geocoding_state.search_in_progress = true;
    geocoding_state.num_results = 0;
    geocoding_state.selected_result_index = -1;
    ui_geocoding_bridge_show_searching();

    geocoding_search_request_t req = {0};
    strncpy(req.country_code, country_code, 2); req.country_code[2] = '\0';
    strncpy(req.city, city, sizeof(req.city)-1);

    if (event_system_post_simple(EVENT_GEOCODING_SEARCH_REQUESTED, &req, sizeof(req)) != ESP_OK) {
        geocoding_state.search_in_progress = false;
        ui_geocoding_bridge_show_error("No se pudo enviar la solicitud");
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

// Procesar evento COMPLETE/FAILED desde UI task
void ui_geocoding_bridge_process_updates(system_event_t *evt)
{
    if (!evt) return;
    switch (evt->type) {
        case EVENT_GEOCODING_SEARCH_START:
            ESP_LOGI(TAG, "Geocoding search started");
            break;
        case EVENT_GEOCODING_SEARCH_COMPLETE:
            if (evt->data && evt->data_size == sizeof(geocoding_search_result_evt_t)) {
                const geocoding_search_result_evt_t *res = (const geocoding_search_result_evt_t*)evt->data;
                ui_geocoding_bridge_process_result_event(res);
            } else {
                ESP_LOGW(TAG, "Unexpected payload size for COMPLETE");
            }
            break;
        case EVENT_GEOCODING_SEARCH_FAILED:
            geocoding_state.search_in_progress = false;
            ui_geocoding_bridge_show_error("Búsqueda fallida");
            break;
        default:
            break;
    }
}

static void ui_geocoding_bridge_process_result_event(const geocoding_search_result_evt_t *res)
{
    geocoding_state.search_in_progress = false;
    geocoding_state.num_results = res->num_results;
    if (geocoding_state.num_results > 5) geocoding_state.num_results = 5;
    for (int i = 0; i < geocoding_state.num_results; ++i) {
        geocoding_state.search_results[i] = res->results[i];
    }
    ui_geocoding_bridge_show_results();
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
    
    // Configurar la ubicación de usuario con el resultado seleccionado
    esp_err_t ret = app_weather_set_custom_location(&geocoding_state.search_results[result_index]);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set user location: %s", esp_err_to_name(ret));
        ui_geocoding_bridge_show_error("Error al configurar la ubicación");
        return;
    }
    
    // Solicitar clima para la ubicación customizada
    ESP_LOGI(TAG, "Requesting weather for user location: %s", 
             geocoding_state.search_results[result_index].city_name);
             
    ret = app_weather_request(LOCATION_NUM_USER);
    
    if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Weather request sent for user location");
    } else {
    ESP_LOGW(TAG, "Failed to request weather for user location: %s", esp_err_to_name(ret));
    }
    
    ESP_LOGI(TAG, "Weather request sent for location: %s (%.4f, %.4f)", 
             geocoding_state.search_results[result_index].city_name, 
             geocoding_state.search_results[result_index].latitude,
             geocoding_state.search_results[result_index].longitude);
}

// Helper functions

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
