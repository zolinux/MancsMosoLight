#include "stateBoot.hpp"

#include <msp430.h>

#include "context.hpp"
#include "timer.hpp"

namespace
{
    enum class BootState
    {
        Init,
        W4ButtonRelease,
    };
    BootState state;
}

StateBoot::StateBoot(/* args */)
{
}

void StateBoot::tick()
{
    if (state == BootState::Init)
    {
        context().enableAdc();
        const auto adcVal = context().readAdc(0);
        if (lowVoltageThreshold > adcVal)
        {
            // blink red led
            context().ledErrBlink.setRate(4);
            context().ledErrBlink.setCount(Blinker::CountInfinite);
        }
        else
        {
            context().blinkSpeedLed();
        }
    }
    else
    { // clr interrupt for buttons and enter idle mode
        context().swOn.clearInterruptFlag();
        context().swFunc.clearInterruptFlag();
        context().lastActivityTime = Timer::now();
        setState(MainState::Idle);
    }
}

void StateBoot::gpio()
{
    state = BootState::Init;
    Timer::setInterval(WdtInterval::MS250);
}

StateBoot::~StateBoot()
{
}
