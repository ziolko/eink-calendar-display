#include "device_state.hpp"

DeviceState::DeviceState(int status_code, const JsonDocument &response)
{
    this->status_code = status_code;
    this->response = response;
}

StateInfo DeviceState::getState()
{
    if (status_code == 0)
        return StateInfo::NETWORK_ERROR;
    if (status_code == 403)
        return StateInfo::MISSING_SESSION_ID;
    if (status_code == 418)
        return StateInfo::DEVICE_REMOVED;
    if (status_code == 200 && !response["time"].isNull())
        return StateInfo::SUCCESS;
    if (status_code == 200 && !response["connectionCode"].isNull())
        return StateInfo::CONNECTION_CODE;
    if (status_code == 200 && !response["error"].isNull())
        return StateInfo::ROOMBELT_ERROR;

    return StateInfo::UNKNOWN;
}

String DeviceState::getConnectionCode()
{
    return (response.isNull() || response["connectionCode"].isNull())
               ? String("")
               : response["connectionCode"];
}

String DeviceState::getError()
{
    return (response.isNull() || response["error"].isNull())
               ? String("")
               : response["error"];
}

String DeviceState::getRoomName()
{
    return response["name"].as<String>();
}

String DeviceState::getTime()
{
    return response["time"].as<String>();
}

String DeviceState::getRoomStatus()
{
    return response["status"].as<String>();
}

MeetingData DeviceState::getCurrentMeeting()
{
    return MeetingData(response["current"].as<JsonVariant>());
}

MeetingData DeviceState::getNextMeeting()
{
    return MeetingData(response["next"].as<JsonVariant>());
}

MeetingData::MeetingData(const JsonVariant &meeting)
{
    is_defined = !meeting.isNull();

    if (is_defined)
    {
        summary = meeting["summary"].as<String>();
        host = meeting["host"].as<String>();

        startTime = meeting["start"].as<String>();
        endTime = meeting["end"].as<String>();

        start_timestamp = meeting["startTimestamp"];
        end_timestamp = meeting["endTimestamp"];
    }
}