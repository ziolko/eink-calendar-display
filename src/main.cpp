#include <stdlib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Inkplate.h>

#include "storage.h"
#include "network.h"

Inkplate display(INKPLATE_1BIT);

void displayDevice(DeviceState &device);
void displayStatusCode(DeviceState &device);

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

    init_wifi();
}

void loop()
{
    // Max key length with null terminator - 16
    char TOKEN_STORAGE_KEY[16] = "token";

    Serial.println("LOOP");

    delay(1000);
    Serial.println("Get token");

    String token = load_string(TOKEN_STORAGE_KEY, "session-key");
    Serial.println("token: " + token);

    delay(1000);
    DeviceState device = get_device_state(token);
    Serial.println("\nGet device done");

    if (device.needsNewConnectionCode())
    {
        token = create_session_token();
        save_string(TOKEN_STORAGE_KEY, token.c_str());
        device = get_device_state(token);
    }

    if (device.isConnected())
    {
        displayDevice(device);
    }
    else if (device.getConnectionCode().length() > 0)
    {
        displayStatusCode(device);
    }
    else
    {
        Serial.println("isNetworkError");

        display.clearDisplay();

        display.setCursor(10, 10);
        display.setTextSize(6);
        display.printf("Network error");

        display.display();
    }

    delay(30000);
}

void displayStatusCode(DeviceState &device)
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
    String status = device.getStatus();
    String name = device.getName();

    if (!current.is_empty)
    {
        currentSummary = current.summary;
        currentTime = current.startTime + " - " + current.endTime;
        currentHost = "Hosted by " + current.host;
    }

    if (!next.is_empty)
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

    if (!current.is_empty)
    {
        display.setTextSize(4);
        display.setCursor(10, 160);
        display.print(currentSummary);
        display.setCursor(10, 200);
        display.print(currentTime);
        display.setCursor(10, 240);
        display.print(currentHost);
    }

    if (!next.is_empty)
    {
        display.setTextSize(4);
        display.setCursor(10, 300);
        display.print(nextSummary);
        display.setCursor(10, 340);
        display.print(nextTime);
    }

    display.display();
}