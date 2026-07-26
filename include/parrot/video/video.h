#ifndef __SRC_PARROT_INCLUDE_PARROT_VIDEO_VIDEO_H_
#define __SRC_PARROT_INCLUDE_PARROT_VIDEO_VIDEO_H_

#include <stdbool.h>

void ParrotVideo_init(void);
void ParrotVideo_shutdown(void);
bool ParrotVideo_is_initialized(void);

void ParrotVideo_render(void);

#endif // __SRC_PARROT_INCLUDE_PARROT_VIDEO_VIDEO_H_