#pragma once

class States
{
public:
    States();
    void tick();
    void gpio();
    ~States();
};

States &app();