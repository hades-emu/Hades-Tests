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

u8 volatile g_write_data[32] EWRAM_DATA = { 0 };
u32 g_remaining_errors = 0;

THUMB_CODE
void
cheat_codebreaker_basic_please_hook_me(
    void
) { }

void
cheat_codebreaker_basic_print_result(
    int width,
    uint32_t expected,
    uint32_t result
) {
    if (expected != result && g_remaining_errors) {
        printf("    0x%0*lx != 0x%0*lx\n", width * 2, expected, width * 2, result);
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
cheat_codebreaker_basic_codes_if_equal(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[8] == 0x6298);

    if (success) {
        printf("IF EQUAL         PASS\n");
    } else {
        printf("IF EQUAL         FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[8], 0x6298);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_word_write(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[10] == 0xBBCA);

    if (success) {
        printf("16-BIT WRITE     PASS\n");
    } else {
        printf("16-BIT WRITE     FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[10], 0xBBCA);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_if_not_equal(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[14] == 0x7C2B);

    if (success) {
        printf("IF NOT EQUAL     PASS\n");
    } else {
        printf("IF NOT EQUAL     FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[14], 0x7C2B);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_if_greater(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[18] == 0x551E);

    if (success) {
        printf("IF GREATER       PASS\n");
    } else {
        printf("IF GREATER       FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[18], 0x551E);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_if_lower(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[22] == 0xB892);

    if (success) {
        printf("IF LOWER         PASS\n");
    } else {
        printf("IF LOWER         FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[22], 0xB892);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_if_button(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[24] == 0x2793);

    if (success) {
        printf("IF BUTTON        PASS\n");
    } else {
        printf("IF BUTTON        FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[24], 0x2793);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_add_assign(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[26] == 0xA868);

    if (success) {
        printf("ADD ASSIGN       PASS\n");
    } else {
        printf("ADD ASSIGN       FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[26], 0xA868);
    }

    return success;
}

IWRAM_CODE
bool
cheat_codebreaker_basic_codes_if_and(
    void
) {
    bool success;

    success = (*(u16 volatile *)&g_write_data[30] == 0xFDD8);

    if (success) {
        printf("IF AND           PASS\n");
    } else {
        printf("IF AND           FAIL\n");
        cheat_codebreaker_basic_print_result(2, *(u16 volatile *)&g_write_data[30], 0xFDD8);
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

    printf("Cheat - CodeBreaker Basic\n");

    printf("\n");
    printf("Address to hook:  0x%08lx\n", (u32)&cheat_codebreaker_basic_please_hook_me & ~01);
    printf("Address to write: 0x%08lx\n", (u32)&g_write_data);
    printf("\n");

    // Seed the values read by the OR/AND/ADD-assign and IF conditional codes.
    *(u16 volatile *)&g_write_data[0] = 0xD159;    // OR Assign
    *(u16 volatile *)&g_write_data[4] = 0x2A8F;    // AND Assign
    *(u16 volatile *)&g_write_data[6] = 0x96D2;    // IF Equal
    *(u16 volatile *)&g_write_data[12] = 0xE176;   // IF Not Equal
    *(u16 volatile *)&g_write_data[16] = 0x8D7A;   // IF Greater (Unsigned)
    *(u16 volatile *)&g_write_data[20] = 0x03BD;   // IF Lower (Unsigned)
    *(u16 volatile *)&g_write_data[26] = 0x4BEC;   // Add Assign
    *(u16 volatile *)&g_write_data[28] = 0x7A6C;   // IF And

    cheat_codebreaker_basic_please_hook_me();

    bool (* const tests[])(void) = {
        cheat_codebreaker_basic_codes_or_assign,
        cheat_codebreaker_basic_codes_byte_write,
        cheat_codebreaker_basic_codes_and_assign,
        cheat_codebreaker_basic_codes_if_equal,
        cheat_codebreaker_basic_codes_word_write,
        cheat_codebreaker_basic_codes_if_not_equal,
        cheat_codebreaker_basic_codes_if_greater,
        cheat_codebreaker_basic_codes_if_lower,
        cheat_codebreaker_basic_codes_if_button,
        cheat_codebreaker_basic_codes_add_assign,
        cheat_codebreaker_basic_codes_if_and,
    };

    nb_tests = sizeof(tests) / sizeof(tests[0]);
    nb_tests_passed = 0;
    g_remaining_errors = 4;

    for (i = 0; i < nb_tests; ++i) {
        nb_tests_passed += tests[i]();
    }

    printf("\n");
    printf("Total: %lu/%zu", nb_tests_passed, nb_tests);

    while (true) {
        VBlankIntrWait();
    }
}
