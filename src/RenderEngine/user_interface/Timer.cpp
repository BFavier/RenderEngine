#include <RenderEngine/user_interface/Timer.hpp>
using namespace RenderEngine;

Timer::Timer()
{
    reset();
}

Timer::~Timer()
{
}

double Timer::t() const
{
    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - _start;
    return duration.count();
}

double Timer::peak_at_dt()
{
    std::chrono::time_point<std::chrono::steady_clock> now;
    now = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = now - _last;
    return duration.count();
}

double Timer::dt()
{
    std::chrono::time_point<std::chrono::steady_clock> now;
    now = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = now - _last;
    _last = now;
    return duration.count();
}

void Timer::reset_dt()
{
    _last = std::chrono::steady_clock::now();;
}

void Timer::reset()
{
    _start = std::chrono::steady_clock::now();
    _last = _start;
}