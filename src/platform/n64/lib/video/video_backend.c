#include "src/video/video_backend.h"
#include "parrot/config.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "parrot/platform/n64/rdp.h"
#include "parrot/platform/n64/rsp.h"
#include "parrot/platform/n64/util.h"
#include "stb_ds.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FRAMEBUFFER_COUNT 2

extern uint8_t ParrotVideoBackend_rsp_ucode[];
extern uint32_t ParrotVideoBackend_rsp_ucode_size;

typedef struct {
    uint32_t key;

    int width;

    bool framebuffer_dirty;

    uint32_t *heap_framebuffer[FRAMEBUFFER_COUNT];
    uint32_t *heap_zbuffer;

    uint32_t *framebuffer[FRAMEBUFFER_COUNT];
    uint32_t *zbuffer;

    size_t framebuffer_index;
} ParrotVideoBackendViewport;

typedef struct {
    ParrotVideoBackendViewport *hm_viewports;
    uint32_t next_viewport_id;

    bool auto_rsp_shutdown;
} ParrotVideoBackend;

static ParrotVideoBackend *self = NULL;

static int get_height(int width) {
    return width / (4.0 / 3.0);
}

static ParrotRDPCommand *ParrotVideoBackendViewport_new_dl(ParrotVideoBackendViewport *self) {
    ParrotRDPCommand *arr_dl = NULL;

    arrsetlen(arr_dl, 0);

    arrpush(arr_dl, ParrotRDPCommand_pipe_sync());
    arrpush(arr_dl,
            ParrotRDPCommand_set_color_image(
                ParrotRDPCommand_FORMAT_RGBA,
                ParrotRDPCommand_SIZE_32BPP,
                self->width - 1,
                PARROT_N64_PHYSICAL_ADDRESS(self->framebuffer[self->framebuffer_index % FRAMEBUFFER_COUNT])));
    arrpush(arr_dl, ParrotRDPCommand_set_depth_image(PARROT_N64_PHYSICAL_ADDRESS(self->zbuffer)));
    arrpush(arr_dl,
            ParrotRDPCommand_set_scissor(0, 0, false, false, (self->width - 1) * 4, (get_height(self->width) - 1) * 4));

    return arr_dl;
}

static void ParrotVideoBackendViewport_finish_dl(ParrotVideoBackendViewport *self, ParrotRDPCommand *arr_dl) {
    (void)self;

    arrpush(arr_dl, ParrotRDPCommand_sync_full());

    ParrotRDP_send_dl(arr_dl, arrlen(arr_dl));

    arrfree(arr_dl);
}

void ParrotVideoBackend_init(void) {
    PARROT_FAIL_COND(ParrotVideoBackend_is_initialized());

    bool auto_rsp_shutdown = false;
    if (!ParrotRSP_is_initialized()) {
        ParrotRSP_init();
        auto_rsp_shutdown = true;
    }

    self = malloc(sizeof(ParrotVideoBackend));
    PARROT_FAIL_NULL(self);
    memset(self, 0, sizeof(ParrotVideoBackend));

    self->auto_rsp_shutdown = auto_rsp_shutdown;
}

void ParrotVideoBackend_shutdown(void) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    for (size_t i = 0; i < hmlen(self->hm_viewports); i++) {
        ParrotVideoBackend_delete_viewport((ParrotVideoBackendViewportHandle){
            .index = self->hm_viewports[i].key,
        });
    }

    hmfree(self->hm_viewports);

    if (self->auto_rsp_shutdown) {
        ParrotRSP_shutdown();
    }

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

    return viewport;
}

ParrotVideoBackendViewportHandle ParrotVideoBackend_create_viewport(int width, int height) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport viewport = {0};

    viewport.key = self->next_viewport_id++;

    viewport.width = PARROT_ALIGN_UP(width, 64);

    for (size_t i = 0; i < FRAMEBUFFER_COUNT; i++) {
        viewport.heap_framebuffer[i] = calloc(width * get_height(width), sizeof(uint32_t));
        viewport.framebuffer[i] = PARROT_N64_UNCACHED_ADDRESS(viewport.heap_framebuffer[i]);
    }

    viewport.heap_zbuffer = calloc(width * get_height(width), sizeof(uint16_t));
    viewport.zbuffer = PARROT_N64_UNCACHED_ADDRESS(viewport.heap_zbuffer);

    hmputs(self->hm_viewports, viewport);
    return (ParrotVideoBackendViewportHandle){
        .index = viewport.key,
    };
}

void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle) {
    PARROT_FAIL_COND(!ParrotVideoBackend_is_initialized());

    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);

    free(viewport->heap_zbuffer);
    for (size_t i = 0; i < FRAMEBUFFER_COUNT; i++) {
        free(viewport->heap_framebuffer[i]);
    }

    hmdel(self->hm_viewports, handle.index);
}

