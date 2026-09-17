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

u32 const volatile g_patched_data = 0xDE4DB3EF;
u32 volatile g_write_data[16] EWRAM_DATA = { 0 };
u32 g_remaining_errors = 0;

THUMB_CODE
void
cheat_parv3_basic_please_hook_me(
    void
) { }

void
cheat_parv3_basic_print_result(
    uint32_t expected,
    uint32_t result
) {
    if (expected != result && g_remaining_errors) {
        printf("    0x%08lx != 0x%08lx\n", expected, result);
        --g_remaining_errors;
    }
}

IWRAM_CODE
bool
cheat_parv3_basic_codes_rom_patch(
    void
) {
    bool success;

    success = g_patched_data == 0x8BADF00D;

    if (success) {
        printf("ROM PATCH        PASS\n");
    } else {
        printf("ROM PATCH        FAIL\n");
        cheat_parv3_basic_print_result(g_patched_data, 0x8BADF00D);
    }

    return success;
}

IWRAM_CODE
bool
cheat_parv3_basic_codes_assign(
    void
) {
    bool success;

    cheat_parv3_basic_please_hook_me();

    success = true;
    success &= (g_write_data[0] == 0x42424242);
    success &= (g_write_data[1] == 0x21842184);
    success &= (g_write_data[2] == 0xAABBCCDD);

    if (success) {
        printf("ASSIGN           PASS\n");
    } else {
        printf("ASSIGN           FAIL\n");
        cheat_parv3_basic_print_result(g_write_data[0], 0x42424242);
        cheat_parv3_basic_print_result(g_write_data[1], 0x21842184);
        cheat_parv3_basic_print_result(g_write_data[2], 0xAABBCCDD);
    }

    return success;
}

IWRAM_CODE
bool
cheat_parv3_basic_codes_indirect_assign(
    void
) {
    bool success;

    g_write_data[3] = (u32)&g_write_data[4];

    cheat_parv3_basic_please_hook_me();

    success = true;
    success &= (g_write_data[4] == 0xA1B2C3D4);
    success &= (g_write_data[5] == 0x00432285);

    if (success) {
        printf("INDIRECT ASSIGN  PASS\n");
    } else {
        printf("INDIRECT ASSIGN  FAIL\n");
        cheat_parv3_basic_print_result(g_write_data[4], 0xA1B2C3D4);
        cheat_parv3_basic_print_result(g_write_data[5], 0x00432285);
    }

    return success;
}

IWRAM_CODE
bool
cheat_parv3_basic_codes_add_assign(
    void
) {
    bool success;

    g_write_data[6] = 0x01234567;
    g_write_data[7] = 0x89ABCDEF;

    cheat_parv3_basic_please_hook_me();

    success = true;
    success &= (g_write_data[6] == 0x18684531);
    success &= (g_write_data[7] == 0xCBEE1031);

    if (success) {
        printf("ADD ASSIGN       PASS\n");
    } else {
        printf("ADD ASSIGN       FAIL\n");
        cheat_parv3_basic_print_result(g_write_data[6], 0x18684531);
        cheat_parv3_basic_print_result(g_write_data[7], 0xCBEE1031);
    }

    return success;
}

IWRAM_CODE
bool
cheat_parv3_basic_codes_io_assign(
    void
) {
    bool success;
    u32 *io1;
    u32 *io2;

    io1 = (u32 *)(REG_BASE + 0x90);
    io2 = (u32 *)(REG_BASE + 0x94);

    *io1 = 0xFEEDFACE;
    *io2 = 0xACE0FBA5;

    cheat_parv3_basic_please_hook_me();

    success = true;
    success &= (*io1 == 0xFEEDBAAD);
    success &= (*io2 == 0xBEEFBABE);

    if (success) {
        printf("IO               PASS\n");
    } else {
        printf("IO               FAIL\n");
        cheat_parv3_basic_print_result(*io1, 0xFACEBAAD);
        cheat_parv3_basic_print_result(*io2, 0xBEEFBABE);
    }

    return success;
}

THUMB_CODE
int
main(
    void
) {
    size_t nb_tests;
    u32 nb_tests_passed;
    u32 i;

    irqInit();
    consoleDemoInit();

    irqEnable(IRQ_VBLANK);

    printf("Cheat - PARv3 Basic\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_parv3_basic_please_hook_me & ~01);
    printf("Address to patch: 0x%08lx\n", (u32)&g_patched_data);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("\n");

    bool (* const tests[])(void) = {
        cheat_parv3_basic_codes_rom_patch,
        cheat_parv3_basic_codes_assign,
        cheat_parv3_basic_codes_indirect_assign,
        cheat_parv3_basic_codes_add_assign,
        cheat_parv3_basic_codes_io_assign,
    };

    nb_tests = sizeof(tests) / sizeof(tests[0]);
    nb_tests_passed = 0;

    for (i = 0; i < nb_tests; ++i) {
        g_remaining_errors = 1;
        nb_tests_passed += tests[i]();
    }

    printf("\n");
    printf("Total: %lu/%zu\n", nb_tests_passed, nb_tests);

    while (true) {
        VBlankIntrWait();
    }
}
