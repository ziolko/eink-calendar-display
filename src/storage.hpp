#include <Arduino.h>
#include <nvs.h>

class Storage
{
public:
    Storage();
    ~Storage();

    String getString(const char *key);
    String getString(const char *key, const char *defaultValue);

    void setString(const char *key, const char *value);

private:
    nvs_handle_t handle;
    bool hasChanges;
    void checkError(esp_err_t err, const char *message);
};