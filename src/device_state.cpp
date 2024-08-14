#include "device_state.h"

DeviceState::DeviceState(int status_code, const JsonDocument &response)
{
    this->status_code = status_code;
    this->response = response;
}

bool DeviceState::isNetworkError()
{
    return status_code != 200 && status_code != 403 && status_code != 418;
}

bool DeviceState::isConnected()
{
    return status_code == 200 && !response.isNull() && !response["time"].isNull();
}

bool DeviceState::needsNewConnectionCode()
{
    return status_code == 403 || status_code == 418;
}

String DeviceState::getConnectionCode()
{
    return (response.isNull() || response["connectionCode"].isNull())
               ? String("")
               : response["connectionCode"];
}

String DeviceState::getName()
{
    assert(isConnected());
    return response["name"].as<String>();
}

String DeviceState::getTime()
{
    assert(isConnected());
    return response["time"].as<String>();
}

String DeviceState::getStatus()
{
    assert(isConnected());
    return response["status"].as<String>();
}

MeetingData DeviceState::getCurrentMeeting()
{
    assert(isConnected());
    return MeetingData(response["current"].as<JsonVariant>());
}

MeetingData DeviceState::getNextMeeting()
{
    assert(isConnected());
    return MeetingData(response["next"].as<JsonVariant>());
}

MeetingData::MeetingData(const JsonVariant &meeting)
{
    is_empty = meeting.isNull();

    if (!is_empty)
    {
        summary = meeting["summary"].as<String>();
        host = meeting["host"].as<String>();

        startTime = meeting["start"].as<String>();
        endTime = meeting["end"].as<String>();

        start_timestamp = meeting["startTimestamp"];
        end_timestamp = meeting["endTimestamp"];
    }
}