#include "mainLogic.hpp"
#include "gpio.hpp"
#include "timer.hpp"

#include <msp430.h>

Main::Main()
{
}

void Main::onTimerFired(void)
{
}

void Main::run()
{
}

void Main::deepSleep()
{
    Timer::reset();
    _low_power_mode_4();
}
