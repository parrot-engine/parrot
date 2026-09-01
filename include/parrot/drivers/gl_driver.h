#ifndef PARROT_PARROT_INCLUDE_PARROT_DRIVERS_GL_DRIVER_H_
#define PARROT_PARROT_INCLUDE_PARROT_DRIVERS_GL_DRIVER_H_

#include "parrot/core/api.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct ParrotGLDriverContext ParrotGLDriverContext;
typedef struct ParrotGLDriver ParrotGLDriver;

struct ParrotGLDriver {
    /// Returns NULL if cannot create context
    ParrotGLDriverContext *(*create_context)(ParrotGLDriver *self, int major, int minor, int width, int height);
    void (*delete_context)(ParrotGLDriverContext *context);
    void (*use_context)(ParrotGLDriverContext *context);
};

/// Returns NULL if error
PARROT_API ParrotGLDriver *Parrot_x11_gl_driver_new(void);
PARROT_API void Parrot_x11_gl_driver_delete(ParrotGLDriver *self);

#endif // PARROT_PARROT_INCLUDE_PARROT_DRIVERS_GL_DRIVER_H_
