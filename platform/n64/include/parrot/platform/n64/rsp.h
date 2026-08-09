#ifndef PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_RSP_H_
#define PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_RSP_H_

#include "parrot/core/api.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    ParrotRSPTaskStatus_NOT_FOUND = 0,
    ParrotRSPTaskStatus_QUEUED,
    ParrotRSPTaskStatus_RUNNING,
    ParrotRSPTaskStatus_FINISHED,
} ParrotRSPTaskStatus;

typedef struct {
    size_t handle;
} ParrotRSPTaskHandle;

PARROT_API void ParrotRSP_init(void);
/// Cancels all running tasks
PARROT_API void ParrotRSP_shutdown(void);
PARROT_API bool ParrotRSP_is_initialized(void);

/// Checks the RSP status to update task status'. Auto called by any task functions
PARROT_API void ParrotRSP_schedule(void);
/**
 * Will pause schelduing of tasks any further but finishes the currently running task. If you want to use the RSP
 * without shutting down ParrotRSP, you must pause ParrotRSP and wait for the running task.
 */
PARROT_API void ParrotRSP_set_paused(bool paused);
/** Makes sure no modfications to the queue (which includes the running task) is done while this is active. This
 * function can be called multiple times but requires an equal amount of ends to remove this effect.
 */
PARROT_API void ParrotRSP_begin_safe(void);
PARROT_API void ParrotRSP_end_safe(void);
PARROT_API bool ParrotRSP_get_running_task(/* NULL = not populated */ ParrotRSPTaskHandle *handle);

PARROT_API ParrotRSPTaskHandle ParrotRSP_queue_task(const void *ucode,
                                                    size_t ucode_size,
                                                    const void *data,
                                                    size_t data_size);
/// Makes the task jump to the front of the queue immediately executing after that task is done
PARROT_API void ParrotRSP_move_task_to_front(ParrotRSPTaskHandle handle);
/// Removes task from queue no matter the state it's in
PARROT_API void ParrotRSP_cancel_task(ParrotRSPTaskHandle handle);
PARROT_API void
ParrotRSP_set_finish_callback(ParrotRSPTaskHandle handle, /* NULL = uncalled */ void (*callback)(void *ctx), void *ctx);
/// Asynchrously cancels the task after it's done or immediately cancels it if it's already done
PARROT_API void ParrotRSP_auto_cancel_task(ParrotRSPTaskHandle handle);
/// Waits until the task either doesn't exist or is finished
PARROT_API void ParrotRSP_wait_task(ParrotRSPTaskHandle handle);
PARROT_API ParrotRSPTaskStatus ParrotRSP_poll_task(ParrotRSPTaskHandle handle);

#endif // PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_RSP_H_
