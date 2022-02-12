#include "states.hpp"
#include "motorControl.hpp"

#include <msp430.h>

#include "context.hpp"
#include "timer.hpp"

namespace
{
    const uint32_t inactiveTimeShutdown = 3UL * 60 * 1000;
    const uint16_t buttonTimePowerOn = 3UL * 1000 / 2; // 1.5seconds
    const uint16_t buttonTimeShutdown = 3UL * 1000;
    const uint16_t runWithoutPawTime = 3000U;
    const uint16_t infraSensorActivationTime = 500U;

    void handleBoot();
    void handleIdle();
    void handleActive();

    using StateHandler = void (*)();
    const StateHandler stateHandlers[] = {&handleBoot, &handleIdle, &handleActive};

    enum class MainState
    {
        Boot,
        Idle,
        Active,
    };

    MainState state;
    bool irEnabled;
    bool powerOn;
    uint32_t irAssertedTime;
    uint32_t irDeAssertedTime;
    uint32_t swOnPressedTime;
    uint32_t swFuncPressedTime;

    void clear()
    {
        state = MainState::Boot;
        swOnPressedTime = 0;
        irEnabled = false;
    }

    void shutdown()
    {
        __disable_interrupt();
        context().disableAdc();
        context().irIn.setInterruptEnabled(false);
        context().irEn.clear();
        powerOn = false;
        irDeAssertedTime = 0;
        clear();
        state = MainState::Boot;
        Timer::stop();
        __enable_interrupt();
    }

    void handleBoot()
    {
        if (!powerOn)
            return;

        auto &c = context();

        c.adcEn.set();
        c.enableAdc();
        __nop();
        __nop();
        __nop();
        __nop();
        const auto adcVal = c.readAdc(0);
        if (lowVoltageThreshold > adcVal)
        {
            // blink red led
            c.ledErrBlink.setRate(4);
            c.ledErrBlink.setCount(Blinker::CountInfinite);
        }
        else
        {
            c.blinkSpeedLed();
            c.ledBlink.setCount(ledBlinkCountSpeedChange + 1); // leave LEO on after finish
        }

        state = MainState::Idle;
        swOnPressedTime = 0;
        irEnabled = false;
        c.irEn.set();
    }

    void handleIdle()
    {
        auto &c = context();
        if (Timer::elapsed(c.lastActivityTime, inactiveTimeShutdown))
        {
            // automatic turn-off after ... minutes
            shutdown();
            return;
        }

        if (!irEnabled && Timer::elapsed(c.lastActivityTime, infraSensorActivationTime))
        { // turn on infra gate
            irEnabled = true;
            c.irIn.setInterruptEnabled(true);
        }

        if (irAssertedTime)
        {
            state = MainState::Active;
            motor().startMotor();
        }
    }

    void handleActive()
    {
        const auto &c = context();
        if (!c.irIn && !irDeAssertedTime)
        {
            irDeAssertedTime = Timer::now();
            irAssertedTime = 0;
        }
        if (irDeAssertedTime && Timer::elapsed(irDeAssertedTime, runWithoutPawTime))
        {
            irDeAssertedTime = 0;
            state = MainState::Idle;
            motor().stopMotor();
        }
    }
}

States::States()
{
}

void States::tick()
{
    // do common things
    auto &c = context();

    if (swOnPressedTime)
    { // still pressed
        if (c.swOn)
        {
            if (state == MainState::Boot)
            {
                if (Timer::elapsed(swOnPressedTime, buttonTimePowerOn))
                { // button pressed long => turn on
                    powerOn = true;
                    c.lastActivityTime = Timer::now();
                }
            }
            else if (Timer::elapsed(swOnPressedTime, buttonTimeShutdown))
            { // button pressed long => turn off
                shutdown();
            }
        }
        else
        { // switch was released
            swOnPressedTime = 0;
            if (state == MainState::Boot)
            { // no power-up yet, it was too short -> go back to off
                Timer::stop();
            }
            else
            { // speed change
                c.incrementSpeed();
                c.blinkSpeedLed();
                c.lastActivityTime = Timer::now();
            }
        }
    }

    if (state != MainState::Boot && swFuncPressedTime)
    { // still pressed
        if (c.swFunc)
        { // change rotation mode
            c.rotationMode = static_cast<RotationMode>(
                (static_cast<uint8_t>(c.rotationMode) + 1) % numberOfModes);
            swFuncPressedTime = 0;
        }
    }

    // call specific state handler
    stateHandlers[static_cast<uint8_t>(state)]();
}

void States::gpio()
{
    const auto t = Timer::now();
    auto &c = context();
    c.lastActivityTime = t;

    if (c.swOn.interrupt())
    {
        Timer::start();
        if (c.swOn)
            swOnPressedTime = t;
        c.swOn.clearInterruptFlag();
    }

    if (c.swFunc.interrupt())
    {
        if (c.swOn)
            swFuncPressedTime = t;
        c.swFunc.clearInterruptFlag();
    }

    if (c.irIn.interrupt())
    {
        c.irIn.clearInterruptFlag();
        if (c.irIn)
        {
            irAssertedTime = t;
            irDeAssertedTime = 0;
        }
    }

    tick();
}

States::~States()
{
}
