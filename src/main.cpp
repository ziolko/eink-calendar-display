#include <stdlib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "Inkplate.h"

JsonDocument getDeviceState();
void initWiFi();

Inkplate display(INKPLATE_1BIT);

void setup()
{
    display.begin();
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(150, 320);
    display.setTextSize(4);
    display.print("Welcome to Roombelt!");
    display.display(); // Write hello message
    initWiFi();
}

void loop()
{
    JsonDocument device = getDeviceState();
    Serial.println("\nGet device done");

    static String lastValues;

    String currentSummary, currentTime, currentHost;
    String nextSummary, nextTime;
    String status = device["status"].as<String>();

    if (!device["current"].isNull())
    {
        currentSummary = device["current"]["summary"].as<String>();
        currentTime = device["current"]["start"].as<String>() + " - " + device["current"]["end"].as<String>();
        currentHost = "Hosted by " + device["current"]["host"].as<String>();
    }

    if (!device["next"].isNull())
    {
        nextSummary = "Next: " + device["next"]["summary"].as<String>();
        nextTime = device["next"]["start"].as<String>() + " - " + device["next"]["end"].as<String>();
    }

    display.clearDisplay();

    display.setCursor(10, 10);
    display.setTextSize(6);
    display.print(status);

    display.setCursor(600, 10);
    display.print(device["time"].as<String>());

    if (!device["current"].isNull())
    {
        display.setTextSize(4);
        display.setCursor(10, 80);
        display.print(currentSummary);
        display.setCursor(10, 120);
        display.print(currentTime);
        display.setCursor(10, 160);
        display.print(currentHost);
    }

    if (!device["next"].isNull())
    {
        display.setCursor(10, 220);
        display.print(nextSummary);
        display.setCursor(10, 260);
        display.print(nextTime);
    }

    display.display();

    delay(30000);
}

void initWiFi()
{
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA); // Optional
    WiFi.begin("WIFI_SSID", "WIFI_PASSWORD");
    Serial.println("\nConnecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

JsonDocument getDeviceState()
{
    HTTPClient http;
    String serverPath = "https://app.roombelt.com/api/device/summary";
    http.begin(serverPath.c_str());
    http.addHeader("Cookie", "deviceSessionToken=todo;");
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
        JsonDocument deviceState;
        deserializeJson(deviceState, response);
        http.end();
        return deviceState;
    }
    else
    {
        http.end();
        JsonDocument emptyResult;
        return emptyResult;
    }
}

// Small function that will write on the screen what function is currently in demonstration.
void displayCurrentAction(String text)
{
    display.setTextSize(2);
    display.setCursor(2, 580);
    display.print(text);
}
