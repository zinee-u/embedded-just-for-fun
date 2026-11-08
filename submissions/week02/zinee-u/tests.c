#include "decode_rain.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t checks = 0;
static size_t failures = 0;

static void check_equal(const char *name, const char *property,
                        int actual, int expected)
{
    ++checks;
    if (actual == expected) {
        return;
    }

    ++failures;
    if (failures <= 20u) {
        fprintf(stderr, "FAIL [%s] %s: got %d, expected %d\n",
                name, property, actual, expected);
    }
}

/* actual_size is the real array size, separate from the API's len argument. */
static enum wiper_command run_case(const char *name, const uint8_t *bytes,
                                   size_t len, size_t actual_size,
                                   int expected_value)
{
    uint8_t before[3] = {0u, 0u, 0u};
    const enum wiper_command expected_command =
        expected_value < 0 ? WIPER_NO_COMMAND :
        (expected_value >= 500 ? WIPER_ON : WIPER_OFF);

    if (bytes != NULL) {
        memcpy(before, bytes, actual_size);
    }

    check_equal(name, "decoded value", decode_rain(bytes, len), expected_value);
    const enum wiper_command command = make_wiper_command(bytes, len);
    check_equal(name, "command", (int)command, (int)expected_command);

    /* Exercise the real helper from both possible previous actuator states. */
    const enum wiper_command previous_states[2] = {WIPER_OFF, WIPER_ON};
    for (size_t i = 0; i < 2u; ++i) {
        enum wiper_command state = previous_states[i];
        int commands_sent = 0;
        const enum wiper_command next = make_wiper_command(bytes, len);

        if (next != WIPER_NO_COMMAND) {
            state = next;
            ++commands_sent;
        }

        const enum wiper_command expected_state =
            expected_value < 0 ? previous_states[i] : expected_command;
        const char *const state_property =
            i == 0u ? "state from OFF" : "state from ON";
        const char *const count_property =
            i == 0u ? "commands from OFF" : "commands from ON";
        check_equal(name, state_property, (int)state, (int)expected_state);
        check_equal(name, count_property, commands_sent,
                    expected_value < 0 ? 0 : 1);
    }

    if (bytes != NULL) {
        check_equal(name, "input unchanged",
                    memcmp(before, bytes, actual_size) == 0 ? 1 : 0, 1);
    }
    return command;
}

static size_t run_named_cases(void)
{
    const struct {
        const char *name;
        uint8_t bytes[2];
        int expected;
    } cases[] = {
        {"normal / 1",             {0x01u, 0x00u},    1},
        {"normal / 127",           {0x7fu, 0x00u},  127},
        {"normal / low-byte sign", {0x80u, 0x00u},  128},
        {"normal / 255",           {0xffu, 0x00u},  255},
        {"normal / byte carry",    {0x00u, 0x01u},  256},
        {"normal / 750",           {0xeeu, 0x02u},  750},
        {"boundary / minimum",     {0x00u, 0x00u},    0},
        {"boundary / below ON",    {0xf3u, 0x01u},  499},
        {"boundary / exactly ON",  {0xf4u, 0x01u},  500},
        {"boundary / above ON",    {0xf5u, 0x01u},  501},
        {"boundary / maximum",     {0xe8u, 0x03u}, 1000},
        {"error / above maximum",  {0xe9u, 0x03u},   -1},
        {"error / high-byte sign", {0x00u, 0x80u},   -1},
        {"error / raw maximum",    {0xffu, 0xffu},   -1},
        {"error / reversed 500",   {0x01u, 0xf4u},   -1}
    };

    size_t named_count = sizeof cases / sizeof cases[0];
    for (size_t i = 0; i < named_count; ++i) {
        (void)run_case(cases[i].name, cases[i].bytes, sizeof cases[i].bytes,
                       sizeof cases[i].bytes, cases[i].expected);
    }

    const uint8_t valid[2] = {0xf4u, 0x01u};
    /* This really has one element: an accidental bytes[1] read is detectable. */
    const uint8_t short_buffer[1] = {0xf4u};
    const uint8_t long_buffer[3] = {0xf4u, 0x01u, 0xa5u};
    (void)run_case("error / zero length", valid, 0u, sizeof valid, -1);
    (void)run_case("error / one-byte buffer", short_buffer, sizeof short_buffer,
                   sizeof short_buffer, -1);
    (void)run_case("error / three-byte buffer", long_buffer, sizeof long_buffer,
                   sizeof long_buffer, -1);
    (void)run_case("error / NULL with length 2", NULL, 2u, 0u, -1);
    (void)run_case("error / NULL with length 0", NULL, 0u, 0u, -1);
    named_count += 5u;
    return named_count;
}

static size_t run_all_byte_pairs(void)
{
    size_t pairs = 0;
    size_t off_count = 0;
    size_t on_count = 0;
    size_t rejected_count = 0;
    size_t other_count = 0;

    for (unsigned int high = 0u; high < 256u; ++high) {
        for (unsigned int low = 0u; low < 256u; ++low) {
            const uint8_t bytes[2] = {(uint8_t)low, (uint8_t)high};
            /* Independent oracle: decimal place weights, no shift or OR. */
            const uint32_t raw = (uint32_t)low + (uint32_t)high * 256u;
            const int expected = raw <= 1000u ? (int)raw : -1;
            char name[64];
            (void)snprintf(name, sizeof name, "all pairs / low=%u high=%u",
                           low, high);
            const enum wiper_command command =
                run_case(name, bytes, sizeof bytes, sizeof bytes, expected);

            switch (command) {
            case WIPER_OFF:
                ++off_count;
                break;
            case WIPER_ON:
                ++on_count;
                break;
            case WIPER_NO_COMMAND:
                ++rejected_count;
                break;
            default:
                ++other_count;
                break;
            }
            ++pairs;
        }
    }

    check_equal("all pairs", "OFF count", off_count == 500u ? 1 : 0, 1);
    check_equal("all pairs", "ON count", on_count == 501u ? 1 : 0, 1);
    check_equal("all pairs", "rejected count", rejected_count == 64535u ? 1 : 0, 1);
    check_equal("all pairs", "unknown commands", other_count == 0u ? 1 : 0, 1);
    printf("All pairs: %zu; OFF=%zu (expected 500), ON=%zu (expected 501), "
           "NO_COMMAND=%zu (expected 64535), other=%zu (expected 0)\n",
           pairs, off_count, on_count, rejected_count, other_count);
    return pairs;
}

int main(void)
{
    printf("Host: CHAR_BIT=%d, CHAR_MIN=%d, sizeof(int)=%zu, "
           "sizeof(void *)=%zu, sizeof(uint8_t)=%zu, sizeof(uint16_t)=%zu, "
           "sizeof(uint32_t)=%zu\n",
           CHAR_BIT, CHAR_MIN, sizeof(int), sizeof(void *),
           sizeof(uint8_t), sizeof(uint16_t), sizeof(uint32_t));
    puts("Host results are not measurements of the assignment's 32-bit MCU.");

    const size_t named_cases = run_named_cases();
    const size_t byte_pairs = run_all_byte_pairs();
    printf("%s: named cases=%zu, byte pairs=%zu, checks=%zu, failures=%zu\n",
           failures == 0u ? "PASS" : "FAIL", named_cases, byte_pairs,
           checks, failures);
    if (failures > 20u) {
        fprintf(stderr, "Only the first 20 failures were printed.\n");
    }
    return failures == 0u ? EXIT_SUCCESS : EXIT_FAILURE;
}
