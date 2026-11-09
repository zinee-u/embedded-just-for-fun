#include <stdio.h>
#include <stdint.h>

uint8_t enable(uint8_t ctrl);
uint8_t disable(uint8_t ctrl);
uint8_t set_mode(uint8_t ctrl, uint8_t mode);
uint8_t is_ready(uint8_t status);

static int failures = 0;

static void check_hex(const char *name, uint8_t actual, uint8_t expected)
{
    if (actual == expected) {
        printf("PASS: %-20s expected=0x%02X, actual=0x%02X\n",
               name, expected, actual);
    } else {
        printf("FAIL: %-20s expected=0x%02X, actual=0x%02X\n",
               name, expected, actual);
        failures++;
    }
}

static void check_value(const char *name, uint8_t actual, uint8_t expected)
{
    if (actual == expected) {
        printf("PASS: %-20s expected=%u, actual=%u\n",
               name, expected, actual);
    } else {
        printf("FAIL: %-20s expected=%u, actual=%u\n",
               name, expected, actual);
        failures++;
    }
}

int main(void)
{
    check_hex("enable 0xA8",
              enable(0xA8u),
              0xA9u);

    check_hex("enable 0xA9",
              enable(0xA9u),
              0xA9u);

    check_hex("set_mode 0xAB,2",
              set_mode(0xABu, 2u),
              0xADu);

    check_hex("set_mode 0xAF,2",
              set_mode(0xAFu, 2u),
              0xADu);

    check_hex("disable 0xAD",
              disable(0xADu),
              0xACu);

    check_value("ready 0x09",
                is_ready(0x09u),
                1u);

    check_value("ready 0x01",
                is_ready(0x01u),
                0u);

    printf("\nTotal failures: %d\n", failures);

    return failures == 0 ? 0 : 1;
}