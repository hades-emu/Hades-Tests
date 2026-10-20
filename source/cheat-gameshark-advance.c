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

u8 volatile g_write_data[16] EWRAM_DATA = { 0 };
u16 g_remaining_errors = 0;

THUMB_CODE
void
cheat_gameshark_advance_please_hook_me(
    void
) { }

IWRAM_CODE
void
cheat_gameshark_advance_print_result(
    int width,
    uint32_t result,
    uint32_t expected
) {
    if (expected != result && g_remaining_errors) {
        printf("    0x%0*lx != 0x%0*lx\n", width * 2, result, width * 2, expected);
        --g_remaining_errors;
    }
}

IWRAM_CODE
bool
cheat_gameshark_advance_codes_multi_write(
    void
) {
    bool success;

    success = true;
    success &= (*(u32 volatile *)&g_write_data[0] == 0xC0FFEE11);
    success &= (*(u32 volatile *)&g_write_data[4] == 0xC0FFEE11);
    success &= (*(u32 volatile *)&g_write_data[8] == 0xC0FFEE11);

    if (success) {
        printf("MULTI-WRITE      PASS\n");
    } else {
        printf("MULTI-WRITE      FAIL\n");
        cheat_gameshark_advance_print_result(4, *(u32 volatile *)&g_write_data[0], 0xC0FFEE11);
        cheat_gameshark_advance_print_result(4, *(u32 volatile *)&g_write_data[4], 0xC0FFEE11);
        cheat_gameshark_advance_print_result(4, *(u32 volatile *)&g_write_data[8], 0xC0FFEE11);
    }

    return success;
}

IWRAM_CODE
bool
cheat_gameshark_advance_button_ready(
    void
) {
    return g_write_data[12] == 0xAB
        && *(u16 volatile *)&g_write_data[14] == 0xBEEF;
}

IWRAM_CODE
bool
cheat_gameshark_advance_codes_button(
    void
) {
    bool success;
    u32 i;

    printf("IF GS BUTTON     WAIT...");

    success = cheat_gameshark_advance_button_ready();

    for (i = 0; i < 180 && !success; ++i) {
        VBlankIntrWait();
        cheat_gameshark_advance_please_hook_me();
        success = cheat_gameshark_advance_button_ready();
    }

    printf(CON_LEFT(7));

    if (success) {
        printf("PASS   \n");
    } else {
        printf("FAIL   \n");
        cheat_gameshark_advance_print_result(1, g_write_data[12], 0xAB);
        cheat_gameshark_advance_print_result(2, *(u16 volatile *)&g_write_data[14], 0xBEEF);
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

    printf("Cheat - GameShark Advance\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_gameshark_advance_please_hook_me & ~01);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("Button to press:  SELECT\n");
    printf("\n");

    cheat_gameshark_advance_please_hook_me();

    bool (* const tests[])(void) = {
        cheat_gameshark_advance_codes_multi_write,
        cheat_gameshark_advance_codes_button,
    };

    nb_tests = sizeof(tests) / sizeof(tests[0]);
    nb_tests_passed = 0;
    g_remaining_errors = 4;

    for (i = 0; i < nb_tests; ++i) {
        nb_tests_passed += tests[i]();
    }

    printf("\n");
    printf("Total: %u/%zu\n", nb_tests_passed, nb_tests);

    while (true) {
        VBlankIntrWait();
    }
}
