#include "util.h"

#include <chrono>
#include <thread>

/*
 * Sleeps the current thread for up to `ns` nanoseconds. Remaining 
 * number of nanoseconds left to sleep are returned.
 *
 * Note: `ns` may be negative. In that case the remainder is negative too.
 *
 * Inspiration from https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
 */
int64_t Util::precise_sleep(int64_t ns)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::nanoseconds;
    using std::chrono::milliseconds;

    const int sleep_method_threshold_ns = 5000000; // 5 ms
    const int spin_lock_threshold_ns = 1000; // 1 us 

    // utilize OS sleep function for coarser sleeps
    while (ns > sleep_method_threshold_ns) {
        auto t_start = high_resolution_clock::now();
        std::this_thread::sleep_for(milliseconds(1));
        auto t_end = high_resolution_clock::now();

        int measured_ns = duration_cast<nanoseconds>(t_end - t_start).count();
        ns -= measured_ns;
    }

    // spin for finer grained sleeps
    auto t_start = high_resolution_clock::now();
    while (ns > spin_lock_threshold_ns) {
        ns -= duration_cast<nanoseconds>(high_resolution_clock::now() - t_start).count();
    }

    // return remaining nanoseconds left to sleep (< `spin_lock_threshold_ns`)
    return ns;
}
