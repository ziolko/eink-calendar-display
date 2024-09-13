#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "error.hpp"
#include "roombelt_api.hpp"
#include "storage.hpp"
#include "device_state.hpp"

const char TOKEN_STORAGE_KEY[16] = "token";

RoombeltApi::~RoombeltApi()
{
    WiFi.disconnect();
}

void RoombeltApi::connect(String ssid, String password)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("\nConnecting to WiFi");

    for (int i = 0; WiFi.status() != WL_CONNECTED && i < 300; i++)
    {
        Serial.print(".");
        delay(100);
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Unable to connect to Wifi. Status: " + WiFi.status());
        throw ErrorWifiConnection(WiFi.status());
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}
void RoombeltApi::disconnect()
{
    WiFi.disconnect();
}

void RoombeltApi::removeDevice()
{
    Storage storage;
    storage.setString(TOKEN_STORAGE_KEY, "");
}

void RoombeltApi::registerNewDevice()
{
    assertWiFi();

    Storage storage;

    CookieJar cookies;
    HTTPClient http;
    String serverPath = "https://app.roombelt.com/api/device";
    http.begin(serverPath.c_str());
    http.setCookieJar(&cookies);
    int httpResponseCode = http.POST("");

    if (httpResponseCode != 200)
    {
        throw ErrorHttp("Invalid error code while creating session token: " + String(httpResponseCode));
    }

    for (int i = 0; i < cookies.size(); i++)
    {
        if (cookies[i].name == "deviceSessionToken")
        {
            storage.setString(TOKEN_STORAGE_KEY, cookies[i].value.c_str());
            return;
        }
    }

    throw ErrorHttp("No session token header in the respones while creating session token");
}

DeviceState RoombeltApi::getDeviceState(int retryCount)
{
    assertWiFi();

    Storage storage;
    HTTPClient http;
    String serverPath = "https://app.roombelt.com/api/device/summary";
    String sessionToken = storage.getString(TOKEN_STORAGE_KEY);

    http.begin(serverPath.c_str());
    http.addHeader("Cookie", "deviceSessionToken=" + sessionToken + ";");

    int httpResponseCode = http.GET();

    JsonDocument deviceState;
    if (httpResponseCode == 200)
    {
        String response = http.getString();
        deserializeJson(deviceState, response);
    }
    http.end();

    // Retry in case of e.g. network glitch
    if (httpResponseCode != 200 && httpResponseCode != 403 && httpResponseCode != 418 && retryCount > 0)
    {
        delay(500);
        return getDeviceState(retryCount - 1);
    }

    return DeviceState(httpResponseCode, deviceState);
}

void RoombeltApi::assertWiFi()
{
    auto status = WiFi.status();
    if (status != WL_CONNECTED)
    {
        throw ErrorWifiConnection(status);
    }
}
