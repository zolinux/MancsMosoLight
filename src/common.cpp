#include "blinker.hpp"
#include "context.hpp"
#include "gpio.hpp"
#include "motorControl.hpp"

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
        {1, 5, false, true},  // HN
        {1, 2, false, false}, // HP
        {1, 7, false, true},  // LN
        {1, 6, false, false}, // LP
        {&g_context.ledErr},
        {&g_context.led},
        0,
        Speed::Slow,
        RotationMode::OneWay,
        Direction::Forward,
    };

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

Context &context()
{
    return g_context;
}
