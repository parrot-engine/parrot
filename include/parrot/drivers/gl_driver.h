#ifndef PARROT_PARROT_INCLUDE_PARROT_DRIVERS_GL_DRIVER_H_
#define PARROT_PARROT_INCLUDE_PARROT_DRIVERS_GL_DRIVER_H_

#include "parrot/core/api.h"
#include <stdbool.h>
#include <stdint.h>

#define ParrotGLDriver_GL_VERSION(major, minor) ((((major) & 0xF) << 4) | ((minor) & 0xF))

typedef struct ParrotGLDriverContext ParrotGLDriverContext;

typedef struct {
    void (*init)(void);
    void (*shutdown)(void);

    ParrotGLDriverContext *(*create_context)(uint8_t version, int width, int height);
    void (*delete_context)(ParrotGLDriverContext *context);
    void (*use_context)(ParrotGLDriverContext *context);
} ParrotGLDriver;

PARROT_API const ParrotGLDriver *Parrot_gl_driver;

PARROT_API const ParrotGLDriver Parrot_x11_gl_driver;

#endif // PARROT_PARROT_INCLUDE_PARROT_DRIVERS_GL_DRIVER_H_
