/******************************************************************************\
**
**  This file is part of the Hades GBA Emulator, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2021-2026 - The Hades Authors
**
\******************************************************************************/

#include "gba_console.h"
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <stdio.h>

u8 volatile g_write_data[8] EWRAM_DATA = { 0 };

THUMB_CODE
void
cheat_parv3_advance_please_hook_me(
    void
) { }

static
bool
cheat_parv3_advance_done(
    void
) {
    return (
        g_write_data[0] == 0xAB
        && *(u16 volatile *)&g_write_data[2] == 0xBEEF
        && *(u32 volatile *)&g_write_data[4] == 0xCAFEBABE
    );
}

THUMB_CODE
int
main(
    void
) {
    irqInit();
    consoleDemoInit();

    irqEnable(IRQ_VBLANK);

    printf("Cheat - PARv3 Advance\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_parv3_advance_please_hook_me & ~01);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("\n");
    printf("AR Button: WAIT...\n");

    while (!cheat_parv3_advance_done()) {
        cheat_parv3_advance_please_hook_me();
        VBlankIntrWait();
    }

    printf("AR Button: PASS\n");
    printf("\n");
    printf("Total: 1/1\n");

    while (true) {
        VBlankIntrWait();
    }
}
