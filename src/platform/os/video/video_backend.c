#include "src/video/video_backend.h"
#include "parrot/core/hash.h"
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

#define MAX_TEXTURE_CACHE_SIZE 16

typedef struct {
    GLuint handle;
    uint32_t crc32;
} ParrotVideoBackendViewportTextureCacheEntry;

typedef struct {
    uint32_t key;

    Display *display;
    GLXPbuffer pbuffer;

    GLXContext context;

    int width;
    int height;

    uint32_t *framebuffer;

    ParrotVideoBackendViewportTextureCacheEntry texture_cache[MAX_TEXTURE_CACHE_SIZE];
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

static ParrotVideoBackendViewport *use_viewport(ParrotVideoBackendViewportHandle handle) {
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

    viewport.framebuffer = calloc(width * height, sizeof(uint32_t));

    glXMakeContextCurrent(viewport.display, viewport.pbuffer, viewport.pbuffer, viewport.context);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    hmputs(self->hm_viewports, viewport);
    return (ParrotVideoBackendViewportHandle){
        .index = viewport.key,
    };
}

void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = use_viewport(handle);

    free(viewport->framebuffer);

    XCloseDisplay(viewport->display);

    hmdel(self->hm_viewports, handle.index);
}

void ParrotVideoBackend_set_viewport_texture(
    ParrotVideoBackendViewportHandle handle, int width, int height, const uint32_t *rgba8888, bool nearest_filter) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = use_viewport(handle);

    size_t index = MAX_TEXTURE_CACHE_SIZE;
    uint32_t crc32 = Parrot_crc32(rgba8888, width * height * sizeof(uint32_t));

    for (size_t i = 0; i < MAX_TEXTURE_CACHE_SIZE; i++) {
        if (viewport->texture_cache[i].crc32 == crc32) {
            index = i;
            goto apply_texture;
        }
    }

    index--;
    if (viewport->texture_cache[index].handle > 0) {
        glDeleteTextures(1, &viewport->texture_cache[index].handle);
    }

    viewport->texture_cache[index] = (ParrotVideoBackendViewportTextureCacheEntry){0};
    viewport->texture_cache[index].crc32 = crc32;

    glGenTextures(1, &viewport->texture_cache[index].handle);
    glBindTexture(GL_TEXTURE_2D, viewport->texture_cache[index].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba8888);

apply_texture: {
    ParrotVideoBackendViewportTextureCacheEntry entry = viewport->texture_cache[index];
    glBindTexture(GL_TEXTURE_2D, entry.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, !nearest_filter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, !nearest_filter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glEnable(GL_TEXTURE_2D);

    for (size_t i = MAX_TEXTURE_CACHE_SIZE - 1; i > 0; i--) {
        viewport->texture_cache[i] = viewport->texture_cache[i - 1];
    }

    viewport->texture_cache[0] = entry;
}
}

void ParrotVideoBackend_clear_viewport_texture(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    use_viewport(handle);
    glDisable(GL_TEXTURE_2D);
}

const uint32_t *ParrotVideoBackend_get_viewport_pixels(ParrotVideoBackendViewportHandle handle) {
    ParrotVideoBackendViewport *viewport = use_viewport(handle);
    glReadPixels(0, 0, viewport->width, viewport->height, GL_RGBA, GL_UNSIGNED_BYTE, viewport->framebuffer);
    return viewport->framebuffer;
}

void ParrotVideoBackend_clear_viewport(ParrotVideoBackendViewportHandle handle, ParrotColor clear_color) {
    use_viewport(handle);

    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ParrotVideoBackend_draw_viewport_vertices(ParrotVideoBackendViewportHandle handle,
                                               ParrotGMatSet matrix_set,
                                               const ParrotVideoBackendVertex *vertices,
                                               size_t count) {
    ParrotVideoBackendViewport *viewport = use_viewport(handle);
    (void)viewport;

    glMatrixMode(GL_MODELVIEW);
    ParrotMat modelview_matrix = ParrotMat_mul(matrix_set.view, matrix_set.model);
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
            glColor4f(vertex->tint.r, vertex->tint.g, vertex->tint.b, vertex->tint.a);
            glTexCoord2f(vertex->uv.x, vertex->uv.y);
            glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
            glVertex3f(vertex->position.x, vertex->position.y, vertex->position.z);
        }
    }
    glEnd();
}
