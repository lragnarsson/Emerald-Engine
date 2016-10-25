#include "Profiler.hpp"

using namespace std;

// Initiate static
map<string, Profiler::perf_counter> Profiler::counters;

Profiler::~Profiler()
{
    print_results();
}

// --------------

void Profiler::print_results(){
#ifdef PROFILING
    cout << endl << "PERFORMANCE PROFILING"<< endl << "-----------" << endl;
    
    for (map<string, perf_counter>::iterator it = counters.begin(); it != counters.end(); ++it ){
        perf_counter values = it->second;
        if (values.nr_runs != 0){
            double average = chrono::duration_cast<chrono::nanoseconds>(values.total_time).count()/((double)values.nr_runs*(double)1000000);
            double max = chrono::duration_cast<chrono::nanoseconds>(values.max_time).count()/(double)1000000;

            cout << "Performance counter " << it->first << " averages: " << \
                average << " ms. " \
                "Longest run was " << \
                max << " ms." << endl;
        }
    }
#endif
    return;
}

// -------------

void Profiler::start_timer(string name)
{
#ifdef PROFILING
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
#endif
    return;
}

// ----------------

void Profiler::stop_timer(string name)
{
#ifdef PROFILING
    perf_counter* current_counter;
    try {
        current_counter = &counters.at(name);
        if (current_counter->started){
            current_counter->started = false;
            current_counter->nr_runs++;
            
            // Get delta for current time
            chrono::duration<double> current_time_delta = chrono::duration_cast<chrono::duration<double>>(\
                    Clock::now() - current_counter->start_time);

            if (current_time_delta > current_counter->max_time){
                current_counter->max_time = current_time_delta;
            }
            
            current_counter->total_time += current_time_delta;
        }
        else {
            Error::throw_warning(Error::performance_counter_not_started, name);
        }
    }
    catch (const out_of_range exception){
        Error::throw_warning(Error::performance_counter_not_started, name);
    }
#endif
    return;
}
