#include "parrot/video/video.h"
#include "parrot/core/util.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool test;
} ParrotVideo;

static ParrotVideo *self = NULL;

void ParrotVideo_init(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    self = malloc(sizeof(ParrotVideo));
    memset(self, 0, sizeof(ParrotVideo));
}

void ParrotVideo_shutdown(void) {
    PARROT_FAIL_COND(ParrotVideo_is_initialized());

    free(self);
    self = NULL;
}

bool ParrotVideo_is_initialized(void) {
    return self;
}
