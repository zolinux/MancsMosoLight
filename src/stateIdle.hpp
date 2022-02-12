#pragma once

#include "istate.hpp"

class StateIdle : public IState
{
private:
    /* data */
public:
    StateIdle();
    void tick() override;
    void gpio() override;
    ~StateIdle();
};