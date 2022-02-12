#pragma once
#include <cstdint>

enum class WdtInterval
{
    MS1000,
    MS250,
    MS16,
    MS2
};

class Timer
{
private:
    friend void wdt_ISR(void);

    /**
     * @brief count time
     *
     * @return true if wait time elapsed or free-running mode
     * @return false if there is still time to alarm
     */
    static bool tick();

public:
    static void init();
    static void reset();

    /**
     * @brief return current time
     *
     * @return milliseconds
     */
    static uint32_t now();

    static bool elapsed(const uint32_t since, const uint32_t);

    /**
     * @brief Set periodic timer interrupt Interval. Running timer must be stopped
     * before setting new timeout. Implicitly starts the timer
     *
     * @param interval in milliseconds (rounded)
     * @return true success
     * @return false timer is already running.
     */
    static bool setInterval(WdtInterval interval);

    /**
     * @brief Set the next timer interrupt to occur in ... msecs. Running timer must be stopped
     * before setting new timeout.
     *
     * @param ms time after interrupt to be triggered
     * @return true success
     * @return false timer is already running.
     */
    static bool setInterval(uint32_t ms);

    /**
     * @brief stop timer
     */
    static void stop();

    /** @brief start interval timer with current settings
     */
    static void start();
};
