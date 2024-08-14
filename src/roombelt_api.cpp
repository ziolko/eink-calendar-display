#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "error.hpp"
#include "roombelt_api.hpp"
#include "storage.hpp"
#include "device_state.hpp"

// Provide WiFi credentials below. After the first run they will be
// stored in device memory and can be removed from the constants below
const char WIFI_SSID[] = "";
const char WIFI_PASSWORD[] = "";
const char TOKEN_STORAGE_KEY[16] = "token";

RoombeltApi::RoombeltApi()
{
    auto credentials = getWiFiCredentials();

    WiFi.mode(WIFI_STA);
    WiFi.begin(std::get<0>(credentials), std::get<1>(credentials));

    Serial.println("\nConnecting to WiFi");

    for (int i = 0; WiFi.status() != WL_CONNECTED && i < 300; i++)
    {
        Serial.print(".");
        delay(100);
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Unable to connect to Wifi. Status: " + WiFi.status());
        String message = "Unable to connect to Wifi. Status: " + String(WiFi.status());
        throw ErrorWifiConnection(message);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

RoombeltApi::~RoombeltApi()
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

DeviceState RoombeltApi::getDeviceState()
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
    return DeviceState(httpResponseCode, deviceState);
}

void RoombeltApi::assertWiFi()
{
    auto status = WiFi.status();
    if (status != WL_CONNECTED)
    {
        throw ErrorWifiConnection("WiFi is not connected. Current status: " + String(status));
    }
}

std::tuple<String, String> RoombeltApi::getWiFiCredentials()
{
    Storage storage;

    if (strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0)
    {
        Serial.println("Storing provided WiFi credentials");
        storage.setString("wifi_ssid", WIFI_SSID);
        storage.setString("wifi_password", WIFI_PASSWORD);
    }

    Serial.println("Loading stored WiFi credentials");
    String ssid = storage.getString("wifi_ssid");
    String password = storage.getString("wifi_password");

    if (ssid == "" || password == "")
    {
        throw ErrorStorage("Missing WiFi SSID or password");
    }

    return std::tuple{ssid, password};
}
