#pragma once

#include "istate.hpp"

class StateBoot : public IState
{
private:
    /* data */
public:
    StateBoot();
    void tick() override;
    void gpio() override;
    ~StateBoot();
};