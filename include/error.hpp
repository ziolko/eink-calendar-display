#pragma once

#include <Arduino.h>
#include <Inkplate.h>
#include <WiFiType.h>

class Error
{
public:
    explicit Error(const char *message);
    explicit Error(const String &message);
    explicit Error(const StringSumHelper &message);
    String message;
};

class ErrorWifiConnection : public Error
{
    uint status;

public:
    explicit ErrorWifiConnection(uint status);
    String getStatus();
};

class ErrorStorage : public Error
{
    using Error::Error;
};

class ErrorHttp : public Error
{
    using Error::Error;
};

extern const unsigned char ERROR_IMAGE[] PROGMEM;
