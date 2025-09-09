/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "settings.h"

static const char *TAG = "settings";
static const char *NVS_NAMESPACE = "wifi_config";

static sys_param_t sys_params = {
    .networks = {
        {"TEXEL24", "Texel52b", 0},     // Prioridad 0 (más alta)
        {"TEXEL50", "Texel52b", 1},     // Prioridad 1 (respaldo)
        {"", "", 2}                      // Red vacía para configuración manual
    },
    .network_count = 2,
    .current_network = 0,
    .ssid = "TEXEL24",              // Red WiFi principal
    .password = "Texel52b",         // Contraseña WiFi
    .ssid_len = 0,
    .password_len = 0,
    .need_hint = false,
    .demo_gui = true,
};

esp_err_t settings_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializar longitudes de strings
    sys_params.ssid_len = strlen(sys_params.ssid);
    sys_params.password_len = strlen(sys_params.password);

    // Intentar cargar configuración desde NVS
    nvs_handle_t nvs_handle;
    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret == ESP_OK) {
        size_t ssid_len = sizeof(sys_params.ssid);
        size_t pass_len = sizeof(sys_params.password);
        
        nvs_get_str(nvs_handle, "ssid", sys_params.ssid, &ssid_len);
        nvs_get_str(nvs_handle, "password", sys_params.password, &pass_len);
        
        sys_params.ssid_len = strlen(sys_params.ssid);
        sys_params.password_len = strlen(sys_params.password);
        
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "WiFi config loaded from NVS: SSID=%s", sys_params.ssid);
    } else {
        ESP_LOGI(TAG, "Using default WiFi config: SSID=%s", sys_params.ssid);
    }

    return ESP_OK;
}

esp_err_t settings_save_wifi_config(const char *ssid, const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // Guardar SSID
    ret = nvs_set_str(nvs_handle, "ssid", ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error saving SSID: %s", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    // Guardar contraseña
    ret = nvs_set_str(nvs_handle, "password", password);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error saving password: %s", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    // Confirmar cambios
    ret = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    if (ret == ESP_OK) {
        // Actualizar parámetros locales
        strncpy(sys_params.ssid, ssid, sizeof(sys_params.ssid) - 1);
        strncpy(sys_params.password, password, sizeof(sys_params.password) - 1);
        sys_params.ssid_len = strlen(sys_params.ssid);
        sys_params.password_len = strlen(sys_params.password);
        
        ESP_LOGI(TAG, "WiFi config saved: SSID=%s", ssid);
    }

    return ret;
}

esp_err_t settings_try_next_network(void)
{
    if (sys_params.current_network < sys_params.network_count - 1) {
        sys_params.current_network++;
    } else {
        sys_params.current_network = 0; // Volver al inicio
    }

    // Actualizar credenciales actuales
    strncpy(sys_params.ssid, sys_params.networks[sys_params.current_network].ssid, sizeof(sys_params.ssid) - 1);
    strncpy(sys_params.password, sys_params.networks[sys_params.current_network].password, sizeof(sys_params.password) - 1);
    sys_params.ssid_len = strlen(sys_params.ssid);
    sys_params.password_len = strlen(sys_params.password);

    ESP_LOGI(TAG, "Switching to network %d: %s", sys_params.current_network, sys_params.ssid);
    return ESP_OK;
}

esp_err_t settings_get_current_network(char *ssid, char *password)
{
    if (ssid) {
        strncpy(ssid, sys_params.ssid, MAX_SSID_LEN - 1);
        ssid[MAX_SSID_LEN - 1] = '\0';
    }
    if (password) {
        strncpy(password, sys_params.password, MAX_PASSWORD_LEN - 1);
        password[MAX_PASSWORD_LEN - 1] = '\0';
    }
    return ESP_OK;
}

sys_param_t *settings_get_parameter(void)
{
    return &sys_params;
}
