#include "parrot/core/util.h"
#include <stdlib.h>

static void Parrot_crash_handler_impl(const char *cause) {
    (void)cause;
    abort();
}

ParrotCrashHandlerFunc Parrot_crash_handler = Parrot_crash_handler_impl;