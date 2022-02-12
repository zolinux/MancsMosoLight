#include "MotorControl.hpp"

#include <msp430.h>

#include "context.hpp"
#include "timer.hpp"

namespace
{
    const uint16_t spinUpTime = 500U;
    const uint16_t spinDownTime = 500U;
    const uint16_t dirChangeTime = 3000U;

    enum class MotorState
    {
        Idle,
        Starting,
        Running,
        Stopping,
    };
    MotorState state;
    uint32_t startedAt;
    bool stopRequested;
}

MotorControl::MotorControl(/* args */)
{
}

void MotorControl::tick()
{
    if (state == MotorState::Idle)
        return;

    if (stopRequested)
    {
        state = MotorState::Idle;
        doStopMotor();
        return;
    }

    auto &c = context();

    switch (state)
    {
    case MotorState::Starting:
        if (!startedAt)
        {
            startedAt = Timer::now();
            doStartMotor();
        }
        else if (Timer::elapsed(startedAt, spinUpTime))
        { // spin-up done, switch to running
            startedAt = Timer::now();
            state = MotorState::Running;
            if (c.speed != Speed::Fast)
            {
                doStartMotor(pwmDutyVsSpeed[static_cast<uint8_t>(c.speed)]);
            }
        }

        break;
    case MotorState::Running:
        if (c.rotationMode == RotationMode::TwoWays && Timer::elapsed(startedAt, dirChangeTime))
        { // stop and reverse rotation
            startedAt = Timer::now();
            doStopMotor();
            state = MotorState::Stopping;
        }
        break;
    case MotorState::Stopping:
        if (Timer::elapsed(startedAt, spinDownTime))
        {
            startedAt = 0;
            state = MotorState::Starting;
            if (c.rotationMode == RotationMode::TwoWays)
            {
                c.dir =
                    static_cast<Direction>((static_cast<uint8_t>(c.dir) + 1) % numberOfDirections);
            }
        }
        break;
    case MotorState::Idle:
        break;
    }
}

void MotorControl::startMotor()
{
    if (state != MotorState::Idle)
        return;

    stopRequested = false;
    state = MotorState::Starting;
    startedAt = 0;

    tick();
}

void MotorControl::stopMotor()
{
    if (state == MotorState::Idle)
        return;

    stopRequested = true;
    tick();
}

void MotorControl::doStartMotor()
{
}

void MotorControl::doStartMotor(uint8_t pwmPercent)
{
    (void)pwmPercent;
}

void MotorControl::doStopMotor()
{
}

MotorControl::~MotorControl()
{
}
