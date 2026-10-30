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

u8 volatile g_write_data[0x60] EWRAM_DATA = { 0 };
u16 g_remaining_errors = 0;

#define RESULT_DEFAULT(i)   ((u8)((i) * 0x9D + 0x3C))
#define RESULT_WRITTEN(i)   ((u8)~RESULT_DEFAULT(i))

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

IWRAM_CODE
bool
cheat_parv3_advance_check_results(
    char const *name,
    size_t base,
    bool const *taken,
    size_t len
) {
    bool success;
    size_t i;

    success = true;
    for (i = 0; i < len; ++i) {
        success &= (g_write_data[base + i] == (taken[i] ? RESULT_WRITTEN(base + i) : RESULT_DEFAULT(base + i)));
    }

    printf("%-17s%s\n", name, success ? "PASS" : "FAIL");

    for (i = 0; i < len; ++i) {
        cheat_parv3_advance_print_result(
            g_write_data[base + i],
            taken[i] ? RESULT_WRITTEN(base + i) : RESULT_DEFAULT(base + i)
        );
    }

    return success;
}

IWRAM_CODE
bool
cheat_parv3_advance_check_condition(
    char const *name,
    size_t base
) {
    // 8-bit true, 8-bit false, 16-bit true, 16-bit false, 32-bit true, 32-bit false
    static bool const taken[] = { true, false, true, false, true, false };

    return cheat_parv3_advance_check_results(name, base, taken, sizeof(taken) / sizeof(taken[0]));
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_equal(
    void
) {
    return cheat_parv3_advance_check_condition("IF EQUAL", 0x08);
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_not_equal(
    void
) {
    return cheat_parv3_advance_check_condition("IF NOT EQUAL", 0x0E);
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_signed_lower(
    void
) {
    return cheat_parv3_advance_check_condition("IF SIGNED <", 0x14);
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_signed_greater(
    void
) {
    return cheat_parv3_advance_check_condition("IF SIGNED >", 0x1A);
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_unsigned_lower(
    void
) {
    return cheat_parv3_advance_check_condition("IF UNSIGNED <", 0x20);
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_unsigned_greater(
    void
) {
    return cheat_parv3_advance_check_condition("IF UNSIGNED >", 0x26);
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_and(
    void
) {
    return cheat_parv3_advance_check_condition("IF AND", 0x2C);
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_actions(
    void
) {
    static bool const taken[] = {
        false, true,                // Always skip next line
        false, false, true,         // Always skip next 2 lines
        true, true, true,           // Execute next 2 codes (true)
        false, false, true,         // Execute next 2 codes (false)
        false, true,                // Execute next code (false)
    };

    return cheat_parv3_advance_check_results("IF ACTIONS", 0x32, taken, sizeof(taken) / sizeof(taken[0]));
}

IWRAM_CODE
bool
cheat_parv3_advance_codes_if_blocks(
    void
) {
    static bool const taken[] = {
        true, true, true, true,                 // IF (true) ... ENDIF
        false, false, false, true,              // IF (false) ... ENDIF
        true, true, true, false, false, true,   // IF (true) ... ELSE ... ENDIF
        false, false, false, true, true, true,  // IF (false) ... ELSE ... ENDIF
    };

    return cheat_parv3_advance_check_results("IF BLOCKS", 0x3F, taken, sizeof(taken) / sizeof(taken[0]));
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

    for (i = 0x08; i < 0x53; ++i) {
        g_write_data[i] = RESULT_DEFAULT(i);
    }

    // Data used for signed comparison.
    // The hex constats are negative numbers when read as signed values
    *(u32 volatile *)&g_write_data[0x58] = 0xC3A51F60;  // IF (32-bit)
    *(u16 volatile *)&g_write_data[0x5C] = 0x8E17;      // IF (16-bit)
    g_write_data[0x5E] = 0x9C;                          // IF (8-bit)
    g_write_data[0x5F] = 0x5A;                          // Garbage read by a too-wide 8-bit IF

    cheat_parv3_advance_please_hook_me();

    bool (* const tests[])(void) = {
        cheat_parv3_advance_codes_button,
        cheat_parv3_advance_codes_io,
        cheat_parv3_advance_codes_if_equal,
        cheat_parv3_advance_codes_if_not_equal,
        cheat_parv3_advance_codes_if_signed_lower,
        cheat_parv3_advance_codes_if_signed_greater,
        cheat_parv3_advance_codes_if_unsigned_lower,
        cheat_parv3_advance_codes_if_unsigned_greater,
        cheat_parv3_advance_codes_if_and,
        cheat_parv3_advance_codes_if_actions,
        cheat_parv3_advance_codes_if_blocks,
    };

    nb_tests = sizeof(tests) / sizeof(tests[0]);
    nb_tests_passed = 0;
    g_remaining_errors = 2;

    for (i = 0; i < nb_tests; ++i) {
        nb_tests_passed += tests[i]();
    }

    printf("\n");
    printf("Total: %u/%zu\n", nb_tests_passed, nb_tests);

    while (true) {
        VBlankIntrWait();
    }
}
