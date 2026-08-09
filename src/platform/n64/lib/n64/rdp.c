#include "parrot/platform/n64/rdp.h"
#include "parrot/core/util.h"
#include "parrot/platform/n64/util.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define DPC_START (*(volatile uint32_t *)0xA4100000)
#define DPC_END (*(volatile uint32_t *)0xA4100004)
#define DPC_STATUS (*(volatile uint32_t *)0xA410000C)

#define DPC_STATUS_CMD_BUSY (1 << 6)
#define DPC_STATUS_PIPE_BUSY (1 << 5)
#define DPC_STATUSW_CLR_XBUS (1 << 0)

bool ParrotRDP_send_dl(const ParrotRDPCommand *dl, size_t count) {
    // The cache writeback can't be called on const ptrs
    ParrotRDPCommand *buffer = calloc(count, sizeof(ParrotRDPCommand));
    PARROT_RET_COND_V(!buffer, false);
    memcpy(buffer, dl, sizeof(ParrotRDPCommand) * count);

    Parrot_n64_writeback_invalidiate_data_cache(buffer, sizeof(ParrotRDPCommand) * count);

    while (DPC_STATUS & (DPC_STATUS_CMD_BUSY | DPC_STATUS_PIPE_BUSY))
        ;

    PARROT_N64_IO_WRITE(DPC_STATUS, DPC_STATUSW_CLR_XBUS);
    PARROT_N64_IO_WRITE(DPC_START, PARROT_N64_PHYSICAL_ADDRESS(buffer));
    PARROT_N64_IO_WRITE(DPC_END, PARROT_N64_PHYSICAL_ADDRESS(buffer + count));

    // Kinda a bad idea to free the buffer before the RDP has the DL so we wait here
    while (DPC_STATUS & (DPC_STATUS_CMD_BUSY | DPC_STATUS_PIPE_BUSY))
        ;

    /*printf("DL: 0x%p\n", buffer);

    for (;;)
        ;*/

    free(buffer);
    return true;
}