const uint32_t *ParrotVideoBackend_get_viewport_pixels(ParrotVideoBackendViewportHandle handle) {
    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);
    return viewport->framebuffer[viewport->framebuffer_index++ % FRAMEBUFFER_COUNT];
}

void ParrotVideoBackend_clear_viewport(ParrotVideoBackendViewportHandle handle, ParrotColor clear_color) {
    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);
    ParrotRDPCommand *arr_dl = ParrotVideoBackendViewport_new_dl(viewport);

    clear_color.a = 1.0;

    arrpush(arr_dl, ParrotRDPCommand_set_other_modes(ParrotRDPCommand_OM_CYCLE_TYPE(ParrotRDPCommand_CYCLE_TYPE_FILL)));
    arrpush(arr_dl,
            ParrotRDPCommand_set_fill_color(ParrotRDPCommand_set_fill_color(ParrotColor_to_rgba8888(clear_color))));
    arrpush(arr_dl, ParrotRDPCommand_fill_rect((viewport->width - 1) * 4, (get_height(viewport->width) - 1) * 4, 0, 0));

    ParrotVideoBackendViewport_finish_dl(viewport, arr_dl);
}

void ParrotVideoBackend_draw_viewport_vertices(ParrotVideoBackendViewportHandle handle,
                                               ParrotColor color,
                                               ParrotGMatSet matrix_set,
                                               const ParrotVideoBackendVertex *vertices,
                                               size_t count) {
    ParrotMat matrix = ParrotGMatSet_combine(&matrix_set);

    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);
    ParrotRDPCommand *arr_dl = ParrotVideoBackendViewport_new_dl(viewport);

    arrpush(arr_dl, ParrotRDPCommand_set_fill_color(ParrotColor_to_rgba8888(color)));

    size_t triangle_count = count / 3;
    for (size_t i = 0; i < triangle_count; i++) {
        ParrotVideoBackendVertex a = vertices[i * 3 + 0];
        ParrotVideoBackendVertex b = vertices[i * 3 + 1];
        ParrotVideoBackendVertex c = vertices[i * 3 + 2];

        ParrotVideoBackendVertex *v[3] = {&a, &b, &c};
        for (size_t i = 0; i < 3; i++) {
            v[i]->position = ParrotMat_transform3(matrix, v[i]->position);
            v[i]->position = ParrotVec3_add(v[i]->position, (ParrotVec3){1, 1, 1});
            v[i]->position = ParrotVec3_scale(v[i]->position, 1.0 / 2);
            v[i]->position.y = v[i]->position.y * 0.75;
            v[i]->position.x *= viewport->width - 1;
            v[i]->position.y *= get_height(viewport->width) - 1;
        }

        if (v[0]->position.y > v[1]->position.y) {
            ParrotVideoBackendVertex *tmp = v[0];
            v[0] = v[1];
            v[1] = tmp;
        }
        if (v[1]->position.y > v[2]->position.y) {
            ParrotVideoBackendVertex *tmp = v[1];
            v[1] = v[2];
            v[2] = tmp;
        }
        if (v[0]->position.y > v[1]->position.y) {
            ParrotVideoBackendVertex *tmp = v[0];
            v[0] = v[1];
            v[1] = tmp;
        }

        int32_t yh = (int32_t)(v[0]->position.y * 4.0f);
        int32_t ym = (int32_t)(v[1]->position.y * 4.0f);
        int32_t yl = (int32_t)(v[2]->position.y * 4.0f);

        float dx1 = v[1]->position.x - v[0]->position.x;
        float dy1 = v[1]->position.y - v[0]->position.y;
        float dx2 = v[2]->position.x - v[0]->position.x;
        float dy2 = v[2]->position.y - v[0]->position.y;
        bool lmajor = (dx1 * dy2 - dx2 * dy1) > 0.0f;

        int level = 0;
        int tile = 0;

        float x0 = v[0]->position.x;
        float y0 = v[0]->position.y;
        float x1 = v[1]->position.x;
        float y1 = v[1]->position.y;
        float x2 = v[2]->position.x;
        float y2 = v[2]->position.y;

        float dy_h = y2 - y0;
        float dy_m = y1 - y0;
        float dy_l = y2 - y1;

        float dxhdy = (dy_h != 0.0f) ? (x2 - x0) / dy_h : 0.0f;
        float dxmdy = (dy_m != 0.0f) ? (x1 - x0) / dy_m : 0.0f;
        float dxldy = (dy_l != 0.0f) ? (x2 - x1) / dy_l : 0.0f;

        int32_t xh_fixed = (int32_t)(x0 * 65536.0f);
        int32_t xm_fixed = (int32_t)(x0 * 65536.0f);
        int32_t xl_fixed = (int32_t)(x1 * 65536.0f);
        int32_t dxhdy_fixed = (int32_t)(dxhdy * 65536.0f);
        int32_t dxmdy_fixed = (int32_t)(dxmdy * 65536.0f);
        int32_t dxldy_fixed = (int32_t)(dxldy * 65536.0f);

        arrpush(arr_dl, ParrotRDPCommand_triangle(false, false, false, lmajor, level, tile, yl, ym, yh));
        arrpush(
            arr_dl,
            ParrotRDPCommand_triangle_xl(xl_fixed >> 16, xl_fixed & 0xFFFF, dxldy_fixed >> 16, dxldy_fixed & 0xFFFF));
        arrpush(
            arr_dl,
            ParrotRDPCommand_triangle_xh(xh_fixed >> 16, xh_fixed & 0xFFFF, dxhdy_fixed >> 16, dxhdy_fixed & 0xFFFF));
        arrpush(
            arr_dl,
            ParrotRDPCommand_triangle_xm(xm_fixed >> 16, xm_fixed & 0xFFFF, dxmdy_fixed >> 16, dxmdy_fixed & 0xFFFF));
    }

    ParrotVideoBackendViewport_finish_dl(viewport, arr_dl);
}

