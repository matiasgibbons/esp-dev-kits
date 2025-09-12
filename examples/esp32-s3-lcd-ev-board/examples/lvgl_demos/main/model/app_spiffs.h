/*
 * SPIFFS file system management for user data
 */

#pragma once

#include "esp_err.h"
#include "esp_spiffs.h"
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// SPIFFS mount point
#define SPIFFS_BASE_PATH "/spiffs"

// Directory paths
#define LOGS_DIR        SPIFFS_BASE_PATH "/logs"
#define CONFIG_DIR      SPIFFS_BASE_PATH "/config"
#define BACKUPS_DIR     SPIFFS_BASE_PATH "/backups"

// File paths
#define WEATHER_LOCATIONS_FILE  CONFIG_DIR "/weather_locations.json"
#define USER_PREFERENCES_FILE   CONFIG_DIR "/user_preferences.json"
#define SETTINGS_BACKUP_FILE    BACKUPS_DIR "/settings_backup.json"

// Activity log file pattern: /logs/activity_YYYY-MM.json
#define ACTIVITY_LOG_PATTERN    LOGS_DIR "/activity_%04d-%02d.json"

// Activity log entry structure
typedef struct {
    time_t timestamp;
    char session_type[32];      // "Facial", "Corporal"
    char emission_mode[32];     // "Continuo", "Intermitente"
    uint16_t duration_minutes;
    uint8_t power_level;        // 1-100
    char notes[128];
} activity_log_entry_t;

// Weather location structure for JSON storage
typedef struct {
    char name[64];
    char country[32];
    float latitude;
    float longitude;
    bool is_favorite;
    time_t last_used;
} weather_location_entry_t;

// User preferences structure
typedef struct {
    char preferred_language[8];
    uint8_t default_brightness;
    uint16_t default_session_duration;
    uint8_t default_power_level;
    char default_session_type[32];
    char default_emission_mode[32];
    bool auto_weather_update;
    uint32_t weather_update_interval_minutes;
} user_preferences_t;

/**
 * @brief Initialize SPIFFS file system
 * @return esp_err_t
 */
esp_err_t app_spiffs_init(void);

/**
 * @brief Create directory structure
 * @return esp_err_t
 */
esp_err_t app_spiffs_create_directories(void);

/**
 * @brief Log a user activity session
 * @param entry Activity log entry
 * @return esp_err_t
 */
esp_err_t app_spiffs_log_activity(const activity_log_entry_t *entry);

/**
 * @brief Get activity logs for a specific month
 * @param year Year (e.g., 2025)
 * @param month Month (1-12)
 * @param entries Array to store entries
 * @param max_entries Maximum number of entries to read
 * @param count_read Number of entries actually read
 * @return esp_err_t
 */
esp_err_t app_spiffs_get_activity_logs(int year, int month, 
                                     activity_log_entry_t *entries, 
                                     size_t max_entries, 
                                     size_t *count_read);

/**
 * @brief Save weather locations list
 * @param locations Array of weather locations
 * @param count Number of locations
 * @return esp_err_t
 */
esp_err_t app_spiffs_save_weather_locations(const weather_location_entry_t *locations, size_t count);

/**
 * @brief Load weather locations list
 * @param locations Array to store locations
 * @param max_locations Maximum number of locations to read
 * @param count_read Number of locations actually read
 * @return esp_err_t
 */
esp_err_t app_spiffs_load_weather_locations(weather_location_entry_t *locations, 
                                           size_t max_locations, 
                                           size_t *count_read);

/**
 * @brief Save user preferences
 * @param prefs User preferences structure
 * @return esp_err_t
 */
esp_err_t app_spiffs_save_user_preferences(const user_preferences_t *prefs);

/**
 * @brief Load user preferences
 * @param prefs User preferences structure to fill
 * @return esp_err_t
 */
esp_err_t app_spiffs_load_user_preferences(user_preferences_t *prefs);

/**
 * @brief Create backup of current settings
 * @return esp_err_t
 */
esp_err_t app_spiffs_backup_settings(void);

/**
 * @brief Restore settings from backup
 * @return esp_err_t
 */
esp_err_t app_spiffs_restore_settings(void);

/**
 * @brief Get SPIFFS usage statistics
 * @param total_bytes Total SPIFFS size
 * @param used_bytes Used SPIFFS size
 * @return esp_err_t
 */
esp_err_t app_spiffs_get_usage(size_t *total_bytes, size_t *used_bytes);

/**
 * @brief Clean old activity logs (keep only last 12 months)
 * @return esp_err_t
 */
esp_err_t app_spiffs_cleanup_old_logs(void);

#ifdef __cplusplus
}
#endif