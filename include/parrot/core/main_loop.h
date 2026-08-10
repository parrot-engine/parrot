#ifndef PARROT_INCLUDE_PARROT_CORE_MAIN_LOOP_H_
#define PARROT_INCLUDE_PARROT_CORE_MAIN_LOOP_H_

#include "parrot/core/api.h"
#include <stdbool.h>

typedef struct {
    void *user_data;

    // <=0 = Uncapped
    float max_fps;
} ParrotMainLoopRunSettings;

typedef void (*ParrotMainLoopInitFunc)(ParrotMainLoopRunSettings *settings);
/// @return If the application should close
typedef bool (*ParrotMainLoopUpdateFunc)(ParrotMainLoopRunSettings *settings,
                                         float delta,
                                         /* Clears on next frame */ bool should_close);
typedef void (*ParrotMainLoopRenderFunc)(ParrotMainLoopRunSettings *settings);
typedef void (*ParrotMainLoopShutdownFunc)(ParrotMainLoopRunSettings *settings);

typedef struct ParrotMainLoop ParrotMainLoop;

PARROT_API ParrotMainLoop *ParrotMainLoop_new(void);
PARROT_API void ParrotMainLoop_delete(ParrotMainLoop *self);

/// Clears on next frame
PARROT_API void ParrotMainLoop_request_close(ParrotMainLoop *self);
/// @returns If application should keep running. Clears after first return
PARROT_API bool ParrotMainLoop_next_frame(ParrotMainLoop *self,
                                          /* NULL = Unmodified */ float *delta,
                                          /* <=0 = Uncapped */ float max_fps);

/// Makes the main loop handle your program's lifecycle. Only returns after program shuts down
PARROT_API void ParrotMainLoop_run(ParrotMainLoop *self,
                                   void *user_data,
                                   /* NULL = uncalled */ ParrotMainLoopInitFunc init,
                                   /* Required */ ParrotMainLoopUpdateFunc update,
                                   /* NULL = uncalled */ ParrotMainLoopRenderFunc render,
                                   /* NULL = uncalled */ ParrotMainLoopShutdownFunc shutdown);

#endif // PARROT_INCLUDE_PARROT_CORE_MAIN_LOOP_H_
