#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class MeetingData
{
public:
    bool is_defined;
    String summary;
    String host;
    String startTime, endTime;
    unsigned int start_timestamp, end_timestamp;
    MeetingData(const JsonVariant &meeting);
};

enum StateInfo
{
    UNKNOWN,
    MISSING_SESSION_ID,
    CONNECTION_CODE,
    DEVICE_REMOVED,
    NETWORK_ERROR,
    ROOMBELT_ERROR,
    SUCCESS,
};

class DeviceState
{
public:
    DeviceState(int status_code, const JsonDocument &response);
    StateInfo getState();

    String getConnectionCode();
    String getError();

    String getTime();
    String getRoomName();
    String getRoomStatus();
    MeetingData getCurrentMeeting();
    MeetingData getNextMeeting();

private:
    int status_code;
    JsonDocument response;
};
