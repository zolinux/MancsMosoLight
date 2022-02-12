#include "StateIdle.hpp"

#include <msp430.h>

#include "context.hpp"
#include "timer.hpp"

namespace
{
    const uint32_t inactiveTimeShutdown = 3UL * 60 * 1000;
    const uint16_t buttonTimeShutdown = 3UL * 1000;

    enum class IdleState
    {
        Init,
    };

    IdleState state;
    bool irEnabled;
    uint32_t swOnPressedTime;

    void clear()
    {
        state = IdleState::Init;
        swOnPressedTime = 0;
        irEnabled = false;
    }

    void shutdown()
    {
        __disable_interrupt();
        context().irEn.clear();
        clear();
        setState(MainState::Boot);
        Timer::stop();
        __enable_interrupt();
    }
}

StateIdle::StateIdle(/* args */)
{
}

void StateIdle::tick()
{
    auto &c = context();
    if (Timer::elapsed(c.lastActivityTime, inactiveTimeShutdown))
    {
        // automatic turn-off after ... minutes
        shutdown();
        return;
    }

    if (!irEnabled)
    { // turn on infra gate
        irEnabled = true;
        c.irEn.set();
    }

    if (swOnPressedTime)
    { // still pressed
        if (c.swOn)
        {
            if (Timer::elapsed(swOnPressedTime, buttonTimeShutdown))
            { // button pressed long => turn off
                shutdown();
            }
        }
        else
        { // switch was released
            swOnPressedTime = 0;
            c.incrementSpeed();
            c.blinkSpeedLed();
            c.lastActivityTime = Timer::now();
        }
    }
}

void StateIdle::gpio()
{
    const auto t = Timer::now();
    context().lastActivityTime = t;

    if (context().swOn.interrupt())
    {
        swOnPressedTime = t;
        context().swOn.clearInterruptFlag();
    }
    else
    {
        swOnPressedTime = 0;
    }

    if (context().swFunc.interrupt())
    {
        context().swFunc.clearInterruptFlag();
    }
    else
    {
    }

    if (context().irIn.interrupt())
    {
        context().irIn.clearInterruptFlag();

        swOnPressedTime = 0;
        setState(MainState::Active);
    }
    else
    {
    }
}

StateIdle::~StateIdle()
{
}
