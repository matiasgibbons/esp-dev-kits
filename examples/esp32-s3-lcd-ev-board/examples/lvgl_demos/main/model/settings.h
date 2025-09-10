/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64
#define MAX_WIFI_NETWORKS 3

typedef struct {
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASSWORD_LEN];
    uint8_t priority;  // 0 = más alta prioridad
} wifi_network_t;

typedef struct {
    wifi_network_t networks[MAX_WIFI_NETWORKS];
    uint8_t network_count;
    uint8_t current_network;
    char ssid[MAX_SSID_LEN];        // Red actualmente configurada
    char password[MAX_PASSWORD_LEN]; // Contraseña actualmente configurada
    uint8_t ssid_len;
    uint8_t password_len;
    bool need_hint;
    bool demo_gui;
} sys_param_t;

/**
 * @brief Get system parameters
 * @return Pointer to system parameters structure
 */
sys_param_t *settings_get_parameter(void);

/**
 * @brief Initialize settings system
 * @return ESP_OK on success
 */
esp_err_t settings_init(void);

/**
 * @brief Save WiFi credentials
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return ESP_OK on success
 */
esp_err_t settings_save_wifi_config(const char *ssid, const char *password);

/**
 * @brief Try next WiFi network (for fallback)
 * @return ESP_OK if there's another network to try
 */
esp_err_t settings_try_next_network(void);

/**
 * @brief Get current network info
 * @param ssid Buffer for SSID
 * @param password Buffer for password
 * @return ESP_OK on success
 */
esp_err_t settings_get_current_network(char *ssid, char *password);

#ifdef __cplusplus
}
#endif
