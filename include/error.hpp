#pragma once

#include <Arduino.h>

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
    using Error::Error;
};

class ErrorStorage : public Error
{
    using Error::Error;
};

class ErrorHttp : public Error
{
    using Error::Error;
};