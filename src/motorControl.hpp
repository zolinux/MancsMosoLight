#pragma once

#include <stdint.h>

class MotorControl
{
private:
    /* data */
public:
    MotorControl();
    void tick();
    void startMotor();
    void stopMotor();
    ~MotorControl();

private:
    void doStopMotor();

    /**
     * @brief Start motor with full power
     *
     */
    void doStartMotor();

    /**
     * @brief Drive motor by PWM
     *
     * @param pwmPercent duty cycle [%]
     */
    void doStartMotor(uint8_t pwmPercent);
};

MotorControl &motor();
