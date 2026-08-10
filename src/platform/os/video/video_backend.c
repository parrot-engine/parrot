#include "src/video/video_backend.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "stb_ds.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef PARROT_DOUBLE_PRECISION
#define PARROT_GL_REAL GL_FLOAT
#else
#define PARROT_GL_REAL GL_DOUBLE
#endif

extern const char ParrotVideoBackend_vertex_shader[];
extern const char ParrotVideoBackend_fragment_shader[];

typedef struct {
    uint32_t key;

    Display *display;
    GLXPbuffer pbuffer;

    GLXContext context;

    int width;
    int height;

    uint32_t *framebuffer;
} ParrotVideoBackendViewport;

typedef struct {
    ParrotVideoBackendViewport *hm_viewports;
    uint32_t next_viewport_id;
} ParrotVideoBackend;

static ParrotVideoBackend *self = NULL;

void ParrotVideoBackend_init(void) {
    PARROT_FAIL_COND(ParrotVideoBackend_is_initialized());

    self = malloc(sizeof(ParrotVideoBackend));
    PARROT_FAIL_COND(!self);
    memset(self, 0, sizeof(ParrotVideoBackend));
}

void ParrotVideoBackend_shutdown(void) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    for (size_t i = 0; i < hmlen(self->hm_viewports); i++) {
        ParrotVideoBackend_delete_viewport((ParrotVideoBackendViewportHandle){
            .index = self->hm_viewports[i].key,
        });
    }

    hmfree(self->hm_viewports);

    free(self);
    self = NULL;
}

bool ParrotVideoBackend_is_initialized(void) {
    return self;
}

static ParrotVideoBackendViewport *ParrotVideoBackend_use_viewport(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = hmgetp_null(self->hm_viewports, handle.index);
    PARROT_FAIL_NULL(viewport);

    glXMakeContextCurrent(viewport->display, viewport->pbuffer, viewport->pbuffer, viewport->context);

    glViewport(0, 0, viewport->width, viewport->height);

    return viewport;
}

ParrotVideoBackendViewportHandle ParrotVideoBackend_create_viewport(int width, int height) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport viewport = {0};

    viewport.key = self->next_viewport_id++;

    viewport.width = width;
    viewport.height = height;

    viewport.display = XOpenDisplay(NULL);

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
    GLXFBConfig *configs = glXChooseFBConfig(viewport.display, XDefaultScreen(viewport.display), attribs, &count);
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

    viewport.pbuffer = glXCreatePbuffer(viewport.display, config, pb_attribs);
    viewport.context = glXCreateNewContext(viewport.display, config, GLX_RGBA_TYPE, NULL, True);

    glXMakeContextCurrent(viewport.display, viewport.pbuffer, viewport.pbuffer, viewport.context);

    viewport.framebuffer = calloc(width * height, sizeof(uint32_t));

    hmputs(self->hm_viewports, viewport);
    return (ParrotVideoBackendViewportHandle){
        .index = viewport.key,
    };
}

void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);

    glXDestroyPbuffer(viewport->display, viewport->pbuffer);
    glXDestroyContext(viewport->display, viewport->context);
    XCloseDisplay(viewport->display);

    free(viewport->framebuffer);

    hmdel(self->hm_viewports, handle.index);
}

const uint32_t *ParrotVideoBackend_get_viewport_pixels(ParrotVideoBackendViewportHandle handle) {
    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);
    glReadPixels(0, 0, viewport->width, viewport->height, GL_RGBA, GL_UNSIGNED_BYTE, viewport->framebuffer);
    return viewport->framebuffer;
}

void ParrotVideoBackend_clear_viewport(ParrotVideoBackendViewportHandle handle, ParrotColor clear_color) {
    ParrotVideoBackend_use_viewport(handle);

    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ParrotVideoBackend_draw_viewport_vertices(ParrotVideoBackendViewportHandle handle,
                                               ParrotGMatSet matrix_set,
                                               const ParrotVideoBackendVertex *vertices,
                                               size_t count) {
    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);
    (void)viewport;

    glMatrixMode(GL_MODELVIEW);
    ParrotMat modelview_matrix = ParrotMat_mul(matrix_set.model, matrix_set.view);
#ifndef PARROT_DOUBLE_PRECISION
    glLoadMatrixf(modelview_matrix.data[0]);
#else
    glLoadMatrixd(modelview_matrix.data[0]);
#endif

    glMatrixMode(GL_PROJECTION);

#ifndef PARROT_DOUBLE_PRECISION
    glLoadMatrixf(matrix_set.projection.data[0]);
#else
    glLoadMatrixd(matrix_set.projection.data[0]);
#endif

    glBegin(GL_TRIANGLES);
    {
        for (size_t i = 0; i < count; i++) {
            const ParrotVideoBackendVertex *vertex = &vertices[i];
            glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
            glColor4f(vertex->tint.r, vertex->tint.g, vertex->tint.b, vertex->tint.a);
            glVertex3f(vertex->position.x, vertex->position.y, vertex->position.z);
        }
    }
    glEnd();
}
