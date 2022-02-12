#pragma once

#include "blinker.hpp"
#include "gpio.hpp"

enum class Speed
{
    Slow,
    Normal,
    Fast,
};

enum class RotationMode
{
    OneWay,
    TwoWays,
};

enum class Direction
{
    Forward,
    Backward,
};
const uint8_t numberOfModes = 2;
const uint8_t numberOfDirections = 2;
const uint8_t pwmDutyVsSpeed[] = {45, 75, 100};
const uint8_t ledBlinkRatesVsSpeed[] = {3, 2, 1};
const uint8_t ledBlinkCountSpeedChange = 8U;
const uint16_t lowVoltageThreshold = 444U;

struct Context
{
    Gpio swOn;
    Gpio swFunc;
    Gpio irIn;
    Gpio ledErr;
    Gpio led;
    Gpio adcEn;
    Gpio irEn;
    Gpio motorHN;
    Gpio motorHP;
    Gpio motorLN;
    Gpio motorLP;

    Blinker ledErrBlink;
    Blinker ledBlink;

    uint32_t lastActivityTime;
    Speed speed;
    RotationMode rotationMode;
    Direction dir;

    bool enableAdc();
    bool disableAdc();
    uint16_t readAdc(const uint8_t channel);
    void blinkSpeedLed();
    void incrementSpeed();
};

Context &context();