/* TODO: RSP implementation

#define ParrotVideoBackend_VERTEX_BATCHES 3
#define ParrotVideoBackend_VERTEX_COUNT (ParrotVideoBackend_VERTEX_BATCHES * 8)

PARROT_PACK_BEGIN()
typedef struct {
    // accessed like matrix[x][y] or column-major
    ParrotFixed32s matrix[4][4];

    ParrotFixed16i vertex_x[ParrotVideoBackend_VERTEX_COUNT];
    ParrotFixed16i vertex_y[ParrotVideoBackend_VERTEX_COUNT];
    ParrotFixed16i vertex_z[ParrotVideoBackend_VERTEX_COUNT];
    ParrotFixed16f vertex_u[ParrotVideoBackend_VERTEX_COUNT];
    ParrotFixed16f vertex_v[ParrotVideoBackend_VERTEX_COUNT];

    uint32_t color_rgba8888;
} ParrotVideoBackendRSPData;
PARROT_PACK_END()

void ParrotVideoBackend_draw_viewport_vertices(ParrotVideoBackendViewportHandle handle,
                                               ParrotVec4 color,
                                               ParrotGMatSet matrix_set,
                                               const ParrotVideoBackendVertex *vertices,
                                               size_t count) {
    ParrotVideoBackendViewport *viewport = ParrotVideoBackend_use_viewport(handle);

    ParrotRSPTaskHandle *arr_tasks = NULL;

    ParrotVideoBackendRSPData data = {0};

    ParrotMat matrix = ParrotMat_mul(ParrotMat_mul(matrix_set.projection, matrix_set.view), matrix_set.model);
    for (size_t x = 0; x < 4; x++) {
        for (size_t y = 0; y < 4; y++) {
            data.matrix[x][y] = ParrotFixed32s_from_float(matrix.data[x][y]);
        }
    }

    for (size_t i = 0; i < count; i += ParrotVideoBackend_VERTEX_COUNT) {
        for (size_t j = 0; j < ParrotVideoBackend_VERTEX_COUNT; j++) {
            const ParrotVideoBackendVertex *vertex = &vertices[PARROT_MIN(i + j, count - 1)];

            data.vertex_x[j] = ParrotFixed16i_from_float(vertex->position.x);
            data.vertex_y[j] = ParrotFixed16i_from_float(vertex->position.y);
            data.vertex_z[j] = ParrotFixed16i_from_float(vertex->position.z);

            data.vertex_u[j] = ParrotFixed16i_from_float(vertex->uv.x);
            data.vertex_v[j] = ParrotFixed16i_from_float(vertex->uv.y);
        }

#define X(x) ((uint8_t)((x) * 255.0f + 0.5f))
        data.color_rgba8888 = (X(color.x) << 24) | (X(color.y) << 16) | (X(color.z) << 8) | X(color.w);
#undef X
        ParrotRSPTaskHandle task = ParrotRSP_queue_task(
            &ParrotVideoBackend_rsp_ucode, ParrotVideoBackend_rsp_ucode_size, &data, sizeof(ParrotVideoBackendRSPData));

        arrpush(arr_tasks, task);
    }

    ParrotRSP_begin_safe();
    for (size_t i = arrlen(arr_tasks); i > 0; i--) {
        ParrotRSP_move_task_to_front(arr_tasks[i - 1]);
        ParrotRSP_auto_cancel_task(arr_tasks[i - 1]);
    }
    ParrotRSP_end_safe();

    for (size_t i = 0; i < arrlen(arr_tasks); i++) {
        ParrotRSP_wait_task(arr_tasks[i]);
    }

    arrfree(arr_tasks);
}
*/
