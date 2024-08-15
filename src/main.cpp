#include <stdlib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Inkplate.h>

#include "error.hpp"
#include "storage.hpp"
#include "roombelt_api.hpp"
#include "display.hpp"

#define uS_TO_S_FACTOR 1000000

void showWelcomeView(Display &display)
{
    display.showMessageScreen("Welcome");
    sleep(2);
    display.showMessageScreen("Connecting to WiFi...", true);
    RoombeltApi api;
    sleep(2);
    display.showMessageScreen("Connecting to Roombelt...", true);
    sleep(2);
}

void showDeviceView(Display &display)
{
    RoombeltApi api;
    DeviceStateHash hash;

    auto device = api.getDeviceState();

    if (device.getState() == StateInfo::DEVICE_REMOVED || device.getState() == StateInfo::MISSING_SESSION_ID)
    {
        api.removeDevice();
        api.registerNewDevice();
        device = api.getDeviceState();
    }

    // Exit early if there's no need to update screen
    if (hash.isEqualStoredHash(device))
        return;

    hash.storeHash(device);

    switch (device.getState())
    {
    case StateInfo::CONNECTION_CODE:
        return display.showConnectionCodeScreen(device);
    case StateInfo::SUCCESS:
        return display.showDeviceScreen(device);
    case StateInfo::ROOMBELT_ERROR:
        return display.showMessageScreen("Error while loading data", device.getError());
    default:
        return display.showMessageScreen("Unknown device state: " + String(device.getState()));
    }
}

void setup()
{
    Serial.begin(115200);
    Display display;

    try
    {
        if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER)
            showWelcomeView(display);

        showDeviceView(display);
    }
    catch (Error error)
    {
        display.showErrorScreen(error);
    }

    esp_sleep_enable_timer_wakeup(5 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void loop()
{
}