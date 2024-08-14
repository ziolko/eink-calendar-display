#include <Arduino.h>
#include <nvs_flash.h>
#include <nvs.h>

class NvsHandle
{
public:
    nvs_handle_t handle;
    esp_err_t err;

    NvsHandle()
    {
        handle = 0;
        err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
            err = nvs_flash_init();
        }
        err = nvs_open("storage", NVS_READWRITE, &handle);
    }

    ~NvsHandle()
    {
        if (handle != 0)
        {
            nvs_close(handle);
        }
    }
};

String load_string(const char *key, const char *defaultValue)
{
    NvsHandle storage_handle;
    if (storage_handle.err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(storage_handle.err));
        return defaultValue;
    }

    esp_err_t err;

    size_t length;
    err = nvs_get_str(storage_handle.handle, key, NULL, &length);
    switch (err)
    {
    case ESP_OK:
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        return defaultValue;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(err));
        return defaultValue;
    }

    char result[length];
    err = nvs_get_str(storage_handle.handle, key, result, &length);
    switch (err)
    {
    case ESP_OK:
        return result;
    case ESP_ERR_NVS_NOT_FOUND:
        return defaultValue;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(err));
        return defaultValue;
    }
}

void save_string(const char *key, const char *value)
{
    NvsHandle storage_handle;
    if (storage_handle.err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(storage_handle.err));
        return;
    }

    esp_err_t err;
    err = nvs_set_str(storage_handle.handle, key, value);

    ESP_ERROR_CHECK(err);

    err = nvs_commit(storage_handle.handle);

    ESP_ERROR_CHECK(err);
}
