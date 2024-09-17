#include <stdlib.h>
#include <Inkplate.h>
#include <SdFat.h>
#include <ArduinoJson.h>

#include "display.hpp"

#include "fonts/roboto_16.h"
#include "fonts/roboto_24.h"
#include "fonts/roboto_36.h"
#include "fonts/roboto_48.h"
#include "fonts/roboto_64.h"

#define uS_TO_mS_FACTOR 1000

#define SCREEN_ROTATION 3
#define SCREEN_PADDING_RIGHT 0
#define SCREEN_PADDING_BOTTOM 55

Inkplate display(INKPLATE_3BIT);

Display::Display()
{
    hasChanges = false;

    display.begin();
    display.clearDisplay();
    display.setTextColor(BLACK);
    display.setRotation(SCREEN_ROTATION);
}

Display::~Display()
{
    commit();
}

double Display::getBattery()
{
    return display.readBattery();
}

void Display::deepSleep(uint sleepTimeMs)
{
    if (display.getSdCardOk())
    {
        display.sdCardSleep();
    }

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);                 // wake up on the "wake up" side button
    esp_sleep_enable_timer_wakeup(sleepTimeMs * uS_TO_mS_FACTOR); // wake up on the RTC clock
    esp_deep_sleep_start();
}

void Display::showImage(const String &fileName)
{
    initMicroSD();

    display.drawImage(fileName, 0, 0);
    hasChanges = true;
    commit();
}

void Display::showRandomImage()
{
    initMicroSD();

    SdFile root, file;

    std::vector<String> files;
    char fileNameBuffer[100];
    root.open("/");

    while (file.openNext(&root, O_RDONLY))
    {
        if (file.getName(fileNameBuffer, 100) && String(fileNameBuffer).endsWith(".bmp"))
            files.push_back(fileNameBuffer);

        file.close();
    }

    root.close();

    if (files.size() > 0)
        showImage(files.at(random(files.size())));
}

void Display::showImage(const unsigned char *imageData, uint x, uint y, uint width, uint height)
{
    display.drawBitmap(x, y, imageData, width, height, WHITE);
    hasChanges = true;
}

Config Display::getConfig()
{
    initMicroSD();

    SdFile file;
    if (!file.open("/config.json", O_RDONLY))
        throw Error("No config.json file on the micro SD card.");

    auto size = file.fileSize();
    if (size > 1024)
        throw Error("config.json file is too large.");

    char buffer[size];
    file.read(buffer, size);

    JsonDocument config;
    deserializeJson(config, buffer);

    return Config{
        ssid : config["ssid"],
        password : config["password"],
    };
}

void Display::initMicroSD()
{
    if (!display.getSdCardOk())
    {
        if (!display.sdCardInit())
        {
            throw Error("Unable to initialize the micro SD card.");
        }
    }
}

void Display::showErrorScreen(const Error &error)
{
    this->showMessageScreen("There was an error", error.message);
    commit();
}

void Display::showMessageScreen(const String &message)
{
    printOnCenter(message, Font::ROBOTO_48);
    commit();
}

void Display::showMessageScreen(const String &message, const String &secondaryMessage)
{
    auto primaryFont = Font::ROBOTO_48;
    auto secondaryFont = Font::ROBOTO_24;
    auto margin = 20;

    setCurrentFont(primaryFont);
    int16_t primaryX, primaryY;
    uint16_t primaryWidth, primaryHeight;
    display.getTextBounds(message, 0, 0, &primaryX, &primaryY, &primaryWidth, &primaryHeight);

    setCurrentFont(secondaryFont);
    int16_t secondaryX, secondaryY;
    uint16_t secondaryWidth, secondaryHeight;
    display.getTextBounds(secondaryMessage, 0, 0, &secondaryX, &secondaryY, &secondaryWidth, &secondaryHeight);

    auto totalHeight = primaryHeight + secondaryHeight + margin;
    auto screenWidth = display.width() - SCREEN_PADDING_RIGHT;
    auto screenHeight = display.height() - SCREEN_PADDING_BOTTOM;

    setCurrentFont(primaryFont);
    display.setCursor(screenWidth / 2 - primaryWidth / 2 - primaryX, screenHeight / 2 - totalHeight / 2 - primaryY);
    display.print(message);

    setCurrentFont(secondaryFont);
    display.setCursor(screenWidth / 2 - secondaryWidth / 2 - secondaryX, screenHeight / 2 - totalHeight / 2 - primaryY + margin + primaryHeight);
    display.print(secondaryMessage);

    hasChanges = true;
    commit();
}

