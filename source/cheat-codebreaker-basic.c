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

u8 volatile g_write_data[10] EWRAM_DATA = { 0 };
u16 g_remaining_errors = 0;

THUMB_CODE
void
cheat_codebreaker_basic_please_hook_me(
    void
) { }

IWRAM_CODE
void
cheat_codebreaker_basic_print_result(
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
cheat_codebreaker_basic_codes_or_assign(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[0] == 0xF5FD);

    if (success) {
        printf("OR ASSIGN        PASS\n");
    } else {
        printf("OR ASSIGN        FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[0], 0xF5FD);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_byte_write(
    void
) {
    bool success;

    success = (g_write_data[2] == 0xA2);

    if (success) {
        printf("8-BIT WRITE      PASS\n");
    } else {
        printf("8-BIT WRITE      FAIL\n");
        cheat_codebreaker_basic_print_result(1, g_write_data[2], 0xA2);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_and_assign(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[4] == 0x280F);

    if (success) {
        printf("AND ASSIGN       PASS\n");
    } else {
        printf("AND ASSIGN       FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[4], 0x280F);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_word_write(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[6] == 0xBBCA);

    if (success) {
        printf("16-BIT WRITE     PASS\n");
    } else {
        printf("16-BIT WRITE     FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[6], 0xBBCA);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_add_assign(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[8] == 0xA868);

    if (success) {
        printf("ADD ASSIGN       PASS\n");
    } else {
        printf("ADD ASSIGN       FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[8], 0xA868);
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

    printf("Cheat - CodeBreaker Basic\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_codebreaker_basic_please_hook_me & ~01);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("\n");

    // Set the initial values of all the memory that will be read by the different cheat codes
    *(u16 volatile *)&g_write_data[0] = 0xD159;    // OR Assign
    *(u16 volatile *)&g_write_data[4] = 0x2A8F;    // AND Assign
    *(u16 volatile *)&g_write_data[8] = 0x4BEC;    // Add Assign

    cheat_codebreaker_basic_please_hook_me();

    bool (* const tests[])(void) = {
        cheat_codebreaker_basic_codes_or_assign,
        cheat_codebreaker_basic_codes_byte_write,
        cheat_codebreaker_basic_codes_and_assign,
        cheat_codebreaker_basic_codes_word_write,
        cheat_codebreaker_basic_codes_add_assign,
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
