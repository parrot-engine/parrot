#include "parrot/platform/n64/rsp.h"
#include "parrot/core/util.h"
#include "parrot/platform/n64/util.h"
#include "stb_ds.h"
#include <stddef.h>
#include <string.h>

#define SP_STATUS (*(volatile uint32_t *)0xA4040010)
#define SP_PC (*(volatile uint32_t *)0xA4080000)

typedef struct {
    size_t key;

    uint8_t *ucode;
    size_t ucode_size;

    uint8_t *data;
    size_t data_size;

    bool cancel_on_finish;
    void (*finish_callback)(void *ctx);
    void *finish_callback_ctx;

    bool finished;
    bool running;
} ParrotRSPTask;

typedef struct {
    bool paused;

    size_t next_task_handle;
    ParrotRSPTask *hm_tasks;
    ParrotRSPTaskHandle *arr_task_queue;

    ParrotRSPTask *running_task;

    size_t safe_count;
} ParrotRSP;

static ParrotRSP *self = NULL;

void ParrotRSP_init(void) {
    PARROT_FAIL_COND(ParrotRSP_is_initialized());

    self = malloc(sizeof(ParrotRSP));
    PARROT_FAIL_NULL(self);
    memset(self, 0, sizeof(ParrotRSP));
}

void ParrotRSP_shutdown(void) {
    PARROT_FAIL_COND(!ParrotRSP_is_initialized());

    ParrotRSP_set_paused(true);
    {
        ParrotRSPTaskHandle handle;
        if (ParrotRSP_get_running_task(&handle)) {
            ParrotRSP_cancel_task(handle);
        }
    }

    arrfree(self->arr_task_queue);
    hmfree(self->hm_tasks);

    free(self);
    self = NULL;
}

bool ParrotRSP_is_initialized(void) {
    return self;
}

void ParrotRSP_schedule(void) {
    PARROT_FAIL_COND(!ParrotRSP_is_initialized());
    PARROT_RET_COND(self->safe_count > 0);

    uint8_t *sp_dmem = (uint8_t *)0xA4000000;
    uint8_t *sp_imem = (uint8_t *)0xA4001000;

    ParrotRSP_begin_safe();

    if (self->running_task && (SP_STATUS & /* Broke */ (1 << 1))) {
        self->running_task->finished = true;

        if (self->running_task->finish_callback) {
            self->running_task->finish_callback(self->running_task->finish_callback_ctx);
        }

        if (self->running_task->cancel_on_finish) {
            ParrotRSP_cancel_task((ParrotRSPTaskHandle){
                .handle = self->running_task->key,
            });
        }

        self->running_task = NULL;
    }

    if (self->paused || self->running_task) {
        ParrotRSP_end_safe();
        return;
    }

    while (arrlen(self->arr_task_queue) > 0) {
        ParrotRSPTaskHandle handle = arrpop(self->arr_task_queue);
        ParrotRSPTask *task = hmgetp_null(self->hm_tasks, handle);

        if (!task) {
            /* HACK: Cancelled tasks are removed from the queue when they get to the front. This removes the need to
             * expensively delete immediately when cancelled
             */
            continue;
        }

        self->running_task = task;

        memset(sp_imem, 0, 4096);
        memset(sp_dmem, 0, 4096);

        memcpy(sp_imem, task->ucode, task->ucode_size);
        memcpy(sp_dmem, task->data, task->data_size);

        free(task->ucode);
        free(task->data);

        task->ucode = NULL;
        task->data = NULL;

        PARROT_N64_IO_WRITE(SP_PC, 0);
        PARROT_N64_IO_WRITE(SP_STATUS, (/* Resume */ 1 << 0) | (/* Clear break */ 1 << 2));
        task->running = true;

        break;
    }

    ParrotRSP_end_safe();
}

void ParrotRSP_set_paused(bool paused) {
    PARROT_FAIL_COND(!ParrotRSP_is_initialized());

    self->paused = paused;

    ParrotRSP_schedule();
}

void ParrotRSP_begin_safe(void) {
    PARROT_FAIL_COND(!ParrotRSP_is_initialized());

    self->safe_count++;
}

void ParrotRSP_end_safe(void) {
    PARROT_FAIL_COND(!ParrotRSP_is_initialized());

    PARROT_FAIL_COND(self->safe_count == 0);
    self->safe_count--;
}

bool ParrotRSP_get_running_task(ParrotRSPTaskHandle *handle) {
    ParrotRSP_schedule();
    ParrotRSP_begin_safe();

    if (handle && self->running_task) {
        *handle = (ParrotRSPTaskHandle){
            .handle = self->running_task->key,
        };
    }

    ParrotRSP_end_safe();
    return self->running_task;
}

