#include "Profiler.hpp"

using namespace std;

// Initiate static
map<string, Profiler::perf_counter> Profiler::counters;

Profiler::~Profiler()
{
    for (map<string, perf_counter>::iterator it = counters.begin(); it != counters.end(); ++it ){
        if (it->second.nr_runs != 0){
            cout << "Performance counter " << it->first << " averages: " << \
                chrono::duration_cast<chrono::milliseconds>(\
                    it->second.total_time).count()/(double)it->second.nr_runs << " ms" << endl;
        }
    }
}

// --------------

void Profiler::start_timer(string name)
{
    perf_counter* current_counter;
    try {
        current_counter = &counters.at(name);
        if (!current_counter->started){
            current_counter->started = true;
            current_counter->start_time = Clock::now();
        }
        else {
            Error::throw_warning(Error::performance_counter_already_started, name);
        }
    }
    catch (const out_of_range exception){
        perf_counter new_counter;
        new_counter.started = true;
        new_counter.start_time = Clock::now();
        counters[name] = new_counter;
    }
}

// ----------------

void Profiler::stop_timer(string name)
{
    perf_counter* current_counter;
    try {
        current_counter = &counters.at(name);
        if (current_counter->started){
            current_counter->started = false;
            current_counter->nr_runs++;
            current_counter->total_time += chrono::duration_cast<chrono::duration<double>>(\
                    Clock::now() - current_counter->start_time);
        }
        else {
            Error::throw_warning(Error::performance_counter_not_started, name);
        }
    }
    catch (const out_of_range exception){
        Error::throw_warning(Error::performance_counter_not_started, name);
    }
}
