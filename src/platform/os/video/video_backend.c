#include "src/video/video_backend.h"
#include "GL/gl3w.h"
#include "GL/glcorearb.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "stb_ds.h"
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

    GLuint frame_buffer;
    GLuint depth_render_buffer;
    GLuint texture;

    int width;
    int height;

    GLuint shader;
    GLint shader_matrix_loc;
    GLint shader_texture_loc;
    GLint shader_color_loc;

    GLuint vbo;

    GLuint white_texture;
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

    PARROT_FAIL_COND(!gl3wInit());
    PARROT_FAIL_COND_MSG(gl3wIsSupported(3, 3), "OpenGL 3.3 or later is required");
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

    glBindFramebuffer(GL_FRAMEBUFFER, viewport->frame_buffer);
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        const char *src = NULL;

        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        src = ParrotVideoBackend_vertex_shader;
        glShaderSource(vertex_shader, 1, &src, NULL);
        glCompileShader(vertex_shader);

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        src = ParrotVideoBackend_fragment_shader;
        glShaderSource(fragment_shader, 1, &src, NULL);
        glCompileShader(fragment_shader);

        viewport.shader = glCreateProgram();
        glAttachShader(viewport.shader, vertex_shader);
        glAttachShader(viewport.shader, fragment_shader);
        glLinkProgram(viewport.shader);

        viewport.shader_matrix_loc = glGetUniformLocation(viewport.shader, "u_matrix");
        viewport.shader_texture_loc = glGetUniformLocation(viewport.shader, "u_texture");
        viewport.shader_color_loc = glGetUniformLocation(viewport.shader, "u_color");

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    glGenBuffers(1, &viewport.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, viewport.vbo);

    {
#define ATTRIBUTE(index, size, type, field)                                                                             \
    do {                                                                                                                \
        glVertexAttribPointer(index,                                                                                    \
                              size,                                                                                     \
                              type,                                                                                     \
                              GL_FALSE,                                                                                 \
                              sizeof(ParrotVideoBackendVertex),                                                         \
                              (void *)offsetof(ParrotVideoBackendVertex, field));                                       \
        glEnableVertexAttribArray(index);                                                                               \
    } while (0)

        ATTRIBUTE(0, 3, PARROT_GL_REAL, position);
        ATTRIBUTE(1, 3, PARROT_GL_REAL, normal);
        ATTRIBUTE(2, 2, PARROT_GL_REAL, uv);

#undef ATTRIBUTE
    }

    {
        glGenTextures(1, &viewport.white_texture);
        glBindTexture(GL_TEXTURE_2D, viewport.white_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        uint32_t white = 0xFFFFFFFF;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    hmputs(self->hm_viewports, viewport);
    return (ParrotVideoBackendViewportHandle){
        .index = viewport.key,
    };
}

void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);

    glDeleteTextures(1, &viewport->white_texture);

    glDeleteBuffers(1, &viewport->vbo);

    glDeleteProgram(viewport->shader);

    glDeleteFramebuffers(1, &viewport->frame_buffer);
    glDeleteTextures(1, &viewport->texture);
    glDeleteRenderbuffers(1, &viewport->depth_render_buffer);

    hmdel(self->hm_viewports, handle.index);
}

void ParrotVideoBackend_read_viewport(ParrotVideoBackendViewportHandle handle, uint32_t *bgra) {
    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);

    glReadPixels(0, 0, viewport->width, viewport->height, GL_BGRA, GL_UNSIGNED_BYTE, bgra);
}

void ParrotVideoBackend_clear_viewport(ParrotVideoBackendViewportHandle handle, ParrotVec3 clear_color) {
    ParrotVideoBackend_use_viewport(handle);

    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ParrotVideoBackend_draw_viewport_vertices(ParrotVideoBackendViewportHandle handle,
                                               ParrotVec4 color,
                                               ParrotMat matrix,
                                               const ParrotVideoBackendVertex *vertices,
                                               size_t count) {
    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);

    glBindTexture(GL_TEXTURE_2D, viewport->white_texture);

    glBindBuffer(GL_ARRAY_BUFFER, viewport->vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParrotVideoBackendVertex), vertices, GL_DYNAMIC_DRAW);

    float gl_matrix[4][4];
    ParrotReal_to_float_array(matrix.data[0], gl_matrix[0], 4 * 4);

    glUseProgram(viewport->shader);
    glUniformMatrix4fv(viewport->shader_matrix_loc, 1, GL_FALSE, gl_matrix[0]);
    glUniform1i(viewport->shader_texture_loc, 0);
    glUniform4f(viewport->shader_color_loc, color.x, color.y, color.z, color.w);

    glDrawArrays(GL_TRIANGLES, 0, count);
}