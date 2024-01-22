#pragma once
#include <chrono>

namespace RenderEngine
{
    class Timer
    {
    public:
        Timer();
        ~Timer();
        //!< Time in seconds since creation of the timer.
        double t() const;
        //!< Time in seconds since last call to dt (or since creation time)
        double peak_dt();
        //!< Time in seconds since last call to dt (or since creation time)
        double dt();
        // Reset the dt timer
        void reset_dt();
        //!< Reset the t and dt timers.
        void restart();
    private:
        std::chrono::time_point<std::chrono::steady_clock> _start;
        std::chrono::time_point<std::chrono::steady_clock> _last;
    protected:
    };
}
