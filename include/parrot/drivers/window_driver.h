#ifndef PARROT_PARROT_INCLUDE_PARROT_DRIVERS_WINDOW_DRIVER_H_
#define PARROT_PARROT_INCLUDE_PARROT_DRIVERS_WINDOW_DRIVER_H_

#include "parrot/core/api.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ParrotWindowDriverEventType_QUIT = 0,

    ParrotWindowDriverEventType_RESIZE,
} ParrotWindowDriverEventType;

typedef struct {
    ParrotWindowDriverEventType type;

    union {
        struct {
            int width;
            int height;

            int old_width;
            int old_height;
        } resize;
    } data;
} ParrotWindowDriverEvent;

typedef struct ParrotWindowDriverWindow ParrotWindowDriverWindow;

typedef struct {
    void (*init)(void);
    void (*shutdown)(void);

    ParrotWindowDriverWindow *(*create_window)(int width, int height);
    void (*delete_window)(ParrotWindowDriverWindow *window);

    bool (*poll_events)(ParrotWindowDriverWindow *window, ParrotWindowDriverEvent *out_event);

    void (*set_title)(ParrotWindowDriverWindow *window, const char *title);

    void (*set_size)(ParrotWindowDriverWindow *window, int width, int height);
    int (*get_width)(ParrotWindowDriverWindow *window);
    int (*get_height)(ParrotWindowDriverWindow *window);

    void (*set_image)(ParrotWindowDriverWindow *window, const uint32_t *rgbx8888);
} ParrotWindowDriver;

PARROT_API extern const ParrotWindowDriver *Parrot_window_driver;

PARROT_API extern const ParrotWindowDriver Parrot_x11_window_driver;

#endif // PARROT_PARROT_INCLUDE_PARROT_DRIVERS_WINDOW_DRIVER_H_
