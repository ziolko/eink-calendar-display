#include <stdlib.h>
#include <Crypto.h>
#include <ArduinoJson.h>

#include "device_state.hpp"

DeviceState::DeviceState(int status_code, const JsonDocument &response)
{
    this->status_code = status_code;
    this->response = response;
}

StateInfo DeviceState::getState() const
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

String DeviceState::getConnectionCode() const
{
    return (response.isNull() || response["connectionCode"].isNull())
               ? String("")
               : response["connectionCode"];
}

String DeviceState::getError() const
{
    return (response.isNull() || response["error"].isNull())
               ? String("")
               : response["error"];
}

String DeviceState::getRoomName() const
{
    return response["name"].as<String>();
}

String DeviceState::getTime() const
{
    return response["time"].as<String>();
}

MeetingData DeviceState::getCurrentMeeting() const
{
    return MeetingData(response["current"]);
}

MeetingData DeviceState::getNextMeeting() const
{
    return MeetingData(response["next"]);
}

std::vector<MeetingData> DeviceState::getUpcomingMeetings() const
{
    std::vector<MeetingData> result;
    for (auto i = 0; i < response["later"].size(); i++)
    {
        result.push_back(MeetingData(response["later"][i]));
    }
    return result;
}

int DeviceState::getMsToNextRefresh() const
{
    return response["msToNextRefresh"].isNull() ? 0 : response["msToNextRefresh"];
}

bool DeviceState::isOccupied() const
{
    return !response["current"].isNull();
}

bool DeviceState::isEnergySaving() const
{
    return response["isEnergySaving"].isNull() ? false : response["isEnergySaving"];
}

MeetingData::MeetingData(const JsonVariantConst &meeting)
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

        is_all_day = meeting["isAllDayEvent"];
    }
    else
    {
        summary = "";
        host = "";
        startTime = "";
        endTime = "";
        start_timestamp = 0;
        end_timestamp = 0;
        is_all_day = false;
    }
}

#define HASH_SIZE 32
RTC_DATA_ATTR byte lastStateHash[HASH_SIZE];

bool DeviceStateHash::isEqualStoredHash(const DeviceState &state)
{
    byte hash[HASH_SIZE];
    computeHash(state, hash);

    for (int i = 0; i < HASH_SIZE; i++)
        if (hash[i] != lastStateHash[i])
            return false;

    return true;
}

void DeviceStateHash::storeHash(const DeviceState &state)
{
    computeHash(state, lastStateHash);
}

void DeviceStateHash::computeHash(const DeviceState &state, byte *result) const
{
    SHA256 hasher;
    String str;

    str += state.getState() + state.getError() + state.getRoomName() + state.getConnectionCode();

    MeetingData current = state.getCurrentMeeting();
    str += current.summary + current.startTime + current.endTime + current.host;

    MeetingData next = state.getNextMeeting();
    str += next.summary + next.startTime + next.endTime;

    for (auto meeting : state.getUpcomingMeetings())
    {
        str += meeting.summary + meeting.startTime + meeting.endTime;
    }

    hasher.doUpdate(str.c_str());
    hasher.doFinal(result);
}
