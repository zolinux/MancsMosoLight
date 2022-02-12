#pragma once

#include <cstdint>
#include <msp430.h>

class Gpio
{
    struct port_t
    {
        volatile unsigned char in;
        volatile unsigned char out;
        volatile unsigned char dir;
        volatile unsigned char ifg;
        volatile unsigned char ies;
        volatile unsigned char ie;
        volatile unsigned char sel;
    } __attribute__((__packed__));

    port_t *const m_port;
    const uint8_t m_bit;
    const uint8_t m_mask;

public:
    Gpio(const uint8_t port, const uint8_t bit)
    : m_port(reinterpret_cast<port_t *>(port == 1 ? P1IN : P2IN)), m_bit(bit), m_mask(1 << bit)
    {
    }
    Gpio(const uint8_t port, const uint8_t bit, bool in) : Gpio(port, bit)
    {
        setDirection(in);
    }
    Gpio(const uint8_t port, const uint8_t bit, bool in, bool value) : Gpio(port, bit, in)
    {
        value ? set() : clear();
    }
    void set()
    {
        m_port->out |= m_mask;
    }
    void clear()
    {
        m_port->out &= ~m_mask;
    }
    void toggle()
    {
        m_port->out ^= m_mask;
    }

    void setDirection(bool in)
    {
        const auto cv = m_port->dir;
        m_port->dir = in ? cv & ~m_mask : cv | m_mask;
    }
    void setSelection(bool alt)
    {
        const auto cv = m_port->sel;
        m_port->sel = alt ? cv | m_mask : cv & ~m_mask;
    }

    void setInterruptEnabled(bool enabled)
    {
        const auto cv = m_port->ie;
        m_port->ie = enabled ? cv | m_mask : cv & ~m_mask;
    }

    void setInterruptEnabledEdge(bool rising)
    {
        const auto cv = m_port->ies;
        m_port->ies = rising ? cv & ~m_mask : cv | m_mask;
    }

    bool interrupt() const
    {
        return m_port->ifg & m_mask;
    }
    void clearInterruptFlag()
    {
        m_port->ifg &= ~m_mask;
    }
    constexpr uint8_t mask()
    {
        return m_mask;
    }

    operator bool() const
    {
        return m_port->in & m_mask;
    }
};
