#include <ArduinoJson.h>

#include "device_state.hpp"

class RoombeltApi
{
public:
    RoombeltApi();
    ~RoombeltApi();

    void removeDevice();
    void registerNewDevice();
    DeviceState getDeviceState();

private:
    String getSessionToken();
    std::tuple<String, String> getWiFiCredentials();
    void assertWiFi();
};