ParrotRSPTaskHandle ParrotRSP_queue_task(const void *ucode, size_t ucode_size, const void *data, size_t data_size) {
    PARROT_FAIL_COND(ucode_size > 4096);
    PARROT_FAIL_COND(data_size > 4096);

    ParrotRSP_begin_safe();

    ParrotRSPTask task = {0};

    task.ucode = malloc(ucode_size);
    memcpy(task.ucode, ucode, ucode_size);
    task.ucode_size = ucode_size;

    task.data = malloc(data_size);
    memcpy(task.data, data, data_size);
    task.data_size = data_size;

    task.key = self->next_task_handle++;

    ParrotRSPTaskHandle handle = (ParrotRSPTaskHandle){
        .handle = task.key,
    };

    hmputs(self->hm_tasks, task);
    arrins(self->arr_task_queue, 0, handle);

    ParrotRSP_end_safe();
    ParrotRSP_schedule();

    return handle;
}

void ParrotRSP_move_task_to_front(ParrotRSPTaskHandle handle) {
    PARROT_RET_COND(ParrotRSP_poll_task(handle) == ParrotRSPTaskStatus_NOT_FOUND);
    ParrotRSP_begin_safe();

    bool found = false;
    for (size_t i = 0; i < arrlen(self->arr_task_queue); i++) {
        if (self->arr_task_queue[i].handle == handle.handle) {
            arrdel(self->arr_task_queue, i);
            found = true;
            break;
        }
    }

    if (found) {
        arrpush(self->arr_task_queue, handle);
    }

    ParrotRSP_end_safe();
    ParrotRSP_schedule();
}

void ParrotRSP_cancel_task(ParrotRSPTaskHandle handle) {
    PARROT_RET_COND(ParrotRSP_poll_task(handle) == ParrotRSPTaskStatus_NOT_FOUND);
    ParrotRSP_begin_safe();

    ParrotRSPTask *task = hmgetp(self->hm_tasks, handle);

    if (self->running_task == task) {
        self->running_task = NULL;
    }

    if (task->running) {
        PARROT_N64_IO_WRITE(SP_STATUS, /* Halt */ 1 << 1);
    }

    free(task->ucode);
    free(task->data);

    hmdel(self->hm_tasks, task->key);

    /* HACK: Lazily freed. ParrotRSP_schedule() removes the task later because searching and deleting immediately is
     * computationally expensive depending on the task count and the position in the queue.
     */

    ParrotRSP_end_safe();
    ParrotRSP_schedule();
}

void ParrotRSP_set_finish_callback(ParrotRSPTaskHandle handle, void (*callback)(void *), void *ctx) {
    PARROT_RET_COND(ParrotRSP_poll_task(handle) == ParrotRSPTaskStatus_NOT_FOUND);
    ParrotRSP_begin_safe();

    ParrotRSPTask *task = hmgetp(self->hm_tasks, handle);
    task->finish_callback = callback;
    task->finish_callback_ctx = ctx;

    ParrotRSP_end_safe();
    ParrotRSP_schedule();
}

void ParrotRSP_auto_cancel_task(ParrotRSPTaskHandle handle) {
    PARROT_RET_COND(ParrotRSP_poll_task(handle) == ParrotRSPTaskStatus_NOT_FOUND);

    if (ParrotRSP_poll_task(handle) == ParrotRSPTaskStatus_FINISHED) {
        ParrotRSP_cancel_task(handle);
    } else {
        hmgetp(self->hm_tasks, handle)->cancel_on_finish = true;
    }

    ParrotRSP_schedule();
}

void ParrotRSP_wait_task(ParrotRSPTaskHandle handle) {
    // While I would like to do better than a busy loop, on bare-metal, you kinda have nothing else to do really
    for (;;) {
        ParrotRSPTaskStatus status = ParrotRSP_poll_task(handle);
        if (status == ParrotRSPTaskStatus_NOT_FOUND || status == ParrotRSPTaskStatus_FINISHED) {
            break;
        }
    }
}

ParrotRSPTaskStatus ParrotRSP_poll_task(ParrotRSPTaskHandle handle) {
    ParrotRSP_schedule();
    ParrotRSP_begin_safe();

    ParrotRSPTask *task = hmgetp_null(self->hm_tasks, handle);
    PARROT_RET_COND_V(!task, ParrotRSPTaskStatus_NOT_FOUND);

    if (task->finished) {
        ParrotRSP_end_safe();
        return ParrotRSPTaskStatus_FINISHED;
    }

    if (self->running_task == task) {
        ParrotRSP_end_safe();
        return ParrotRSPTaskStatus_RUNNING;
    }

    ParrotRSP_end_safe();
    return ParrotRSPTaskStatus_QUEUED;
}
