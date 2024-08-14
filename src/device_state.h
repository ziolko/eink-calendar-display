#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class MeetingData
{
public:
    bool is_empty;
    String summary;
    String host;
    String startTime, endTime;
    unsigned int start_timestamp, end_timestamp;
    MeetingData(const JsonVariant &meeting);
};

class DeviceState
{
public:
    DeviceState(int status_code, const JsonDocument &response);

    bool isNetworkError();
    bool isConnected();
    bool needsNewConnectionCode();
    String getConnectionCode();
    String getName();
    String getTime();
    String getStatus();
    MeetingData getCurrentMeeting();
    MeetingData getNextMeeting();

private:
    int status_code;
    JsonDocument response;
};
