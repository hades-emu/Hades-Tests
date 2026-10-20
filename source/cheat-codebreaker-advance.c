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

u8 volatile g_write_data[22] EWRAM_DATA = { 0 };
u16 g_remaining_errors = 0;

THUMB_CODE
void
cheat_codebreaker_advance_please_hook_me(
    void
) { }

IWRAM_CODE
void
cheat_codebreaker_advance_print_result(
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
cheat_codebreaker_advance_codes_if_equal(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[0] == 0x6298);

    if (success) {
        printf("IF EQUAL         PASS\n");
    } else {
        printf("IF EQUAL         FAIL\n");
        cheat_codebreaker_advance_print_result(2, *(u16 volatile *)&g_write_data[0], 0x6298);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_advance_codes_if_not_equal(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[2] == 0x7C2B);

    if (success) {
        printf("IF NOT EQUAL     PASS\n");
    } else {
        printf("IF NOT EQUAL     FAIL\n");
        cheat_codebreaker_advance_print_result(2, *(u16 volatile *)&g_write_data[2], 0x7C2B);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_advance_codes_if_greater(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[4] == 0x551E);

    if (success) {
        printf("IF GREATER       PASS\n");
    } else {
        printf("IF GREATER       FAIL\n");
        cheat_codebreaker_advance_print_result(2, *(u16 volatile *)&g_write_data[4], 0x551E);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_advance_codes_if_lower(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[6] == 0xB892);

    if (success) {
        printf("IF LOWER         PASS\n");
    } else {
        printf("IF LOWER         FAIL\n");
        cheat_codebreaker_advance_print_result(2, *(u16 volatile *)&g_write_data[6], 0xB892);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_advance_codes_if_and(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[8] == 0xFDD8);

    if (success) {
        printf("IF AND           PASS\n");
    } else {
        printf("IF AND           FAIL\n");
        cheat_codebreaker_advance_print_result(2, *(u16 volatile *)&g_write_data[8], 0xFDD8);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_advance_codes_if_button(
    void
) {
    bool success;
    u32 i;

    printf("IF BUTTON        WAIT...");

    success = (*(u16 volatile *)&g_write_data[10] == 0x2793);

    for (i = 0; i < 180 && !success; ++i) {
        VBlankIntrWait();
        cheat_codebreaker_advance_please_hook_me();
        success = (*(u16 volatile *)&g_write_data[10] == 0x2793);
    }

    printf(CON_LEFT(7));

    if (success) {
        printf("PASS   \n");
    } else {
        printf("FAIL   \n");
        cheat_codebreaker_advance_print_result(2, *(u16 volatile *)&g_write_data[10], 0x2793);
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

    printf("Cheat - CodeBreaker Advance\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_codebreaker_advance_please_hook_me & ~01);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("Button to press:  SELECT\n");
    printf("\n");

    // Set the initial values of all the memory that will be read by the different cheat codes
    *(u16 volatile *)&g_write_data[12] = 0x96D2;   // IF Equal
    *(u16 volatile *)&g_write_data[14] = 0xE176;   // IF Not Equal
    *(u16 volatile *)&g_write_data[16] = 0x8D7A;   // IF Greater (Unsigned)
    *(u16 volatile *)&g_write_data[18] = 0x03BD;   // IF Lower (Unsigned)
    *(u16 volatile *)&g_write_data[20] = 0x7A6C;   // IF And

    cheat_codebreaker_advance_please_hook_me();

    bool (* const tests[])(void) = {
        cheat_codebreaker_advance_codes_if_equal,
        cheat_codebreaker_advance_codes_if_not_equal,
        cheat_codebreaker_advance_codes_if_greater,
        cheat_codebreaker_advance_codes_if_lower,
        cheat_codebreaker_advance_codes_if_and,
        cheat_codebreaker_advance_codes_if_button,
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
