#pragma once

#include "error.hpp"
#include "device_state.hpp"

enum Font
{
    ROBOTO_16 = 16,
    ROBOTO_24 = 24,
    ROBOTO_36 = 36,
    ROBOTO_48 = 48,
    ROBOTO_64 = 64,
};

enum TextAlign
{
    LEFT = 0,
    RIGHT,
    CENTER,
};

struct Config
{
    String ssid;
    String password;
};

class Display
{
public:
    Display();
    ~Display();

    void showImage(const String &fileName);
    void showRandomImage();
    void showErrorScreen(const Error &error);
    void showDeviceScreen(const DeviceState &deviceState);
    void showConnectionCodeScreen(const DeviceState &deviceState);
    void showMessageScreen(const String &message);
    void showMessageScreen(const String &message, const String &secondaryMessage);

    void deepSleep(uint sleepTimeMs);
    double readBattery();

    Config getConfig();

private:
    bool hasChanges;

    void printCurrentMeeting(const DeviceState &deviceState);
    void printNextMeeting(const DeviceState &deviceState);

    void printOnCenter(const String &text, Font font);
    void print(const String &text, uint x, uint y, Font font = Font::ROBOTO_24, TextAlign textAlign = TextAlign::LEFT, bool wrapText = true);

    void setCurrentFont(Font font);
    void commit();
    void initMicroSD();
};