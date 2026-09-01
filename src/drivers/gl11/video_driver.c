#include "parrot/drivers/video_driver.h"
#include "parrot/core/hash.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/drivers/gl_driver.h"
#include "parrot/video/video.h"
#include "stb_ds.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <string.h>

#define MAX_TEXTURE_CACHE_SIZE 256

typedef struct Driver Driver;

typedef struct {
    GLuint handle;
    uint32_t crc32;
} ViewportTextureCacheEntry;

struct ParrotVideoDriverViewport {
    Driver *driver;
    ParrotScope *scope;

    ParrotGLDriverContext *gl_context;

    int width;
    int height;

    uint32_t *framebuffer;

    ViewportTextureCacheEntry texture_cache[MAX_TEXTURE_CACHE_SIZE];
};

struct Driver {
    ParrotVideoDriver base;
    ParrotScope *scope;

    ParrotGLDriver *gl_driver;
};

static void driver_viewport_scope_delete_context_wrapper(void *ctx) {
    ParrotVideoDriverViewport *self = ctx;

    self->driver->gl_driver->delete_context(self->gl_context);
}

static ParrotVideoDriverViewport *driver_create_viewport(ParrotVideoDriver *base, int width, int height) {
    ParrotVideoDriverViewport *self = PARROT_ALLOC(ParrotVideoDriverViewport);

    self->driver = (Driver *)base;
    self->scope = ParrotScope_new(self->driver->scope);
    ParrotScope_push_free(self->scope, self);

    self->width = width;
    self->height = height;

    self->framebuffer = calloc(width * height, sizeof(uint32_t));
    ParrotScope_push_free(self->scope, self->framebuffer);

    self->gl_context = self->driver->gl_driver->create_context(self->driver->gl_driver, 1, 1, width, height);
    ParrotScope_push(self->scope, driver_viewport_scope_delete_context_wrapper, self);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return self;
}

static void driver_delete_viewport(ParrotVideoDriverViewport *self) {
    ParrotScope_delete(self->scope);
}

static void use_viewport(ParrotVideoDriverViewport *self) {
    self->driver->gl_driver->use_context(self->gl_context);

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

    self->texture_cache[index] = (ViewportTextureCacheEntry){0};
    self->texture_cache[index].crc32 = crc32;

    glGenTextures(1, &self->texture_cache[index].handle);
    glBindTexture(GL_TEXTURE_2D, self->texture_cache[index].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba8888);

apply_texture: {
    ViewportTextureCacheEntry entry = self->texture_cache[index];
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

ParrotVideoDriver *Parrot_gl11_video_driver_new(ParrotGLDriver *gl_driver) {
    Driver *self = PARROT_ALLOC(Driver);

    self->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(self->scope, self);

    self->base.create_viewport = driver_create_viewport;
    self->base.delete_viewport = driver_delete_viewport;

    self->base.get_viewport_pixels = driver_get_viewport_pixels;

    self->base.set_viewport_texture = driver_set_viewport_texture;
    self->base.clear_viewport_texture = driver_clear_viewport_texture;

    self->base.clear_viewport = driver_clear_viewport;

    self->base.draw_viewport_vertices = driver_draw_viewport_vertices;

    self->gl_driver = gl_driver;

    return &self->base;
}

void Parrot_gl11_video_driver_delete(ParrotVideoDriver *base) {
    ParrotScope_delete(((Driver *)base)->scope);
}
