#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "error.hpp"
#include "roombelt_api.hpp"
#include "storage.hpp"
#include "device_state.hpp"

const char TOKEN_STORAGE_KEY[16] = "token";
const char ROOT_CA[] = "-----BEGIN CERTIFICATE-----\n"
                       "MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n"
                       "VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n"
                       "A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n"
                       "WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n"
                       "IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n"
                       "AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n"
                       "QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n"
                       "HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n"
                       "BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n"
                       "9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n"
                       "p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"
                       "-----END CERTIFICATE-----\n";

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
    http.begin(serverPath.c_str(), ROOT_CA);
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

DeviceState RoombeltApi::getDeviceState(int retryCount, double battery)
{
    assertWiFi();

    Storage storage;
    HTTPClient http;
    String serverPath = "https://app.roombelt.com/api/device/summary?battery=" + String(battery);
    String sessionToken = storage.getString(TOKEN_STORAGE_KEY);

    http.begin(serverPath.c_str(), ROOT_CA);
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
        return getDeviceState(retryCount - 1, battery);
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
