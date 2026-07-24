#include "parrot/video/video.h"
#include "parrot/core/util.h"

void ParrotVideo_init(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());
}

void ParrotVideo_shutdown(void) {
    PARROT_FAIL_COND(ParrotVideo_is_initialized());
}

bool ParrotVideo_is_initialized() {
    return false;
}