void Display::setCurrentFont(Font font)
{
    switch (font)
    {
    case Font::ROBOTO_16:
        return display.setFont(&Roboto_16);
    case Font::ROBOTO_24:
        return display.setFont(&Roboto_24);
    case Font::ROBOTO_36:
        return display.setFont(&Roboto_36);
    case Font::ROBOTO_48:
        return display.setFont(&Roboto_48);
    case Font::ROBOTO_64:
        return display.setFont(&Roboto_64);
    }
}

void Display::printOnCenter(const String &text, Font font)
{
    auto screenWidth = display.width() - SCREEN_PADDING_RIGHT;
    auto screenHeight = display.height() - SCREEN_PADDING_BOTTOM;

    setCurrentFont(font);
    int16_t x1, y1;
    uint16_t width, height;
    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
    display.setCursor(screenWidth / 2 - width / 2 - x1, screenHeight / 2 - height / 2 - y1);
    display.print(text);

    hasChanges = true;
}

void Display::print(const String &message, uint x, uint y, Font font, TextAlign textAlign, bool wrapText)
{
    setCurrentFont(font);
    display.setTextWrap(wrapText);

    int16_t x1, y1;
    uint16_t width, height;

    String text = message;
    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    auto screenWidth = display.width() - SCREEN_PADDING_RIGHT;

    Serial.println("Starting measuring for text: " + message);
    // Add ellipsis if text wrapping is disabled
    int i = 0;
    while (!wrapText && i < message.length() - 1 && width > screenWidth - x)
    {
        text = message.substring(0, message.length() - i) + "...";
        display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
        i++;
    }

    if (textAlign == TextAlign::CENTER)
        display.setCursor(screenWidth / 2 - width / 2 - x1, y - y1);
    else if (textAlign == TextAlign::LEFT)
        display.setCursor(x - x1, y - y1);
    else if (textAlign == TextAlign::RIGHT)
        display.setCursor(screenWidth - x - x1 - width, y - y1);

    display.print(text);

    hasChanges = true;
}

void Display::commit()
{
    if (!hasChanges)
    {
        return;
    }

    display.display();
    display.clearDisplay();
    hasChanges = false;
}

void Display::showDeviceScreen(const DeviceState &device)
{
    // Special case if there are no meetings today at all
    if (!device.getCurrentMeeting().is_defined && !device.getNextMeeting().is_defined)
    {
        print("Wolne", 0, 280, Font::ROBOTO_64, TextAlign::CENTER);
        print("do konca dnia", 0, 360, Font::ROBOTO_48, TextAlign::CENTER);
        display.drawThickLine(280, 362, 288, 355, BLACK, 3);
        commit();
        return;
    }

    int screenHeight = display.height() - SCREEN_PADDING_BOTTOM;
    auto upcoming = device.getUpcomingMeetings();
    auto displayedUpcoming = std::min(2, (int)upcoming.size());
    auto remainingUpcoming = std::max(0, (int)upcoming.size() - displayedUpcoming);

    auto endOfFirstRow = std::max(300, screenHeight / (displayedUpcoming + 2));
    auto upcomingRowHeight = std::max(160, (screenHeight - endOfFirstRow) / (displayedUpcoming + 1));

    printCurrentMeeting(device, 0, endOfFirstRow);

    for (int i = 0; i < displayedUpcoming; i++)
    {
        auto startY = endOfFirstRow + i * upcomingRowHeight;
        display.drawThickLine(0, startY, display.width(), startY, BLACK, 3);
        printNextMeeting(upcoming[i], startY, startY + upcomingRowHeight);
    }

    {
        auto startY = endOfFirstRow + displayedUpcoming * upcomingRowHeight;
        display.drawThickLine(0, startY, display.width(), startY, BLACK, 3);
        printRemainingMeetingsCount(remainingUpcoming, startY, screenHeight);
    }

    commit();
}

