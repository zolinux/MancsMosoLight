#pragma once

#include "istate.hpp"

#include <stdint.h>

class StateActive : public IState
{
private:
    /* data */
public:
    StateActive();
    void tick() override;
    void gpio() override;
    void startMotor();
    void startMotor(uint8_t pwmPercent);
    void stopMotor();
    ~StateActive();
};