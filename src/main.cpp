#include "blinker.hpp"
#include "context.hpp"
#include "gpio.hpp"
#include "motorControl.hpp"
#include "states.hpp"
#include "timer.hpp"

#include <msp430.h>

namespace
{
    States gApp;
}

int main(void)
{
    auto &ctx = context();
    __disable_interrupt();
    // stop WDT
    Timer::stop();

    // configure 32khz clock
    BCSCTL2 = SELM_3 | SELS;

    ctx.swOn.setInterruptEnabledEdge(true);
    ctx.swOn.setInterruptEnabled(true);
    ctx.swFunc.setInterruptEnabledEdge(true);
    ctx.swFunc.setInterruptEnabled(true);
    ctx.irIn.setInterruptEnabledEdge(false);

    ctx.ledBlink.setRate(1);
    ctx.ledBlink.setCount(10);
    Timer::init();
    // Timer::setInterval(500);
    // __low_power_mode_3();
    // ctx.led.clear();

    // // initialize main logic with parameters
    // Main m;
    // mainLogic = &m;

    // Timer::reset();
    // Timer::setInterval(WdtInterval::MS250);

    // go to sleep and wait for power switch
    // m.deepSleep();
    __enable_interrupt();

    while (true)
    {
        __low_power_mode_3();
        // everything is interrupt based, the MCU is sleeping in most time
    }
    return 0;
}

__interrupt_vec(PORT1_VECTOR) void port1_ISR(void)
{
    gApp.gpio();
}
__interrupt_vec(PORT2_VECTOR) void port2_ISR(void)
{
    gApp.gpio();
}
__interrupt_vec(ADC10_VECTOR) void adc_ISR(void)
{
}
__interrupt_vec(NMI_VECTOR) void nmi_ISR(void)
{
}
__interrupt_vec(TIMERA0_VECTOR) void ta0_ISR(void)
{
}
__interrupt_vec(TIMERA1_VECTOR) void ta1_ISR(void)
{
}
__interrupt_vec(WDT_VECTOR) void wdt_ISR(void)
{
    const auto alarm = Timer::tick();
    if (alarm)
    {
        auto &ctx = context();
        gApp.tick();
        ctx.ledBlink.tick();
        ctx.ledErrBlink.tick();
        motor().tick();
    }
}
