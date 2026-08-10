#include "parrot/core/timing.h"
#include <math.h>
#include <stdint.h>

#define __USE_POSIX199309
#include <time.h>

uint64_t Parrot_get_performance_counter(void) {
    struct timespec time;
    clock_gettime(/* Would use CLOCK_MONOTONIC_RAW if not linux specific */ CLOCK_MONOTONIC, &time);
    return time.tv_nsec + (time.tv_sec * 1e9L);
}

uint64_t Parrot_get_performance_frequency(void) {
    struct timespec resolution;
    clock_getres(CLOCK_MONOTONIC, &resolution);
    return resolution.tv_nsec != 0 ?
               1e9L / resolution.tv_nsec :
               /* Probability of this happening? Less than 1/1,000,000,000 but just to be safe */ 1e9;
}

void Parrot_sleep(float seconds) {
    struct timespec time = {
        .tv_sec = seconds,
        .tv_nsec = (long)(fmod(seconds, 1.0) * 1e9),
    };
    nanosleep(&time, NULL);
}
