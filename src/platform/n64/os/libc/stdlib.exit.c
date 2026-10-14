#include "src/platform/n64/os/os.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_EXIT_FUNCS 64

static void (*exit_funcs[MAX_EXIT_FUNCS])(void);
static size_t exit_func_count = 0;

int atexit(void (*func)(void)) {
    if (exit_func_count >= MAX_EXIT_FUNCS) {
        return 1;
    }
    exit_funcs[exit_func_count++] = func;
    return 0;
}

void exit(int status) {
    for (size_t i = 0; i < exit_func_count; i++) {
        exit_funcs[i]();
    }

    PARROT_FAIL_FMT("exit(%d)", status);
}
