#include "blinker.hpp"
#include "context.hpp"
#include "gpio.hpp"
#include "motorControl.hpp"
#include "states.hpp"
#include "timer.hpp"

#include <msp430.h>

namespace
{
    volatile bool timeElapsed = false;
    volatile uint32_t swOnPressTime = 0;
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
    ctx.ledBlink.setCount(10);                        // at the end LED will be off
    ctx.ledErrBlink.setRate(3);                       // slower
    ctx.ledErrBlink.setCount(Blinker::CountInfinite); // continuous blinking

    while (true)
    {
        if (timeElapsed && swOnPressTime && !ctx.led && Timer::elapsed(swOnPressTime, 250U))
        {
            timeElapsed = false;
            swOnPressTime = 0;
        }
    }
#elif TEST == 4 // test motor
    Timer::init();
    Timer::setInterval(WdtInterval::MS250);
    ctx.led.clear();
    ctx.ledErr.clear();

    static const uint32_t runDuration = 5000U;
    volatile uint32_t motorTime = 0;
    volatile bool running = false;
    ctx.rotationMode = RotationMode::OneWay;

    while (true)
    {
        if (Timer::elapsed(motorTime, runDuration))
        {
            auto &m = motor();

            if (running)
            {
                // m.stopMotor();
                ctx.led.clear();
            }
            else
            {
                ctx.led.set();
                // m.startMotor();
            }
            running ^= true;

            motorTime = Timer::now();
            (void)m;
        }
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
        context().swOn.clearInterruptFlag();
        if (!swOnPressTime)
        {
            swOnPressTime = Timer::now();
            context().ledBlink.setCount(4);
        }
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
        auto &ctx = context();
        timeElapsed = true;
#if TEST == 3 || TEST == 4
        ctx.ledBlink.tick();
        ctx.ledErrBlink.tick();
#endif
#if TEST == 4
        //        motor().tick();
        ctx.ledErr.toggle();
#endif
    }
}
