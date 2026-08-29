#include "parrot/drivers/gl_driver.h"
#include "parrot/core/util.h"
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <stdint.h>
#include <string.h>

struct ParrotGLDriverContext {
    GLXPbuffer pbuffer;

    GLXContext context;
};

Display *display = NULL;

static void driver_init(void) {
    display = XOpenDisplay(NULL);
    PARROT_FAIL_NULL_MSG(display, "Failed to open X11 display");
}

static void driver_shutdown(void) {
    XCloseDisplay(display);
}

static ParrotGLDriverContext *driver_create_context(uint8_t version, int width, int height) {
    ParrotGLDriverContext *self = malloc(sizeof(ParrotGLDriverContext));
    memset(self, 0, sizeof(*self));

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
    GLXFBConfig *configs = glXChooseFBConfig(display, XDefaultScreen(display), attribs, &count);
    if (count == 0) {
        PARROT_FAIL_MSG("Your platform does not support the requirements for display");
        XFree(configs);
    }
    GLXFBConfig config = configs[0];
    XFree(configs);

    int pb_attribs[] = {
        GLX_PBUFFER_WIDTH,
        width,
        GLX_PBUFFER_HEIGHT,
        height,
        None,
    };

    self->pbuffer = glXCreatePbuffer(display, config, pb_attribs);
    self->context = glXCreateNewContext(display, config, GLX_RGBA_TYPE, NULL, True);

    glXMakeContextCurrent(display, self->pbuffer, self->pbuffer, self->context);

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    int target_major = version >> 4;
    int target_minor = version & 0xF;
    PARROT_FAIL_COND_FMT(major < target_major || (major == target_major && minor < target_minor),
                         "Unsupported OpenGL version %d.%d",
                         target_major,
                         target_minor);

    return self;
}

static void driver_delete_context(ParrotGLDriverContext *self) {
    glXMakeContextCurrent(display, None, None, NULL);
    glXDestroyPbuffer(display, self->pbuffer);
    glXDestroyContext(display, self->context);

    free(self);
}

static void driver_use_context(ParrotGLDriverContext *self) {
    glXMakeContextCurrent(display, self->pbuffer, self->pbuffer, self->context);
}

const ParrotGLDriver Parrot_x11_gl_driver = {
    .init = driver_init,
    .shutdown = driver_shutdown,

    .create_context = driver_create_context,
    .delete_context = driver_delete_context,
    .use_context = driver_use_context,
};
