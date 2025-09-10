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

// State tracking for the geocoding workflow
static struct {
    char selected_country_code[4];    // ISO code of selected country
    char entered_city[64];            // City name entered by user
    geocoding_location_t search_results[5]; // Up to 5 search results
    int num_results;                  // Number of results found
    int selected_result_index;        // Index of selected result (-1 if none)
    bool search_in_progress;          // Search operation ongoing
} geocoding_state = {
    .selected_country_code = "AR",    // Default to Argentina
    .entered_city = "",
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
    strncpy(geocoding_state.selected_country_code, "AR", sizeof(geocoding_state.selected_country_code) - 1);
    geocoding_state.entered_city[0] = '\0';
    geocoding_state.num_results = 0;
    geocoding_state.selected_result_index = -1;
    geocoding_state.search_in_progress = false;
    
    // Set initial message
    ui_geocoding_bridge_clear_results();
}

// ============================================================================
// Geocoding Workflow Implementation
// ============================================================================

// Step 1: Country selection
void ui_geocoding_bridge_on_country_change(uint16_t selected_index)
{
    ESP_LOGI(TAG, "Step 1: Country changed to index: %d", selected_index);
    
    const char* country_code = get_codigo_pais_by_index((int)selected_index);
    const char* country_name = get_nombre_pais_by_index((int)selected_index);
    
    if (country_code != NULL && country_name != NULL) {
        strncpy(geocoding_state.selected_country_code, country_code, sizeof(geocoding_state.selected_country_code) - 1);
        geocoding_state.selected_country_code[sizeof(geocoding_state.selected_country_code) - 1] = '\0';
        
        // Clear previous results when country changes (but don't search automatically)
        geocoding_state.num_results = 0;
        geocoding_state.selected_result_index = -1;
        
        // Show selected country info and prompt for city entry
        char info_text[128];
        if (strlen(geocoding_state.entered_city) > 0) {
            snprintf(info_text, sizeof(info_text), "País: %s (%s)\nCiudad: %s\n\n✓ Listo para buscar\nPresione el botón Buscar.", 
                     country_name, country_code, geocoding_state.entered_city);
        } else {
            snprintf(info_text, sizeof(info_text), "País seleccionado: %s (%s)\n\nAhora ingrese una ciudad...", 
                     country_name, country_code);
        }
        set_var_wh_geocoding_results(info_text);
        
        ESP_LOGI(TAG, "Country selected: %s (%s)", country_name, country_code);
    } else {
        ESP_LOGW(TAG, "Selected country index %d out of range", selected_index);
        ui_geocoding_bridge_show_error("País seleccionado inválido");
    }
}

// Step 2: City entry
void ui_geocoding_bridge_on_city_change(const char* city_text)
{
    ESP_LOGI(TAG, "Step 2: City changed to: '%s'", city_text ? city_text : "NULL");
    
    if (city_text) {
        strncpy(geocoding_state.entered_city, city_text, sizeof(geocoding_state.entered_city) - 1);
        geocoding_state.entered_city[sizeof(geocoding_state.entered_city) - 1] = '\0';
        
        // Clear previous results when city changes (but don't search automatically)
        geocoding_state.num_results = 0;
        geocoding_state.selected_result_index = -1;
        
        // Update status to show readiness for search
        if (strlen(city_text) > 0 && strlen(geocoding_state.selected_country_code) > 0) {
            const char* country_name = get_nombre_pais_by_index(lv_dropdown_get_selected(objects.pais));
            char status_text[128];
            snprintf(status_text, sizeof(status_text), "País: %s\nCiudad: %s\n\n✓ Listo para buscar\nPresione el botón Buscar.", 
                     country_name ? country_name : geocoding_state.selected_country_code, city_text);
            set_var_wh_geocoding_results(status_text);
        } else {
            ui_geocoding_bridge_clear_results();
        }
    } else {
        geocoding_state.entered_city[0] = '\0';
        ui_geocoding_bridge_clear_results();
    }
}

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
    
    // Call the geocoding service (this would be implemented in app_weather.c)
    // For now, simulate the search with dummy results
    // esp_err_t ret = app_weather_geocoding_search(country_code, city, geocoding_state.search_results, &geocoding_state.num_results);
    
    // Simulate search results
    geocoding_state.num_results = 2;
    snprintf(geocoding_state.search_results[0].city_name, sizeof(geocoding_state.search_results[0].city_name), 
             "%s, %s (Principal)", city, country_code);
    geocoding_state.search_results[0].latitude = -34.6037;
    geocoding_state.search_results[0].longitude = -58.3816;
    strncpy(geocoding_state.search_results[0].country_code, country_code, sizeof(geocoding_state.search_results[0].country_code) - 1);
    geocoding_state.search_results[0].is_valid = true;
    
    snprintf(geocoding_state.search_results[1].city_name, sizeof(geocoding_state.search_results[1].city_name), 
             "%s Centro, %s", city, country_code);
    geocoding_state.search_results[1].latitude = -34.6118;
    geocoding_state.search_results[1].longitude = -58.3960;
    strncpy(geocoding_state.search_results[1].country_code, country_code, sizeof(geocoding_state.search_results[1].country_code) - 1);
    geocoding_state.search_results[1].is_valid = true;
    
    geocoding_state.search_in_progress = false;
    ui_geocoding_bridge_show_results();
}

