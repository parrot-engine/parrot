#include "parrot/drivers/video_driver.h"
#include "parrot/core/hash.h"
#include "parrot/core/math.h"
#include "parrot/drivers/gl_driver.h"
#include "parrot/video/video.h"
#include "stb_ds.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <string.h>

#define MAX_TEXTURE_CACHE_SIZE 16

typedef struct {
    GLuint handle;
    uint32_t crc32;
} ParrotVideoDriverViewportTextureCacheEntry;

struct ParrotVideoDriverViewport {
    ParrotGLDriverContext *context;

    int width;
    int height;

    uint32_t *framebuffer;

    ParrotVideoDriverViewportTextureCacheEntry texture_cache[MAX_TEXTURE_CACHE_SIZE];
};

struct {
    ParrotVideoDriverViewport **arr_viewports;
    uint32_t next_viewport_id;
} data;

static void driver_init(void) {
    memset(&data, 0, sizeof(data));
}

static void driver_shutdown(void) {
    for (size_t i = 0; i < arrlen(data.arr_viewports); i++) {
        Parrot_gl11_video_driver.delete_viewport(data.arr_viewports[i]);
    }

    arrfree(data.arr_viewports);
}

static ParrotVideoDriverViewport *driver_create_viewport(int width, int height) {
    ParrotVideoDriverViewport *self = PARROT_ALLOC(ParrotVideoDriverViewport);

    self->width = width;
    self->height = height;

    self->context = Parrot_gl_driver->create_context(ParrotGLDriver_GL_VERSION(1, 1), width, height);
    self->framebuffer = calloc(width * height, sizeof(uint32_t));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    arrpush(data.arr_viewports, self);
    return self;
}

static void driver_delete_viewport(ParrotVideoDriverViewport *self) {
    for (size_t i = 0; i < arrlen(data.arr_viewports); i++) {
        if (data.arr_viewports[i] == self) {
            arrdel(data.arr_viewports, i);
            break;
        }
    }

    free(self->framebuffer);

    Parrot_gl_driver->delete_context(self->context);

    free(self);
}

static void use_viewport(ParrotVideoDriverViewport *self) {
    Parrot_gl_driver->use_context(self->context);

    glViewport(0, 0, self->width, self->height);
}

static const uint32_t *driver_get_viewport_pixels(ParrotVideoDriverViewport *self) {
    use_viewport(self);
    glReadPixels(0, 0, self->width, self->height, GL_RGBA, GL_UNSIGNED_BYTE, self->framebuffer);
    return self->framebuffer;
}

static void driver_set_viewport_texture(
    ParrotVideoDriverViewport *self, int width, int height, const uint32_t *rgba8888, bool nearest_filter) {
    use_viewport(self);

    size_t index = MAX_TEXTURE_CACHE_SIZE;
    uint32_t crc32 = Parrot_crc32(rgba8888, width * height * sizeof(uint32_t));

    for (size_t i = 0; i < MAX_TEXTURE_CACHE_SIZE; i++) {
        if (self->texture_cache[i].crc32 == crc32) {
            index = i;
            goto apply_texture;
        }
    }

    index--;
    if (self->texture_cache[index].handle > 0) {
        glDeleteTextures(1, &self->texture_cache[index].handle);
    }

    self->texture_cache[index] = (ParrotVideoDriverViewportTextureCacheEntry){0};
    self->texture_cache[index].crc32 = crc32;

    glGenTextures(1, &self->texture_cache[index].handle);
    glBindTexture(GL_TEXTURE_2D, self->texture_cache[index].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba8888);

apply_texture: {
    ParrotVideoDriverViewportTextureCacheEntry entry = self->texture_cache[index];
    glBindTexture(GL_TEXTURE_2D, entry.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, !nearest_filter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, !nearest_filter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glEnable(GL_TEXTURE_2D);

    for (size_t i = MAX_TEXTURE_CACHE_SIZE - 1; i > 0; i--) {
        self->texture_cache[i] = self->texture_cache[i - 1];
    }

    self->texture_cache[0] = entry;
}
}

static void driver_clear_viewport_texture(ParrotVideoDriverViewport *self) {
    use_viewport(self);

    glDisable(GL_TEXTURE_2D);
}

static void driver_clear_viewport(ParrotVideoDriverViewport *self, ParrotColor clear_color) {
    use_viewport(self);

    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void driver_draw_viewport_vertices(ParrotVideoDriverViewport *self,
                                          ParrotGMatSet matrix_set,
                                          const ParrotVideoVertex *vertices,
                                          size_t count) {
    use_viewport(self);

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
            const ParrotVideoVertex *vertex = &vertices[i];
            glColor4f(vertex->tint.r, vertex->tint.g, vertex->tint.b, vertex->tint.a);
            glTexCoord2f(vertex->uv.x, vertex->uv.y);
            glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
            glVertex3f(vertex->position.x, vertex->position.y, vertex->position.z);
        }
    }
    glEnd();
}

const ParrotVideoDriver Parrot_gl11_video_driver = {
    .init = driver_init,
    .shutdown = driver_shutdown,

    .create_viewport = driver_create_viewport,
    .delete_viewport = driver_delete_viewport,

    .get_viewport_pixels = driver_get_viewport_pixels,

    .set_viewport_texture = driver_set_viewport_texture,
    .clear_viewport_texture = driver_clear_viewport_texture,

    .clear_viewport = driver_clear_viewport,

    .draw_viewport_vertices = driver_draw_viewport_vertices,
};
