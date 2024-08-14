#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "storage.h"
#include "device_state.h"

// Provide WiFi credentials below. After the first run they will be
// stored in device memory and can be removed from the constants below
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

void init_wifi()
{
    if (strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0)
    {
        Serial.println("Storing provided WiFi credentials");

        save_string("wifi_ssid", WIFI_SSID);
        save_string("wifi_password", WIFI_PASSWORD);
    }

    Serial.println("Loading stored WiFi credentials");
    String ssid = load_string("wifi_ssid", "");
    String password = load_string("wifi_password", "");

    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

String create_session_token()
{
    CookieJar cookies;

    HTTPClient http;
    String serverPath = "https://app.roombelt.com/api/device";
    http.begin(serverPath.c_str());
    http.setCookieJar(&cookies);
    int httpResponseCode = http.POST("");

    Serial.println("Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode != 200)
    {
        // TODO - handle errors
        return "";
    }

    Serial.println("Cookies size");
    Serial.println(cookies.size());

    for (int i = 0; i < cookies.size(); i++)
    {
        Serial.println(cookies[i].name + ": " + cookies[i].value);

        if (cookies[i].name == "deviceSessionToken")
        {
            return cookies[i].value;
        }
    }

    return "";
}

DeviceState get_device_state(const String &sessionToken)
{
    HTTPClient http;
    String serverPath = "https://app.roombelt.com/api/device/summary";

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
