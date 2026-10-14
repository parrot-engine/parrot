#include "parrot/core/util.h"

void Parrot_n64_os_assert(const char *file, int line, const char *func, const char *expr) {
    Parrot_crash_handler("(%s:%d in %s) Assertion failed: %s", file, line, func, expr);
}
