#include "main.h"

struct timespec *calculate_time_delta(
    struct timespec tick,
    struct timespec tock,
    struct timespec *delta
)
{
    delta->tv_nsec = tock.tv_nsec - tick.tv_nsec;
    delta->tv_sec  = tock.tv_sec - tick.tv_sec;

    if (delta->tv_sec > 0 && delta->tv_nsec < 0)
    {
        delta->tv_nsec += NS_PER_SECOND;
        delta->tv_sec--;
    }
    else if (delta->tv_sec < 0 && delta->tv_nsec > 0)
    {
        delta->tv_nsec -= NS_PER_SECOND;
        delta->tv_sec++;
    }
}

double total_time_taken()
{
    struct timespec runtime;

    runtime.tv_sec = time_stats.create.tv_sec +
                    time_stats.encode.tv_sec +
                    time_stats.fragment.tv_sec +
                    time_stats.decode.tv_sec;

    runtime.tv_nsec = time_stats.create.tv_nsec +
                    time_stats.encode.tv_nsec +
                    time_stats.fragment.tv_nsec +
                    time_stats.decode.tv_nsec;

    return runtime.tv_sec + runtime.tv_nsec * 1e-9;
}

void print_time_taken(int iter)
{
    printf(
        "create:    %lf µs/iter\n"
        "fragment:  %lf µs/iter\n"
        "encode:    %lf µs/iter\n"
        "decode:    %lf µs/iter\n"
        "total:     %lf µs/iter\n",
        (time_stats.create.tv_sec   + time_stats.create.tv_nsec     * 1e-9) / iter * 1e6,
        (time_stats.fragment.tv_sec + time_stats.fragment.tv_nsec   * 1e-9) / iter * 1e6,
        (time_stats.encode.tv_sec   + time_stats.encode.tv_nsec     * 1e-9) / iter * 1e6,
        (time_stats.decode.tv_sec   + time_stats.decode.tv_nsec     * 1e-9) / iter * 1e6,
        (time_stats.total.tv_sec    + time_stats.total.tv_nsec      * 1e-9) / iter * 1e6
    );
}
