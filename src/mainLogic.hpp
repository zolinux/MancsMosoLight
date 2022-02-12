#pragma once

#include <cstdint>

class Main
{
private:
public:
    explicit Main();
    ~Main() = default;

    /**
     * @brief called on system timer interrupt (WDT)
     */
    void onTimerFired(void);

    /**
     * @brief start state machine
     *
     */
    void run();

    /**
     * @brief enter deep sleep with resetting time
     *
     */
    void deepSleep();
};
