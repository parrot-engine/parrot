#ifndef PARROT_PARROT_INCLUDE_PARROT_DRIVERS_VIDEO_DRIVER_H_
#define PARROT_PARROT_INCLUDE_PARROT_DRIVERS_VIDEO_DRIVER_H_

#include "parrot/core/math.h"
#include "parrot/video/video.h"
#include <stdbool.h>
#include <stdint.h>
typedef struct ParrotVideoDriverViewport ParrotVideoDriverViewport;

typedef struct {
    void (*init)(void);
    void (*shutdown)(void);

    ParrotVideoDriverViewport *(*create_viewport)(int width, int height);
    void (*delete_viewport)(ParrotVideoDriverViewport *viewport);

    /**
     * Return value is RGBA8888 and guarnteed to live at least as long as the viewport which updates every call to this
     * function.
     */
    const uint32_t *(*get_viewport_pixels)(ParrotVideoDriverViewport *viewport);

    void (*set_viewport_texture)(
        ParrotVideoDriverViewport *viewport, int width, int height, const uint32_t *rgba8888, bool nearest_filter);
    void (*clear_viewport_texture)(ParrotVideoDriverViewport *viewport);

    void (*clear_viewport)(ParrotVideoDriverViewport *viewport, ParrotColor clear_color);

    void (*draw_viewport_vertices)(ParrotVideoDriverViewport *viewport,
                                   ParrotGMatSet matrix_set,
                                   const ParrotVideoVertex *vertices,
                                   size_t count);
} ParrotVideoDriver;

PARROT_API extern const ParrotVideoDriver Parrot_gl11_video_driver;

PARROT_API extern const ParrotVideoDriver *Parrot_video_driver;

#endif // PARROT_PARROT_INCLUDE_PARROT_DRIVERS_VIDEO_DRIVER_H_
