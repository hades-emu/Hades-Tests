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

u32 const volatile g_patched_data = 0xDEADBEEF;
u8 volatile g_write_data[20] EWRAM_DATA = { 0 };
u16 g_remaining_errors = 0;

THUMB_CODE
void
cheat_gameshark_basic_please_hook_me(
    void
) { }

IWRAM_CODE
void
cheat_gameshark_basic_print_result(
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
cheat_gameshark_basic_codes_byte_write(
    void
) {
    bool success;

    success = (g_write_data[0] == 0x7E);

    if (success) {
        printf("8-BIT WRITE      PASS\n");
    } else {
        printf("8-BIT WRITE      FAIL\n");
        cheat_gameshark_basic_print_result(1, g_write_data[0], 0x7E);
    }

    return success;
}

IWRAM_CODE
bool
cheat_gameshark_basic_codes_word_write(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[2] == 0xF00D);

    if (success) {
        printf("16-BIT WRITE     PASS\n");
    } else {
        printf("16-BIT WRITE     FAIL\n");
        cheat_gameshark_basic_print_result(2, *(u16 volatile *)&g_write_data[2], 0xF00D);
    }

    return success;
}

IWRAM_CODE
bool
cheat_gameshark_basic_codes_dword_write(
    void
) {
    bool success;

    success = (*(u32 volatile *)&g_write_data[4] == 0xDEADCA5D);

    if (success) {
        printf("32-BIT WRITE     PASS\n");
    } else {
        printf("32-BIT WRITE     FAIL\n");
        cheat_gameshark_basic_print_result(4, *(u32 volatile *)&g_write_data[4], 0xDEADCA5D);
    }

    return success;
}

IWRAM_CODE
bool
cheat_gameshark_basic_codes_rom_patch(
    void
) {
    bool success;

    success = (g_patched_data == 0xFEEDC0DE);

    if (success) {
        printf("ROM PATCH        PASS\n");
    } else {
        printf("ROM PATCH        FAIL\n");
        cheat_gameshark_basic_print_result(4, g_patched_data, 0xFEEDC0DE);
    }

    return success;
}

IWRAM_CODE
bool
cheat_gameshark_basic_codes_if_equal(
    void
) {
    bool success;

    success = true;
    success &= (g_write_data[8] == 0x7A);
    success &= (g_write_data[9] == 0xC3);
    success &= (g_write_data[10] == 0x7C);

    if (success) {
        printf("IF EQUAL         PASS\n");
    } else {
        printf("IF EQUAL         FAIL\n");
        cheat_gameshark_basic_print_result(1, g_write_data[8], 0x7A);
        cheat_gameshark_basic_print_result(1, g_write_data[9], 0xC3);
        cheat_gameshark_basic_print_result(1, g_write_data[10], 0x7C);
    }

    return success;
}

IWRAM_CODE
bool
cheat_gameshark_basic_codes_if_equal_block(
    void
) {
    bool success;

    success = true;
    success &= (g_write_data[11] == 0x11);
    success &= (g_write_data[12] == 0x22);
    success &= (g_write_data[13] == 0x5E);
    success &= (g_write_data[14] == 0xA9);
    success &= (g_write_data[15] == 0x33);

    if (success) {
        printf("IF EQUAL BLOCK   PASS\n");
    } else {
        printf("IF EQUAL BLOCK   FAIL\n");
        cheat_gameshark_basic_print_result(1, g_write_data[11], 0x11);
        cheat_gameshark_basic_print_result(1, g_write_data[12], 0x22);
        cheat_gameshark_basic_print_result(1, g_write_data[13], 0x5E);
        cheat_gameshark_basic_print_result(1, g_write_data[14], 0xA9);
        cheat_gameshark_basic_print_result(1, g_write_data[15], 0x33);
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

    printf("Cheat - GameShark Basic\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_gameshark_basic_please_hook_me & ~01);
    printf("Address to patch: 0x%08lx\n", (u32)&g_patched_data);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("\n");

    // Set the initial values of all the memory that will be read by the different cheat codes
    *(u16 volatile *)&g_write_data[16] = 0x1234;      // IF Equal
    *(u16 volatile *)&g_write_data[18] = 0x5678;      // IF Equal (block)
    g_write_data[9] = 0xC3;                           // IF Equal (false)
    g_write_data[13] = 0x5E;                          // IF Equal (block, false)
    g_write_data[14] = 0xA9;                          // IF Equal (block, false)

    cheat_gameshark_basic_please_hook_me();

    bool (* const tests[])(void) = {
        cheat_gameshark_basic_codes_byte_write,
        cheat_gameshark_basic_codes_word_write,
        cheat_gameshark_basic_codes_dword_write,
        cheat_gameshark_basic_codes_rom_patch,
        cheat_gameshark_basic_codes_if_equal,
        cheat_gameshark_basic_codes_if_equal_block,
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
