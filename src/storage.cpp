#include <Arduino.h>
#include <nvs_flash.h>
#include <nvs.h>
#include "storage.hpp"
#include "error.hpp"

Storage::Storage()
{
    hasChanges = false;
    handle = 0;
    auto err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        err = nvs_flash_init();
    }

    checkError(err, "Initializing flash");
    err = nvs_open("storage", NVS_READWRITE, &handle);
    checkError(err, "Opening storage");
}

Storage::~Storage()
{
    if (hasChanges)
    {
        auto err = nvs_commit(handle);
        checkError(err, "Commiting changes");
    }

    if (handle != 0)
    {
        nvs_close(handle);
        handle = 0;
    }
}

void Storage::checkError(esp_err_t err, const StringSumHelper &message)
{
    return checkError(err, message.c_str());
}

void Storage::checkError(esp_err_t err, const char *message)
{
    if (err != ESP_OK)
    {
        throw ErrorStorage(String(message) + " (ErrorName: " + esp_err_to_name(err) + ")");
    }
}

String Storage::getString(const char *key)
{
    return getString(key, "");
}

String Storage::getString(const char *key, const char *defaultValue)
{
    size_t length;
    auto err = nvs_get_str(handle, key, NULL, &length);

    if (err == ESP_ERR_NVS_NOT_FOUND)
        return defaultValue;

    checkError(err, String("Getting string size for ") + key);

    char result[length];
    err = nvs_get_str(handle, key, result, &length);
    checkError(err, String("Getting string value for ") + key);

    return result;
}

void Storage::setString(const char *key, const char *value)
{
    auto err = nvs_set_str(handle, key, value);
    checkError(err, "Setting string");
    hasChanges = true;
}
