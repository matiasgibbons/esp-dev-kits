/*
 * SPIFFS file system management for user data
 */

#include "app_spiffs.h"
#include "esp_log.h"
#include "cJSON.h"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

static const char *TAG = "app_spiffs";

esp_err_t app_spiffs_init(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = SPIFFS_BASE_PATH,
        .partition_label = "user_data",
        .max_files = 10,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("user_data", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SPIFFS: %d KB total, %d KB used", total / 1024, used / 1024);

    // Create directory structure
    return app_spiffs_create_directories();
}

esp_err_t app_spiffs_create_directories(void)
{
    /* NOTE: SPIFFS in ESP-IDF emulates a flat namespace; directory support is
     * limited (there is no real hierarchy on-flash). On some IDF versions
     * mkdir() will return ENOSYS. We still attempt to call mkdir() so the code
     * can seamlessly switch to LittleFS later (which has real directories).
     * We now log errno for easier diagnosis. */

    struct dir_spec {
        const char *path;
        const char *label;
    } dirs[] = {
        { LOGS_DIR,    "logs" },
        { CONFIG_DIR,  "config" },
        { BACKUPS_DIR, "backups" }
    };

    for (size_t i = 0; i < sizeof(dirs)/sizeof(dirs[0]); ++i) {
        if (mkdir(dirs[i].path, 0755) != 0) {
            int e = errno; // capture before any other call
            if (e == EEXIST) {
                ESP_LOGD(TAG, "Directory already exists: %s", dirs[i].path);
            } else if (e == ENOSYS || e == ENOTSUP) {
                // Downgrade to INFO because this is normal for SPIFFS (flat namespace)
                ESP_LOGI(TAG, "Directory semantics not supported on SPIFFS (path=%s); continuing in flat mode.", dirs[i].path);
            } else {
                ESP_LOGW(TAG, "Failed to create %s directory (path=%s, errno=%d: %s)",
                         dirs[i].label, dirs[i].path, e, strerror(e));
            }
        } else {
            ESP_LOGI(TAG, "Created directory: %s", dirs[i].path);
        }
    }

    ESP_LOGI(TAG, "Directory structure processed");
    return ESP_OK;
}

esp_err_t app_spiffs_log_activity(const activity_log_entry_t *entry)
{
    if (!entry) return ESP_ERR_INVALID_ARG;

    // Get current time for file naming
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    char filename[128];
    snprintf(filename, sizeof(filename), ACTIVITY_LOG_PATTERN, 
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1);

    // Create JSON object for the new entry
    cJSON *json_entry = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_entry, "timestamp", (double)entry->timestamp);
    cJSON_AddStringToObject(json_entry, "session_type", entry->session_type);
    cJSON_AddStringToObject(json_entry, "emission_mode", entry->emission_mode);
    cJSON_AddNumberToObject(json_entry, "duration_minutes", entry->duration_minutes);
    cJSON_AddNumberToObject(json_entry, "power_level", entry->power_level);
    cJSON_AddStringToObject(json_entry, "notes", entry->notes);

    // Load existing log file or create new array
    cJSON *json_array = NULL;
    FILE *file = fopen(filename, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char *file_content = malloc(file_size + 1);
        fread(file_content, 1, file_size, file);
        file_content[file_size] = '\0';
        fclose(file);

        json_array = cJSON_Parse(file_content);
        free(file_content);
    }

    if (!json_array) {
        json_array = cJSON_CreateArray();
    }

    // Add new entry to array
    cJSON_AddItemToArray(json_array, json_entry);

    // Save back to file
    file = fopen(filename, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open activity log file for writing");
        cJSON_Delete(json_array);
        return ESP_FAIL;
    }

    char *json_string = cJSON_Print(json_array);
    fprintf(file, "%s", json_string);
    fclose(file);

    free(json_string);
    cJSON_Delete(json_array);

    ESP_LOGI(TAG, "Activity logged: %s for %d minutes at %d%% power", 
             entry->session_type, entry->duration_minutes, entry->power_level);

    return ESP_OK;
}

