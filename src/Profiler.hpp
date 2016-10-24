#ifndef PROFILER_HPP
#include <string>
#include <map>
#include <iostream>
#include <chrono>
#include "Error.hpp"

typedef std::chrono::high_resolution_clock Clock;

class Profiler
{
public:
    ~Profiler();
    static void start_timer(std::string name);
    static void stop_timer(std::string name);
private:
    struct perf_counter {
        bool started = false;
        Clock::time_point start_time;
        std::chrono::duration<double> total_time;
        unsigned nr_runs = 0;
    };
    static std::map <std::string, perf_counter> counters;
};

#endif // PROFILER_HPP
