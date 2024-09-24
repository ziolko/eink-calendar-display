#include <stdlib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Inkplate.h>

#include "error.hpp"
#include "storage.hpp"
#include "roombelt_api.hpp"
#include "display.hpp"

DeviceState getDeviceState(RoombeltApi api, double battery)
{
    auto device = api.getDeviceState(3, battery);

    if (device.getState() == StateInfo::DEVICE_REMOVED || device.getState() == StateInfo::MISSING_SESSION_ID)
    {
        api.removeDevice();
        api.registerNewDevice();
        device = api.getDeviceState(3, battery);
    }

    return device;
}

void showDeviceView(Display &display, DeviceState &device, bool forceShow)
{
    DeviceStateHash hash;

    // Update screen only if data changed
    if (hash.isEqualStoredHash(device) && !forceShow)
    {
        return;
    }

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
    RoombeltApi api;

    int sleepTimeMs = 0;
    auto battery = display.getBattery();

    try
    {
        if (battery < 2.9)
        {
            display.deepSleep(1000 * 60 * 10);
            return;
        }

        if (battery < 2.95)
        {
            display.showImage(ERROR_IMAGE, 200, 100, 200, 200);
            display.showMessageScreen("Naladuj baterie!", "Stan baterii: " + String(battery) + "V");
            display.deepSleep(1000 * 60 * 10);
            return;
        }

        auto wakeupCause = esp_sleep_get_wakeup_cause();
        auto isForceRefresh = wakeupCause == ESP_SLEEP_WAKEUP_EXT0; // User pressed the "wake up" button
        auto isScheduledRefresh = wakeupCause == ESP_SLEEP_WAKEUP_TIMER;
        if (isForceRefresh)
        {
            display.showMessageScreen("Dobrze Cie widziec!", "Stan baterii: " + String(battery) + "V");
        }
        else if (!isScheduledRefresh)
        {
            display.showMessageScreen("Dobrze Cie widziec!", "Stan baterii: " + String(battery) + "V");
        }

        auto config = display.getConfig();
        api.connect(config.ssid, config.password);
        auto device = getDeviceState(api, battery);
        sleepTimeMs = device.getMsToNextRefresh();

        showDeviceView(display, device, isForceRefresh);
    }
    catch (ErrorWifiConnection error)
    {
        display.showImage(ERROR_IMAGE, 200, 100, 200, 200);
        display.showMessageScreen("Network error", error.getStatus());
    }
    catch (Error error)
    {
        display.showImage(ERROR_IMAGE, 200, 100, 200, 200);
        display.showErrorScreen(error);
    }

    if (sleepTimeMs <= 0)
    {
        sleepTimeMs = 1000 * 60; // One minute
    }

    display.deepSleep(sleepTimeMs);
}

void loop()
{
}