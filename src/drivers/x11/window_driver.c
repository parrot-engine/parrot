#include "parrot/drivers/window_driver.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/dbe.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ParrotWindowDriverWindow {
    Display *display;

    int screen;
    Window window;

    int width;
    int height;
    int old_height;

    Atom wm_delete;

    bool close_requested;

    GC gc;
    XdbeBackBuffer back_buffer;

    uint32_t *image_data;
    XImage *image;
};

static void driver_init(void) {
}

static void driver_shutdown(void) {
}

static ParrotWindowDriverWindow *driver_create_window(int width, int height) {
    ParrotWindowDriverWindow *self = malloc(sizeof(ParrotWindowDriverWindow));
    PARROT_RET_COND_V(!self, NULL);
    memset(self, 0, sizeof(ParrotWindowDriverWindow));

    self->width = width;
    self->height = height;

    self->display = XOpenDisplay(NULL);
    PARROT_FAIL_NULL_MSG(self->display, "Failed to open X11 display");

    self->screen = DefaultScreen(self->display);
    Window root = RootWindow(self->display, self->screen);

    self->window = XCreateSimpleWindow(self->display,
                                       root,
                                       0,
                                       0,
                                       width,
                                       height,
                                       1,
                                       BlackPixel(self->display, self->screen),
                                       BlackPixel(self->display, self->screen));
    XSelectInput(self->display, self->window, ExposureMask);

    XMapWindow(self->display, self->window);

    self->wm_delete = XInternAtom(self->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(self->display, self->window, &self->wm_delete, 1);

    self->gc = XCreateGC(self->display, self->window, 0, NULL);
    self->back_buffer = XdbeAllocateBackBufferName(self->display, self->window, XdbeBackground);

    self->image_data = calloc(width * height, sizeof(uint32_t));
    self->image = XCreateImage(self->display,
                               DefaultVisual(self->display, self->screen),
                               DefaultDepth(self->display, self->screen),
                               ZPixmap,
                               0,
                               (char *)self->image_data,
                               width,
                               height,
                               32,
                               0);

    XPutImage(self->display, self->window, self->gc, self->image, 0, 0, 0, 0, width, height);
    XFlush(self->display);

    return self;
}

static void driver_delete_window(ParrotWindowDriverWindow *self) {
    XDestroyImage(self->image);

    XdbeDeallocateBackBufferName(self->display, self->back_buffer);
    XFreeGC(self->display, self->gc);

    XDestroyWindow(self->display, self->window);

    XCloseDisplay(self->display);

    free(self);
}

static bool driver_poll_events(ParrotWindowDriverWindow *self, ParrotWindowDriverEvent *out_event) {
    XEvent event;
    while (XPending(self->display)) {
        XNextEvent(self->display, &event);
        switch (event.type) {
        case ClientMessage: {
            if ((Atom)event.xclient.data.l[0] == self->wm_delete) {
                *out_event = (ParrotWindowDriverEvent){
                    .type = ParrotWindowDriverEventType_QUIT,
                };
                return true;
            }
        } break;
        case Expose: {
            XWindowAttributes attrs;
            XGetWindowAttributes(self->display, self->window, &attrs);

            if (attrs.width != self->width || attrs.height != self->height) {
                *out_event = (ParrotWindowDriverEvent){
                    .type = ParrotWindowDriverEventType_RESIZE,
                    .data.resize =
                        {
                            .width = attrs.width,
                            .height = attrs.height,
                            .old_width = self->width,
                            .old_height = self->height,
                        },
                };

                XDestroyImage(self->image);

                self->image_data = calloc(attrs.width * attrs.height, sizeof(uint32_t));
                self->image = XCreateImage(self->display,
                                           DefaultVisual(self->display, self->screen),
                                           DefaultDepth(self->display, self->screen),
                                           ZPixmap,
                                           0,
                                           (char *)self->image_data,
                                           attrs.width,
                                           attrs.height,
                                           32,
                                           0);

                self->width = attrs.width;
                self->height = attrs.height;

                return true;
            }
        } break;
        }
    }

    return false;
}

static void driver_set_title(ParrotWindowDriverWindow *self, const char *title) {
    Atom netWmName = XInternAtom(self->display, "_NET_WM_NAME", False);
    Atom utf8 = XInternAtom(self->display, "UTF8_STRING", False);
    XChangeProperty(self->display,
                    self->window,
                    netWmName,
                    utf8,
                    8,
                    PropModeReplace,
                    (const unsigned char *)title,
                    strlen(title) * sizeof(char));
}

static void driver_set_size(ParrotWindowDriverWindow *self, int width, int height) {
    XResizeWindow(self->display, self->window, width, height);
}

static int driver_get_width(ParrotWindowDriverWindow *self) {
    return self->width;
}

static int driver_get_height(ParrotWindowDriverWindow *self) {
    return self->height;
}

static void driver_set_image(ParrotWindowDriverWindow *self, const uint32_t *rgbx8888) {
    PARROT_FAIL_NULL(self);

    XWindowAttributes attrs;
    XGetWindowAttributes(self->display, self->window, &attrs);

    for (int y = 0; y < attrs.height; y++) {
        for (int x = 0; x < attrs.width; x++) {
            uint8_t r = rgbx8888[y * attrs.width + x] & 0xFF;
            uint8_t g = (rgbx8888[y * attrs.width + x] >> 8) & 0xFF;
            uint8_t b = (rgbx8888[y * attrs.width + x] >> 16) & 0xFF;
            self->image_data[y * attrs.width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
    XPutImage(self->display, self->back_buffer, self->gc, self->image, 0, 0, 0, 0, attrs.width, attrs.height);

    XdbeSwapInfo swap_info = {self->window, XdbeBackground};
    XdbeSwapBuffers(self->display, &swap_info, 1);
}

const ParrotWindowDriver Parrot_x11_window_driver = {
    .init = driver_init,
    .shutdown = driver_shutdown,

    .create_window = driver_create_window,
    .delete_window = driver_delete_window,

    .poll_events = driver_poll_events,

    .set_title = driver_set_title,

    .set_size = driver_set_size,
    .get_width = driver_get_width,
    .get_height = driver_get_height,

    .set_image = driver_set_image,
};
