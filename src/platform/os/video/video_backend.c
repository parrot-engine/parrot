#include "src/video/video_backend.h"
#include "GL/gl3w.h"
#include "GL/glcorearb.h"
#include "parrot/core/util.h"
#include "stb_ds.h"
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t key;

    Display *display;
    GLXPbuffer pbuffer;

    GLXContext context;

    GLuint frame_buffer;
    GLuint depth_render_buffer;
    GLuint texture;

    int width;
    int height;
} ParrotVideoBackendViewport;

typedef struct {
    ParrotVideoBackendViewport *hm_viewports;
    uint32_t next_viewport_id;
} ParrotVideoBackend;

static ParrotVideoBackend *self = NULL;

void ParrotVideoBackend_init(void) {
    PARROT_FAIL_COND(ParrotVideoBackend_is_initialized());

    self = malloc(sizeof(ParrotVideoBackend));
    memset(self, 0, sizeof(ParrotVideoBackend));

    PARROT_FAIL_COND(!gl3wInit());
    PARROT_FAIL_COND_MSG(gl3wIsSupported(3, 3), "OpenGL 3.3 or later is required");
}

void ParrotVideoBackend_shutdown(void) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    hmfree(self->hm_viewports);

    free(self);
    self = NULL;
}

bool ParrotVideoBackend_is_initialized(void) {
    return self;
}

ParrotVideoBackendViewportHandle ParrotVideoBackend_create_viewport(int width, int height) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport viewport = {0};
    viewport.key = self->next_viewport_id++;

    viewport.display = XOpenDisplay(NULL);

    int count = 0;
    int attribs[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT, GLX_DEPTH_SIZE, 24, None};
    GLXFBConfig *configs = glXChooseFBConfig(viewport.display, XDefaultScreen(viewport.display), attribs, &count);

    PARROT_FAIL_COND_MSG(count == 0, "Your platform does not support the requirements for display");

    int pb_attribs[] = {GLX_PBUFFER_WIDTH, 1, GLX_PBUFFER_HEIGHT, 1, None};
    viewport.pbuffer = glXCreatePbuffer(viewport.display, configs[0], pb_attribs);

    viewport.context = glXCreateNewContext(viewport.display, configs[0], GLX_RGBA_TYPE, NULL, True);

    glXMakeContextCurrent(viewport.display, viewport.pbuffer, viewport.pbuffer, viewport.context);

    glGenFramebuffers(1, &viewport.frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, viewport.frame_buffer);

    glGenTextures(1, &viewport.texture);
    glBindTexture(GL_TEXTURE_2D, viewport.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &viewport.depth_render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, viewport.depth_render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, viewport.depth_render_buffer);

    PARROT_FAIL_COND(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewport.texture, 0);

    glClearColor(0, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    viewport.width = width;
    viewport.height = height;

    hmputs(self->hm_viewports, viewport);
    return (ParrotVideoBackendViewportHandle){
        .index = viewport.key,
    };
}

void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = hmgetp_null(self->hm_viewports, handle.index);
    PARROT_FAIL_NULL(viewport);

    glXMakeContextCurrent(viewport->display, viewport->pbuffer, viewport->pbuffer, viewport->context);

    glDeleteFramebuffers(1, &viewport->frame_buffer);
    glDeleteTextures(1, &viewport->texture);
    glDeleteRenderbuffers(1, &viewport->depth_render_buffer);

    hmdel(self->hm_viewports, handle.index);
}

static void ParrotVideoBackend_use_gl_context(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = hmgetp_null(self->hm_viewports, handle.index);
    PARROT_FAIL_NULL(viewport);

    glXMakeContextCurrent(viewport->display, viewport->pbuffer, viewport->pbuffer, viewport->context);
}

void ParrotVideoBackend_read_viewport(ParrotVideoBackendViewportHandle handle, uint32_t *bgra) {
    ParrotVideoBackendViewport *viewport = hmgetp_null(self->hm_viewports, handle.index);
    ParrotVideoBackend_use_gl_context(handle);

    PARROT_FAIL_NULL(viewport);

    glReadPixels(0, 0, viewport->width, viewport->height, GL_BGRA, GL_UNSIGNED_BYTE, bgra);
}