void Display::showConnectionCodeScreen(const DeviceState &device)
{
    print("Connection Code", 0, 190, Font::ROBOTO_48, TextAlign::CENTER);
    print(device.getConnectionCode(), 0, 280, Font::ROBOTO_64, TextAlign::CENTER);
    print("Log in to app.roombelt.com", 0, 370, Font::ROBOTO_36, TextAlign::CENTER);
    print("and use the connection code there.", 0, 410, Font::ROBOTO_36, TextAlign::CENTER);
    return commit();
}

std::vector<String> Display::measureLineBreak(const String &input, Font font, int margin)
{
    auto result = std::vector<String>();
    int lastWhitespace = 0;
    int16_t x1, y1;
    uint16_t width, height;

    setCurrentFont(font);
    display.setTextWrap(false);

    Serial.println("Measuring " + input);
    for (int i = 1; i < input.length(); i++)
    {
        if (input[i] == ' ' && input[i - 1] != ' ')
        {
            lastWhitespace = i;
            continue;
        }

        display.getTextBounds(input.substring(0, i + 1), 0, 0, &x1, &y1, &width, &height);

        Serial.println("i = " + String(i) + " width = " + String(width) + " x1 = " + x1 + " whitespace = " + lastWhitespace + " : " + input.substring(0, i));

        if (width >= display.width() - 2 * margin)
        {
            if (lastWhitespace == 0)
            {
                result.push_back(input.substring(0, i));
                result.push_back(input.substring(i));
            }
            else
            {
                result.push_back(input.substring(0, lastWhitespace));
                result.push_back(input.substring(lastWhitespace + 1));
            }

            result[0].trim();
            result[1].trim();
            return result;
        }
    }

    result.push_back(input);
    return result;
}

void Display::printCurrentMeeting(const DeviceState &deviceState, int startY, int endY)
{
    auto current = deviceState.getCurrentMeeting();

    if (!current.is_defined)
    {
        auto textY = startY + (endY - startY - 120) / 2;

        print("Wolne", 0, textY, Font::ROBOTO_64, TextAlign::CENTER);
        print("do " + deviceState.getNextMeeting().startTime, 0, textY + 80, Font::ROBOTO_48, TextAlign::CENTER);
        return;
    }

    auto lines = measureLineBreak(current.summary, Font::ROBOTO_48, 20);

    if (lines.size() == 1)
    {
        auto textY = startY + (endY - startY - 115) / 2;
        print(current.summary, 20, textY, Font::ROBOTO_48, TextAlign::CENTER, false);
        print(current.startTime + " - " + current.endTime, 20, textY + 65, Font::ROBOTO_48, TextAlign::CENTER);
    }
    else
    {
        auto textY = startY + (endY - startY - 160) / 2;
        print(lines[0], 20, textY, Font::ROBOTO_48, TextAlign::CENTER, false);
        print(lines[1], 20, textY + 55, Font::ROBOTO_48, TextAlign::CENTER, false);
        print(current.startTime + " - " + current.endTime, 20, textY + 120, Font::ROBOTO_48, TextAlign::CENTER);
    }
}

void Display::printNextMeeting(const MeetingData &meeting, int startY, int endY)
{

    auto textY = startY + (endY - startY - 80) / 2;

    if (meeting.is_defined)
    {
        print(meeting.summary, 20, textY, Font::ROBOTO_36, TextAlign::CENTER, false);
        print(meeting.startTime + " - " + meeting.endTime, 20, textY + 50, Font::ROBOTO_36, TextAlign::CENTER);
    }
}

void Display::printRemainingMeetingsCount(int count, int startY, int endY)
{
    if (count > 0)
    {
        auto yPos = startY + (endY - startY) / 2 - 15;
        auto meetings = count == 1 ? " rezerwacja " : count < 5 ? " rezerwacje "
                                                                : " rezerwacji ";
        print("Jeszcze " + String(count) + meetings + "dzisiaj", 0, yPos, Font::ROBOTO_36, TextAlign::CENTER);
    }
    else
    {
        auto textY = startY + (endY - startY) / 2 - 36;
        print("Potem wolne", 20, textY, Font::ROBOTO_36, TextAlign::CENTER);
        print("do konca dnia", 20, textY + 40, Font::ROBOTO_36, TextAlign::CENTER);
    }
}
