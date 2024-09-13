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
    MeetingData(const JsonVariantConst &meeting);
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
    StateInfo getState() const;

    String getConnectionCode() const;
    String getError() const;

    String getTime() const;
    String getRoomName() const;
    MeetingData getCurrentMeeting() const;
    MeetingData getNextMeeting() const;
    std::vector<MeetingData> getUpcomingMeetings() const;

    int getMsToNextRefresh() const;
    bool isOccupied() const;
    bool isEnergySaving() const;

private:
    int status_code;
    JsonDocument response;
};

class DeviceStateHash
{
public:
    bool isEqualStoredHash(const DeviceState &state);
    void storeHash(const DeviceState &state);

private:
    void computeHash(const DeviceState &state, byte *result) const;
};