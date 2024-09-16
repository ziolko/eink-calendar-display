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

    int sleepTimeMs = -1;
    auto battery = display.getBattery();

    try
    {
        auto wakeupCause = esp_sleep_get_wakeup_cause();
        auto isForceRefresh = wakeupCause == ESP_SLEEP_WAKEUP_EXT0; // User pressed the "wake up" button
        if (isForceRefresh)
        {
            display.showMessageScreen("Dobrze Cie widziec!", "Stan baterii: " + String(battery) + "V");
            // display.showRandomImage();
        }
        else if (wakeupCause != ESP_SLEEP_WAKEUP_TIMER)
        {
            display.showMessageScreen("Dobrze Cie widziec!", "Stan baterii: " + String(battery) + "V");
        }

        auto config = display.getConfig();
        api.connect(config.ssid, config.password);
        auto device = getDeviceState(api, battery);
        sleepTimeMs = device.getMsToNextRefresh();

        if (!device.isEnergySaving() || device.isOccupied())
        {
            showDeviceView(display, device, isForceRefresh);
        }
        else if (!isForceRefresh)
        {
            display.showRandomImage();
        }
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

    int ONE_MINUTE = 60 * 1000;
    int FIVE_MINUTES = 5 * ONE_MINUTE;

    if (sleepTimeMs <= 0)
        sleepTimeMs = ONE_MINUTE;
    else if (sleepTimeMs > FIVE_MINUTES)
        sleepTimeMs = FIVE_MINUTES;

    display.deepSleep(sleepTimeMs);
}

void loop()
{
}