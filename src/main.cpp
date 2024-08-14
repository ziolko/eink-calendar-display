#include <stdlib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Inkplate.h>

#include "error.hpp"
#include "storage.hpp"
#include "roombelt_api.hpp"

Inkplate display(INKPLATE_1BIT);

void displayDevice(DeviceState &device);
void displayConnectionCode(DeviceState &device);

void setup()
{
    Serial.begin(115200);
    display.begin();
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(150, 320);
    display.setTextSize(4);
    display.print("Welcome to Roombelt!!");
    display.display();
}

void loop()
{
    try
    {
        RoombeltApi api;

        auto device = api.getDeviceState();
        auto state = device.getState();

        if (state == StateInfo::DEVICE_REMOVED || state == StateInfo::MISSING_SESSION_ID)
        {
            api.removeDevice();
            api.registerNewDevice();
            device = api.getDeviceState();
            state = device.getState();
        }

        if (state == StateInfo::SUCCESS)
        {
            displayDevice(device);
        }
        else if (state == StateInfo::CONNECTION_CODE)
        {
            displayConnectionCode(device);
        }
        else
        {
            display.clearDisplay();

            display.setCursor(10, 10);
            display.setTextSize(6);
            display.printf(state == StateInfo::ROOMBELT_ERROR ? device.getError().c_str() : " Network error");

            display.display();
        }
    }
    catch (Error error)
    {
        display.clearDisplay();

        display.setCursor(10, 10);
        display.setTextSize(6);

        auto message = "Error while loading data: " + String(error.message);
        Serial.println(message.c_str());
        display.println(message.c_str());

        display.display();
    }

    delay(30000);
}

void displayConnectionCode(DeviceState &device)
{
    display.clearDisplay();

    display.setCursor(10, 10);
    display.setTextSize(6);

    display.println("Code: " + device.getConnectionCode());

    display.display();
}

void displayDevice(DeviceState &device)
{
    MeetingData current = device.getCurrentMeeting();
    MeetingData next = device.getNextMeeting();

    String currentSummary, currentTime, currentHost;
    String nextSummary, nextTime;
    String status = device.getRoomStatus();
    String name = device.getRoomName();

    if (current.is_defined)
    {
        currentSummary = current.summary;
        currentTime = current.startTime + " - " + current.endTime;
        currentHost = "Hosted by " + current.host;
    }

    if (next.is_defined)
    {
        nextSummary = "Next: " + next.summary;
        nextTime = next.startTime + " - " + next.endTime;
    }

    display.clearDisplay();

    display.setCursor(10, 10);
    display.setTextSize(6);
    display.print(name);

    display.setCursor(600, 10);
    display.print(device.getTime());

    display.setCursor(10, 80);
    display.setTextSize(7);
    display.print(status);

    if (current.is_defined)
    {
        display.setTextSize(4);
        display.setCursor(10, 160);
        display.print(currentSummary);
        display.setCursor(10, 200);
        display.print(currentTime);
        display.setCursor(10, 240);
        display.print(currentHost);
    }

    if (next.is_defined)
    {
        display.setTextSize(4);
        display.setCursor(10, 300);
        display.print(nextSummary);
        display.setCursor(10, 340);
        display.print(nextTime);
    }

    display.display();
}