#include <ArduinoJson.h>

#include "device_state.hpp"

class RoombeltApi
{
public:
    ~RoombeltApi();

    void connect(String ssid, String password);
    void disconnect();

    void removeDevice();
    void registerNewDevice();
    DeviceState getDeviceState(int retryCount = 3);

private:
    String getSessionToken();
    std::tuple<String, String> getWiFiCredentials();
    void assertWiFi();
};