#pragma once

#include <Arduino.h>

typedef unsigned int ErrorCode;

class Error
{
public:
    explicit Error(const char *message);
    explicit Error(const StringSumHelper &message);
    const char *message;
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