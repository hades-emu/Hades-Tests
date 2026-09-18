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
u16 g_remaining_errors = 0;

THUMB_CODE
void
cheat_parv3_advance_please_hook_me(
    void
) { }

IWRAM_CODE
void
cheat_parv3_advance_print_result(
    uint32_t result,
    uint32_t expected
) {
    if (expected != result && g_remaining_errors) {
        printf("    0x%08lx != 0x%08lx\n", result, expected);
        --g_remaining_errors;
    }
}

IWRAM_CODE
bool
cheat_parv3_advance_button_ready(
    void
) {
    return g_write_data[0] == 0xAB
        && *(u16 volatile *)&g_write_data[2] == 0xBEEF
        && *(u32 volatile *)&g_write_data[4] == 0xCAFEBABE;
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_button(
    void
) {
    bool success;
    u32 i;

    printf("IF AR BUTTON     WAIT...");

    success = cheat_parv3_advance_button_ready();

    for (i = 0; i < 180 && !success; ++i) {
        VBlankIntrWait();
        cheat_parv3_advance_please_hook_me();
        success = cheat_parv3_advance_button_ready();
    }

    printf(CON_LEFT(7));

    if (success) {
        printf("PASS   \n");
    } else {
        printf("FAIL   \n");
        cheat_parv3_advance_print_result(g_write_data[0], 0xAB);
        cheat_parv3_advance_print_result(*(u16 volatile *)&g_write_data[2], 0xBEEF);
        cheat_parv3_advance_print_result(*(u32 volatile *)&g_write_data[4], 0xCAFEBABE);
    }

    return success;
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_io(
    void
) {
    bool success;
    u32 volatile *io1;
    u32 volatile *io2;

    io1 = (u32 volatile *)(REG_BASE + 0x90);
    io2 = (u32 volatile *)(REG_BASE + 0x94);

    success = true;
    success &= (*io1 == 0xFEEDBAAD);
    success &= (*io2 == 0xBEEFBABE);

    if (success) {
        printf("IO               PASS\n");
    } else {
        printf("IO               FAIL\n");
        cheat_parv3_advance_print_result(*io1, 0xFEEDBAAD);
        cheat_parv3_advance_print_result(*io2, 0xBEEFBABE);
    }

    return success;
}

THUMB_CODE
int
main(
    void
) {
    size_t nb_tests;
    u16 nb_tests_passed;
    u16 i;

    irqInit();
    consoleDemoInit();

    irqEnable(IRQ_VBLANK);

    printf("Cheat - PARv3 Advance\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_parv3_advance_please_hook_me & ~01);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("\n");

    // Set the initial values of all the memory that will be read by the different cheat codes
    *(u32 volatile *)(REG_BASE + 0x90) = 0xFEEDFACE;
    *(u32 volatile *)(REG_BASE + 0x94) = 0xACE0FBA5;

    bool (* const tests[])(void) = {
        cheat_parv3_advance_codes_button,
        cheat_parv3_advance_codes_io,
    };

    nb_tests = sizeof(tests) / sizeof(tests[0]);
    nb_tests_passed = 0;
    g_remaining_errors = 5;

    for (i = 0; i < nb_tests; ++i) {
        nb_tests_passed += tests[i]();
    }

    printf("\n");
    printf("Total: %u/%zu\n", nb_tests_passed, nb_tests);

    while (true) {
        VBlankIntrWait();
    }
}