esp_err_t app_spiffs_save_weather_locations(const weather_location_entry_t *locations, size_t count)
{
    if (!locations) return ESP_ERR_INVALID_ARG;

    cJSON *json_array = cJSON_CreateArray();
    
    for (size_t i = 0; i < count; i++) {
        cJSON *location = cJSON_CreateObject();
        cJSON_AddStringToObject(location, "name", locations[i].name);
        cJSON_AddStringToObject(location, "country", locations[i].country);
        cJSON_AddNumberToObject(location, "latitude", locations[i].latitude);
        cJSON_AddNumberToObject(location, "longitude", locations[i].longitude);
        cJSON_AddBoolToObject(location, "is_favorite", locations[i].is_favorite);
        cJSON_AddNumberToObject(location, "last_used", (double)locations[i].last_used);
        
        cJSON_AddItemToArray(json_array, location);
    }

    FILE *file = fopen(WEATHER_LOCATIONS_FILE, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open weather locations file for writing");
        cJSON_Delete(json_array);
        return ESP_FAIL;
    }

    char *json_string = cJSON_Print(json_array);
    fprintf(file, "%s", json_string);
    fclose(file);

    free(json_string);
    cJSON_Delete(json_array);

    ESP_LOGI(TAG, "Saved %d weather locations", count);
    return ESP_OK;
}

esp_err_t app_spiffs_save_user_preferences(const user_preferences_t *prefs)
{
    if (!prefs) return ESP_ERR_INVALID_ARG;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "preferred_language", prefs->preferred_language);
    cJSON_AddNumberToObject(json, "default_brightness", prefs->default_brightness);
    cJSON_AddNumberToObject(json, "default_session_duration", prefs->default_session_duration);
    cJSON_AddNumberToObject(json, "default_power_level", prefs->default_power_level);
    cJSON_AddStringToObject(json, "default_session_type", prefs->default_session_type);
    cJSON_AddStringToObject(json, "default_emission_mode", prefs->default_emission_mode);
    cJSON_AddBoolToObject(json, "auto_weather_update", prefs->auto_weather_update);
    cJSON_AddNumberToObject(json, "weather_update_interval_minutes", prefs->weather_update_interval_minutes);

    FILE *file = fopen(USER_PREFERENCES_FILE, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open user preferences file for writing");
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    char *json_string = cJSON_Print(json);
    fprintf(file, "%s", json_string);
    fclose(file);

    free(json_string);
    cJSON_Delete(json);

    ESP_LOGI(TAG, "User preferences saved");
    return ESP_OK;
}

esp_err_t app_spiffs_get_usage(size_t *total_bytes, size_t *used_bytes)
{
    return esp_spiffs_info("user_data", total_bytes, used_bytes);
}

esp_err_t app_spiffs_cleanup_old_logs(void)
{
    DIR *dir = opendir(LOGS_DIR);
    if (!dir) {
        ESP_LOGW(TAG, "Failed to open logs directory");
        return ESP_FAIL;
    }

    time_t now = time(NULL);
    time_t twelve_months_ago = now - (365 * 24 * 60 * 60); // Approximate

    struct dirent *entry;
    int deleted_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "activity_") == entry->d_name) {
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s", LOGS_DIR, entry->d_name);
            
            struct stat file_stat;
            if (stat(full_path, &file_stat) == 0) {
                if (file_stat.st_mtime < twelve_months_ago) {
                    if (unlink(full_path) == 0) {
                        deleted_count++;
                        ESP_LOGI(TAG, "Deleted old log file: %s", entry->d_name);
                    }
                }
            }
        }
    }

    closedir(dir);
    ESP_LOGI(TAG, "Cleanup completed: %d old log files deleted", deleted_count);
    return ESP_OK;
}