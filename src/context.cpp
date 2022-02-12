#include "context.hpp"

#include <msp430.h>

uint16_t Context::readAdc(const uint8_t channel)
{
    const uint16_t ch = static_cast<uint16_t>(channel) << 12;
    ADC10CTL1 = ch;
    __nop();
    __nop();
    ADC10CTL0 |= ENC | ADC10SC;
    __nop();

    while (ADC10CTL1 & ADC10BUSY)
    {
    }

    return ADC10MEM;
}

void Context::blinkSpeedLed()
{
    ledBlink.setRate(ledBlinkRatesVsSpeed[static_cast<uint8_t>(speed)]);
    ledBlink.setCount(ledBlinkCountSpeedChange);
}

void Context::incrementSpeed()
{
    speed = static_cast<Speed>((static_cast<uint8_t>(speed) + 1) % sizeof(ledBlinkRatesVsSpeed));
}

bool Context::enableAdc()
{
    ADC10CTL0 = ADC10ON | REFON | SREF_1;
    ADC10AE = ADC10AE0;
    return true;
}

bool Context::disableAdc()
{
    ADC10CTL0 = 0;
    return true;
}
