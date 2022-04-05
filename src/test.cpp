#include "blinker.hpp"
#include "common.hpp"
#include "gpio.hpp"
#include "states.hpp"
#include "timer.hpp"

#include <msp430.h>

namespace
{
    volatile bool timeElapsed = false;
    volatile bool swOnPressed = false;
}

int main(void)
{
    auto &ctx = context();
    _disable_interrupts();
    // stop WDT
    Timer::stop();

    // configure 32khz clock
    BCSCTL2 = SELM_3 | SELS;
    __enable_interrupt();

#if TEST == 1      // test basic MCU functionality
    P2DIR |= 0x18; // Set LEDs to output direction
    P2OUT |= 0x18;
    P2SEL &= (~0x18);
    for (;;)
    {
        volatile unsigned int i; // volatile to prevent optimization

        P2OUT ^= 0x18; // Toggle P1.0 using exclusive-OR

        i = 10000; // SW Delay
        do
            i--;
        while (i != 0);
    }
#elif TEST == 2 // test GPIO in/out, timer
    uint8_t ctr = 1U;

    ctx.led.clear();
    ctx.ledErr.set();
    Timer::init();
    Timer::setInterval(WdtInterval::MS1000);

    for (;;)
    {
        ctr++;
        ctx.ledErr << static_cast<bool>(ctr & 1);
        ctx.led.toggle();
        while (!timeElapsed)
        {
            if (ctx.swOn)
                ctx.led.toggle();
        }
        timeElapsed = false;
    }
#elif TEST == 3 // test blinking and GPI interrupt
    Timer::init();
    Timer::setInterval(WdtInterval::MS250);

    ctx.swOn.setInterruptEnabledEdge(true);
    ctx.swOn.setInterruptEnabled(true);

    ctx.led.clear();
    ctx.ledBlink.setRate(1);
    ctx.ledBlink.setCount(10); // at the end LED will be off

    while (true)
    {
        if (timeElapsed)
        {
            if (swOnPressed)
            {
                ctx.ledBlink.setCount(4);
                swOnPressed = false;
            }
            timeElapsed = false;
        }
    }
#else

    ctx.swFunc.setInterruptEnabledEdge(true);
    ctx.swFunc.setInterruptEnabled(true);
    ctx.irIn.setInterruptEnabledEdge(false);

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
#endif
    return 0;
}

__interrupt_vec(PORT1_VECTOR) void port1_ISR(void)
{
}
__interrupt_vec(PORT2_VECTOR) void port2_ISR(void)
{
#if TEST == 3
    if (context().swOn.interrupt())
    {
        swOnPressed = true;
        context().swOn.clearInterruptFlag();
    }
#endif
    // P2IFG = 0;
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
        timeElapsed = true;
#if TEST == 3
        auto &ctx = context();
        ctx.ledBlink.tick();
        ctx.ledErrBlink.tick();
#endif
    }
}
