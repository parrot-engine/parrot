#include "src/video/platform.h"
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

struct ParrotVideoWindow {
    Display *display;

    int screen;
    Window window;

    int old_width;
    int old_height;

    Atom wm_delete;

    bool close_requested;

    GC gc;
    XdbeBackBuffer back_buffer;

    uint32_t *image_data;
    XImage *image;
};

ParrotVideoWindow *ParrotVideoWindow_new(int width, int height) {
    ParrotVideoWindow *self = malloc(sizeof(ParrotVideoWindow));
    PARROT_RET_COND_V(!self, NULL);
    memset(self, 0, sizeof(ParrotVideoWindow));

    self->old_width = width;
    self->old_height = height;

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

void ParrotVideoWindow_delete(ParrotVideoWindow *self) {
    XDestroyImage(self->image);

    XdbeDeallocateBackBufferName(self->display, self->back_buffer);
    XFreeGC(self->display, self->gc);

    XDestroyWindow(self->display, self->window);

    XCloseDisplay(self->display);

    free(self);
}

void ParrotVideoWindow_poll_events(ParrotVideoWindow *self) {
    XEvent event;
    while (XPending(self->display)) {
        XNextEvent(self->display, &event);
        switch (event.type) {
        case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == self->wm_delete) {
                self->close_requested = true;
            }
            break;
        case Expose: {
            int width = ParrotVideoWindow_get_width(self);
            int height = ParrotVideoWindow_get_height(self);

            if (width != self->old_width || height != self->old_height) {
                XDestroyImage(self->image);

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

                self->old_width = width;
                self->old_height = height;
            }
            break;
        }
        }
    }
}

bool ParrotVideoWindow_should_close(ParrotVideoWindow *self) {
    bool value = self->close_requested;
    self->close_requested = false;
    return value;
}

void ParrotVideoWindow_set_title(ParrotVideoWindow *self, const char *title) {
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

void ParrotVideoWindow_set_size(ParrotVideoWindow *self, int width, int height) {
    XResizeWindow(self->display, self->window, width, height);
}

int ParrotVideoWindow_get_width(ParrotVideoWindow *self) {
    XWindowAttributes attrs;
    XGetWindowAttributes(self->display, self->window, &attrs);
    return attrs.width;
}

int ParrotVideoWindow_get_height(ParrotVideoWindow *self) {
    XWindowAttributes attrs;
    XGetWindowAttributes(self->display, self->window, &attrs);
    return attrs.height;
}

void ParrotVideoWindow_set_image(ParrotVideoWindow *self, const uint32_t *rgbx8888) {
    PARROT_FAIL_NULL(self);

    int width = ParrotVideoWindow_get_width(self);
    int height = ParrotVideoWindow_get_height(self);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t r = rgbx8888[y * width + x] & 0xFF;
            uint8_t g = (rgbx8888[y * width + x] >> 8) & 0xFF;
            uint8_t b = (rgbx8888[y * width + x] >> 16) & 0xFF;
            self->image_data[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
    XPutImage(self->display, self->back_buffer, self->gc, self->image, 0, 0, 0, 0, width, height);

    XdbeSwapInfo swap_info = {self->window, XdbeBackground};
    XdbeSwapBuffers(self->display, &swap_info, 1);
}
