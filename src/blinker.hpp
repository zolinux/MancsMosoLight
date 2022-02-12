#pragma once

#include "gpio.hpp"

class Blinker
{
private:
    Gpio *m_gpio;
    uint8_t m_count; // 0xFF->infinite
    uint8_t m_rate;  // how many ticks to keep the level
    uint8_t m_ctr;

public:
    static const uint8_t CountInfinite = UINT8_MAX;
    Blinker(Gpio *gpio) : m_gpio(gpio), m_count(CountInfinite), m_rate(2), m_ctr(0)
    {
    }
    ~Blinker() = default;

    void setRate(uint8_t ticksPerLevel)
    {
        m_rate = ticksPerLevel;
        m_ctr = m_rate;
    }
    void setCount(uint8_t count)
    {
        m_count = count;
    }

    void tick()
    {
        if (!m_count)
            return;

        if (--m_ctr == 0)
        {
            m_ctr = m_rate;
            m_gpio->toggle();
            if (m_count != CountInfinite)
                m_count--;
        }
    }
};
