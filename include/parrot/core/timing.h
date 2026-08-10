#ifndef PARROT_INCLUDE_PARROT_CORE_TIMING_H_
#define PARROT_INCLUDE_PARROT_CORE_TIMING_H_

#include "parrot/core/api.h"
#include <stdint.h>

PARROT_API uint64_t Parrot_get_performance_counter(void);
PARROT_API uint64_t Parrot_get_performance_frequency(void);

PARROT_API void Parrot_sleep(float seconds);

#endif // PARROT_INCLUDE_PARROT_CORE_TIMING_H_
