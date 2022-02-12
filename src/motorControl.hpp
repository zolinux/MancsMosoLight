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
    void doStartMotor();
    void doStartMotor(uint8_t pwmPercent);
};

MotorControl &motor();
