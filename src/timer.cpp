#include "timer.hpp"

#include <msp430.h>

namespace
{
    const uint16_t dividerMask = WDTIS0 | WDTIS1;
    const uint8_t timeShiftMantissa = 4;
    const uint16_t clockFrequency = 32768; // Hz
    const uint16_t intervalDivider[4] = {
        32768U,
        8192U,
        512U,
        64U,
    };

    uint32_t currentTime; // time (ms) * 2^timeShiftMantissa
    uint32_t timeToTrigger;
    bool isRunning()
    {
        return !(WDTCTL & WDTHOLD);
    }
    uint32_t getIntervalMs(const WdtInterval interval)
    {
        return (static_cast<uint32_t>(clockFrequency) << timeShiftMantissa) /
               intervalDivider[static_cast<uint8_t>(interval)];
    }
    void startInterval(WdtInterval interval)
    {
        auto reg = WDTCTL;
        reg &= ~(dividerMask | 0xFF00 | WDTHOLD);
        reg |= WDTPW | WDTTMSEL | WDTCNTCL | WDTSSEL | static_cast<uint8_t>(interval);
        WDTCTL = reg;
    }
    WdtInterval getInterval(uint32_t ms)
    {
        ms <<= timeShiftMantissa;
        int i = 0;
        while (i < 4)
        {
            const auto interval = static_cast<WdtInterval>(i);
            if (ms >= getIntervalMs(interval))
                return interval;
            i++;
        }
        return WdtInterval::MS2;
    }
}

uint32_t Timer::now()
{
    return currentTime >> timeShiftMantissa;
}

bool Timer::elapsed(const uint32_t since, const uint32_t period)
{
    const auto t = now();
    return t > since && t - since > period;
}

bool Timer::setInterval(WdtInterval interval)
{
    if (isRunning())
        return false;

    timeToTrigger = 0;
    startInterval(interval);
    return true;
}

bool Timer::setInterval(uint32_t ms)
{
    if (isRunning())
        return false;

    return setInterval(getInterval(ms));
}

void Timer::stop()
{
    WDTCTL = WDTPW | WDTHOLD;
}

void Timer::start()
{
}

bool Timer::tick()
{
    const auto delta = (static_cast<uint32_t>(clockFrequency) << timeShiftMantissa) /
                       intervalDivider[(WDTCTL >> WDTIS0) & 3];

    if (timeToTrigger)
    {
        if (timeToTrigger > delta)
            timeToTrigger -= delta;
        else
            timeToTrigger = 0;

        if (timeToTrigger)
        { // set next trigger timeout
            setInterval(getInterval(timeToTrigger));
        }
    }
    currentTime += delta;

    return !timeToTrigger;
}

void Timer::init()
{
    currentTime = 0;
    timeToTrigger = 0;
}

void Timer::reset()
{
    Timer::init();
}
