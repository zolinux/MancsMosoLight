#pragma once

class IState
{
public:
    virtual void tick() = 0;
    virtual void gpio() = 0;

    ~IState() = default;
};
