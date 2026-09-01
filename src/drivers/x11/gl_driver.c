#include "parrot/drivers/gl_driver.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <stdint.h>
#include <string.h>

typedef struct Driver Driver;

struct ParrotGLDriverContext {
    Driver *driver;
    ParrotScope *scope;

    GLXPbuffer pbuffer;

    GLXContext context;
};

struct Driver {
    ParrotGLDriver base;

    ParrotScope *scope;

    Display *display;
};

static void scope_XFree_wrapper(void *ctx) {
    XFree(ctx);
}

static void scope_context_destroy_pbuffer_wrapper(void *ctx) {
    ParrotGLDriverContext *self = ctx;

    glXDestroyPbuffer(self->driver->display, self->pbuffer);
}

static void scope_context_destroy_context_wrapper(void *ctx) {
    ParrotGLDriverContext *self = ctx;

    glXDestroyContext(self->driver->display, self->context);
}

static ParrotGLDriverContext *driver_create_context(ParrotGLDriver *base, int major, int minor, int width, int height) {
    ParrotGLDriverContext *self = PARROT_ALLOC(ParrotGLDriverContext);

    self->driver = (Driver *)base;

    self->scope = ParrotScope_new(self->driver->scope);
    ParrotScope_push_free(self->scope, self);

    int count = 0;
    int attribs[] = {
        GLX_RENDER_TYPE,
        GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE,
        GLX_PBUFFER_BIT,

        GLX_RED_SIZE,
        8,
        GLX_GREEN_SIZE,
        8,
        GLX_BLUE_SIZE,
        8,
        GLX_DEPTH_SIZE,
        24,
        None,
    };

    GLXFBConfig *configs =
        glXChooseFBConfig(self->driver->display, XDefaultScreen(self->driver->display), attribs, &count);
    ParrotScope_push(self->scope, scope_XFree_wrapper, configs);
    if (count == 0) {
        ParrotScope_delete(self->scope);
        return NULL;
    }
    GLXFBConfig config = configs[0];

    int pb_attribs[] = {
        GLX_PBUFFER_WIDTH,
        width,
        GLX_PBUFFER_HEIGHT,
        height,
        None,
    };

    self->pbuffer = glXCreatePbuffer(self->driver->display, config, pb_attribs);
    ParrotScope_push(self->scope, scope_context_destroy_pbuffer_wrapper, self);
    self->context = glXCreateNewContext(self->driver->display, config, GLX_RGBA_TYPE, NULL, True);
    ParrotScope_push(self->scope, scope_context_destroy_context_wrapper, self);

    glXMakeContextCurrent(self->driver->display, self->pbuffer, self->pbuffer, self->context);

    int actual_major, actual_minor;
    glGetIntegerv(GL_MAJOR_VERSION, &actual_major);
    glGetIntegerv(GL_MINOR_VERSION, &actual_minor);

    if (actual_major < major || (actual_major == major && actual_minor < minor)) {
        ParrotScope_delete(self->scope);
        return NULL;
    }

    return self;
}

static void driver_delete_context(ParrotGLDriverContext *self) {
    ParrotScope_delete(self->scope);
}

static void driver_use_context(ParrotGLDriverContext *self) {
    glXMakeContextCurrent(self->driver->display, self->pbuffer, self->pbuffer, self->context);
}

static void scope_XCloseDisplay_wrapper(void *ctx) {
    XCloseDisplay(ctx);
}

ParrotGLDriver *Parrot_x11_gl_driver_new(void) {
    Driver *self = PARROT_ALLOC(Driver);

    self->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(self->scope, self);

    self->base.create_context = driver_create_context;
    self->base.delete_context = driver_delete_context;
    self->base.use_context = driver_use_context;

    self->display = XOpenDisplay(NULL);
    if (!self->display) {
        ParrotScope_delete(self->scope);
        return NULL;
    }
    ParrotScope_push(self->scope, scope_XCloseDisplay_wrapper, self->display);

    return &self->base;
}

void Parrot_x11_gl_driver_delete(ParrotGLDriver *base) {
    ParrotScope_delete(((Driver *)base)->scope);
}