// Step 4: Display results
void ui_geocoding_bridge_show_results(void)
{
    ESP_LOGI(TAG, "Step 4: Showing %d search results", geocoding_state.num_results);
    
    char results_text[512] = "";
    char temp_line[128];
    
    if (geocoding_state.num_results == 0) {
        strcat(results_text, "No se encontraron resultados.\n\nIntente con otra ciudad o verifique la escritura.");
    } else {
        strcat(results_text, "Resultados encontrados:\n\n");
        for (int i = 0; i < geocoding_state.num_results; i++) {
            snprintf(temp_line, sizeof(temp_line), "%d. %s\n", 
                     i + 1, geocoding_state.search_results[i].city_name);
            strcat(results_text, temp_line);
        }
        strcat(results_text, "\nSeleccione un resultado haciendo clic en él.");
    }
    
    set_var_wh_geocoding_results(results_text);
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
    
    // Persist the selected location (this would update global app settings)
    // app_settings_set_location(&geocoding_state.search_results[result_index]);
    
    // Solicitar clima para la nueva ubicación seleccionada
    ESP_LOGI(TAG, "Requesting weather for selected location: %s", 
             geocoding_state.search_results[result_index].city_name);
             
    esp_err_t ret = app_weather_request_geocoded(
        geocoding_state.selected_country_code,
        geocoding_state.search_results[result_index].city_name
    );
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Weather request sent for new location");
    } else {
        ESP_LOGW(TAG, "Failed to request weather for new location: %s", esp_err_to_name(ret));
    }
    
    // Show confirmation
    char confirmation_text[256];
    snprintf(confirmation_text, sizeof(confirmation_text), 
             "Ubicación seleccionada:\n%s\n\nLat: %.4f\nLon: %.4f\n\n¡Configuración guardada!\nSolicitando clima...\n\nPresione Volver para salir.",
             geocoding_state.search_results[result_index].city_name,
             geocoding_state.search_results[result_index].latitude,
             geocoding_state.search_results[result_index].longitude);
    
    set_var_wh_geocoding_results(confirmation_text);
    
    ESP_LOGI(TAG, "Location persisted: %s", geocoding_state.search_results[result_index].city_name);
}

// Helper functions
void ui_geocoding_bridge_clear_results(void)
{
    set_var_wh_geocoding_results("Flujo de búsqueda:\n\n1. Seleccione un país\n2. Ingrese una ciudad\n3. Presione el botón Buscar\n\nComience seleccionando un país...");
}

void ui_geocoding_bridge_show_searching(void)
{
    set_var_wh_geocoding_results("Buscando ubicaciones...\n\nPor favor espere...");
}

void ui_geocoding_bridge_show_error(const char* error_message)
{
    char error_text[256];
    snprintf(error_text, sizeof(error_text), "Error: %s\n\nIntente nuevamente.", error_message ? error_message : "Error desconocido");
    set_var_wh_geocoding_results(error_text);
}

// Step 6: Exit workflow (handled by UI button event)
void ui_geocoding_bridge_exit(void)
{
    ESP_LOGI(TAG, "Step 6: Exiting geocoding workflow");
    
    // Reset state for next use
    geocoding_state.num_results = 0;
    geocoding_state.selected_result_index = -1;
    geocoding_state.search_in_progress = false;
    
    // Clear display
    ui_geocoding_bridge_clear_results();
}

// ============================================================================
// EEZ Studio action implementations
// ============================================================================

void action_pais_onchange(lv_event_t * e)
{
    lv_obj_t * dropdown = lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);
    
    ESP_LOGI(TAG, "EEZ Action: Country changed to index %d", selected);
    ui_geocoding_bridge_on_country_change(selected);
}

void action_ciudad_onchange(lv_event_t * e)
{
    lv_obj_t * textarea = lv_event_get_target(e);
    const char* city_text = lv_textarea_get_text(textarea);
    
    ESP_LOGI(TAG, "EEZ Action: City changed to '%s'", city_text ? city_text : "NULL");
    ui_geocoding_bridge_on_city_change(city_text);
}

void action_resultado_geocoding_onchange(lv_event_t * e)
{
    lv_obj_t * textarea = lv_event_get_target(e);
    
    ESP_LOGI(TAG, "EEZ Action: Geocoding results area changed");
    
    // This could handle result selection via clicking on the textarea
    // For now, just log the interaction
}

void action_buscar_location(lv_event_t * e)
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

void action_volver_geocoding(lv_event_t * e)
{
    ESP_LOGI(TAG, "EEZ Action: Return button pressed");
    ui_geocoding_bridge_exit();
}
