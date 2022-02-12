#include "context.hpp"
#include "gpio.hpp"
#include "motorControl.hpp"
#include "states.hpp"
#include "timer.hpp"
#include <msp430.h>

namespace
{
    Context g_context{
        {2, 5, true},
        {1, 4, true},
        {1, 0, true},
        {2, 3, false, false},
        {2, 4, false, false},
        {2, 1, false, false},
        {1, 3, false, false},
        {1, 5, false, false},
        {1, 2, false, false},
        {1, 7, false, false},
        {1, 6, false, false},
        {&g_context.ledErr},
        {&g_context.led},
        0,
        Speed::Slow,
        RotationMode::OneWay,
        Direction::Forward,
    };

    States g_app;
    MotorControl g_motor;
}

// Definition of the error function to call if the constructor goes bonkers
extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}
MotorControl &motor()
{
    return g_motor;
}

States &app()
{
    return g_app;
}

int main(void)
{
    _disable_interrupts();
    // stop WDT
    Timer::stop();

#ifdef TEST
    P1DIR |= 0x01; // Set P1.0 to output direction

    for (;;)
    {
        volatile unsigned int i; // volatile to prevent optimization

        P1OUT ^= 0x01; // Toggle P1.0 using exclusive-OR

        i = 10000; // SW Delay
        do
            i--;
        while (i != 0);
    }
#else

    g_context.swOn.setInterruptEnabledEdge(true);
    g_context.swOn.setInterruptEnabled(true);
    g_context.swFunc.setInterruptEnabledEdge(true);
    g_context.swFunc.setInterruptEnabled(true);
    g_context.irIn.setInterruptEnabledEdge(false);

    g_context.ledBlink.setRate(1);
    g_context.ledBlink.setCount(10);
    Timer::init();
    // Timer::setInterval(500);
    // __low_power_mode_3();
    // g_context.led.clear();

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
    g_app.gpio();
}
__interrupt_vec(PORT2_VECTOR) void port2_ISR(void)
{
    g_app.gpio();
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
        g_app.tick();
        g_context.ledBlink.tick();
        g_context.ledErrBlink.tick();
        g_motor.tick();
    }
}

Context &context()
{
    return g_context;
}
