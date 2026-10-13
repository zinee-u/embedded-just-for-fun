#include <stdint.h>
#include <stdio.h>
#include "register_control.h"

static unsigned checks = 0;
static unsigned failures = 0;

static void check(const char *name, unsigned actual, unsigned expected)
{
    ++checks;
    printf("%-34s expected=0x%02X actual=0x%02X %s\n", name, expected,
           actual, actual == expected ? "PASS" : "FAIL");
    if (actual != expected) ++failures;
}

int main(void)
{
    /* Required seven independent calls. Hex is used even for READY (0/1). */
    check("enable(0xA8)", enable(0xA8), 0xA9);
    check("enable(0xA9)", enable(0xA9), 0xA9);
    check("set_mode(0xAB, 2)", set_mode(0xAB, 2), 0xAD);
    check("set_mode(0xAF, 2)", set_mode(0xAF, 2), 0xAD);
    check("disable(0xAD)", disable(0xAD), 0xAC);
    check("is_ready(0x09)", is_ready(0x09), 0x01);
    check("is_ready(0x01)", is_ready(0x01), 0x00);

    /* Additional cases, including all MODE targets and preservation. */
    check("enable(0xFE)", enable(0xFE), 0xFF);
    check("disable(0xFE)", disable(0xFE), 0xFE);
    check("disable(0xFF)", disable(0xFF), 0xFE);
    check("set_mode(0xFF, 0)", set_mode(0xFF, 0), 0xF9);
    check("set_mode(0xFF, 1)", set_mode(0xFF, 1), 0xFB);
    check("set_mode(0xFF, 2)", set_mode(0xFF, 2), 0xFD);
    check("set_mode(0x00, 2)", set_mode(0x00, 2), 0x04);
    check("is_ready(0xFF)", is_ready(0xFF), 0x01);
    check("is_ready(0xF7)", is_ready(0xF7), 0x00);

    /* Exhaustive register inputs for guaranteed mode values. */
    for (unsigned ctrl = 0; ctrl <= UINT8_MAX; ++ctrl) {
        uint8_t c = (uint8_t)ctrl;
        uint8_t e = enable(c), d = disable(c);
        if ((e & UINT8_C(0x01)) != 1 || (e & UINT8_C(0xFE)) != (c & UINT8_C(0xFE))) ++failures;
        if ((d & UINT8_C(0x01)) != 0 || (d & UINT8_C(0xFE)) != (c & UINT8_C(0xFE))) ++failures;
        for (unsigned mode = 0; mode <= 2; ++mode) {
            uint8_t m = set_mode(c, (uint8_t)mode);
            if (((m >> 1) & UINT8_C(0x03)) != mode ||
                (m & UINT8_C(0xF9)) != (c & UINT8_C(0xF9))) ++failures;
        }
        if (is_ready(c) != ((c >> 3) & 1u)) ++failures;
    }
    ++checks;
    printf("%-34s %s\n", "exhaustive invariant checks (256)", failures ? "see failures above if any" : "PASS");
    printf("SUMMARY: %u named checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
