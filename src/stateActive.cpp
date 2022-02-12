#include "StateActive.hpp"

#include <msp430.h>

#include "context.hpp"
#include "timer.hpp"

namespace
{
    const uint16_t spinUpTime = 500U;
    const uint16_t spinDownTime = 500U;
    const uint16_t dirChangeTime = 3000U;
    const uint16_t runWithoutPawTime = 3000U;

    enum class ActiveState
    {
        Starting,
        Running,
        Stopping,
    };
    ActiveState state;
    uint32_t irInAssertTime;
    uint32_t startedAt;
    bool stopRequested;
}

StateActive::StateActive(/* args */)
{
}

void StateActive::tick()
{
    auto &c = context();
    if (c.irIn)
    {
        irInAssertTime = Timer::now();
    }

    if (irInAssertTime && Timer::elapsed(irInAssertTime, runWithoutPawTime))
    {
        stopRequested = true;
        irInAssertTime = 0;
    }

    switch (state)
    {
    case ActiveState::Starting:
        if (!startedAt)
        {
            startedAt = Timer::now();
            startMotor();
        }
        else if (Timer::elapsed(startedAt, spinUpTime))
        { // spi-up done, switch to running
            startedAt = Timer::now();
            state = ActiveState::Running;
            if (c.speed != Speed::Fast)
            {
                startMotor(pwmDutyVsSpeed[static_cast<uint8_t>(c.speed)]);
            }
        }
        break;
    case ActiveState::Running:
        if (stopRequested ||
            (c.rotationMode == RotationMode::TwoWays && Timer::elapsed(startedAt, dirChangeTime)))
        { // stop and reverse rotation
            startedAt = Timer::now();
            stopMotor();
            state = ActiveState::Stopping;
        }
        break;
    case ActiveState::Stopping:
        if (Timer::elapsed(startedAt, spinDownTime))
        {
            startedAt = 0;
            state = ActiveState::Starting;
            if (!stopRequested)
            {
                // ToDo: update rotation dir
            }
        }
        break;
    }

    if (stopRequested && !startedAt)
    {
        // clear states and go idle
        state = ActiveState::Starting;
        irInAssertTime = 0;
        stopRequested = false;
        setState(MainState::Idle);
    }
}

void StateActive::gpio()
{
    const auto t = Timer::now();
    if (context().irIn.interrupt())
    {
        context().irIn.clearInterruptFlag();
        if (!irInAssertTime && !startedAt && !stopRequested)
        {
            irInAssertTime = t;
            startedAt = t;
            state = ActiveState::Starting;
            stopRequested = false;
            startMotor();
        }
    }
    else
    {
    }

    // ToDo: handle speed and mode change
}

void StateActive::startMotor()
{
}

void StateActive::startMotor(uint8_t pwmPercent)
{
    (void)pwmPercent;
}

void StateActive::stopMotor()
{
}

StateActive::~StateActive()
{
}
