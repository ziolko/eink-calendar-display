#include <stdlib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Inkplate.h>

#include "error.hpp"
#include "storage.hpp"
#include "roombelt_api.hpp"
#include "display.hpp"

void showWelcomeView(Display &display, RoombeltApi &api)
{
    display.showMessageScreen("Welcome");
    sleep(2);
    display.showMessageScreen("Connecting to WiFi...");
    auto config = display.getConfig();
    api.connect(config.ssid, config.password);
    sleep(2);
    display.showMessageScreen("Connecting to Roombelt...");
    sleep(2);
}

DeviceState getDeviceState(RoombeltApi api)
{
    auto device = api.getDeviceState();

    if (device.getState() == StateInfo::DEVICE_REMOVED || device.getState() == StateInfo::MISSING_SESSION_ID)
    {
        api.removeDevice();
        api.registerNewDevice();
        device = api.getDeviceState();
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
    Display display;
    try
    {
        auto battery = display.readBattery();
        if (battery < 3.2)
        {
            display.showMessageScreen("Low battery", String(battery) + "V");
            sleep(1);
        }

        display.showRandomImage();
    }
    catch (Error error)
    {
        display.showErrorScreen(error);
    }

    display.deepSleep(10 * 60 * 1000);
}

void loop()
{
}