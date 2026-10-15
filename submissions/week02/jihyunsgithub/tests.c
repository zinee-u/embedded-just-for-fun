#include "decode_rain.h"

#include <stdio.h>
#include <string.h>

static int passed = 0;
static int failed = 0;
static int sent_commands = 0;
static WiperCommand last_sent = WIPER_OFF;

/* 실제 차량 전송이 아닌 호출자의 명령 생성/전송 모의 함수 */
static void send_command(WiperCommand command)
{
    ++sent_commands;
    last_sent = command;
}

/* 성공했을 때만 새 명령을 보낸다는 호출자 규칙 */
static int handle_input(const uint8_t *bytes, size_t len,
                        uint16_t *rain, WiperCommand *command)
{
    int ok = decode_rain(bytes, len, rain, command);
    if (ok) {
        send_command(*command);
    }
    return ok;
}

static void report(const char *name, int condition)
{
    if (condition) {
        ++passed;
        printf("PASS %s\n", name);
    } else {
        ++failed;
        printf("FAIL %s\n", name);
    }
}

static void test_value(const char *name, uint16_t value, int expected_ok,
                       WiperCommand expected_command)
{
    const uint8_t bytes[2] = {(uint8_t)(value & 0xFFu),
                              (uint8_t)(value >> 8)};
    const uint8_t original[2] = {bytes[0], bytes[1]};
    uint16_t rain = 4321u;
    WiperCommand command = WIPER_ON;
    int before = sent_commands;
    int ok = handle_input(bytes, sizeof bytes, &rain, &command);
    int output_ok = expected_ok
        ? (rain == value && command == expected_command)
        : (rain == 4321u && command == WIPER_ON);
    int sent_ok = sent_commands == before + (expected_ok ? 1 : 0);
    int sent_value_ok = !expected_ok || last_sent == expected_command;
    report(name, ok == expected_ok && output_ok && sent_ok &&
                 sent_value_ok && memcmp(bytes, original, sizeof bytes) == 0);
}

static void test_invalid(const char *name, const uint8_t *bytes, size_t len,
                         uint16_t *rain_ptr, WiperCommand *command_ptr)
{
    int before = sent_commands;
    uint16_t old_rain = rain_ptr ? *rain_ptr : 0u;
    WiperCommand old_command = command_ptr ? *command_ptr : WIPER_OFF;
    int ok = handle_input(bytes, len, rain_ptr, command_ptr);
    report(name, !ok && sent_commands == before &&
                 (!rain_ptr || *rain_ptr == old_rain) &&
                 (!command_ptr || *command_ptr == old_command));
}

int main(void)
{
    const uint8_t one_byte[1] = {0xF4};
    const uint8_t three_bytes[3] = {0xF4, 0x01, 0x00};
    const uint8_t valid[2] = {0xF4, 0x01};
    uint16_t rain = 123u;
    WiperCommand command = WIPER_OFF;

    test_value("value 0 -> OFF", 0u, 1, WIPER_OFF);
    test_value("value 128 -> OFF", 128u, 1, WIPER_OFF);
    test_value("value 255 -> OFF", 255u, 1, WIPER_OFF);
    test_value("value 300 -> OFF", 300u, 1, WIPER_OFF);
    test_value("value 499 -> OFF", 499u, 1, WIPER_OFF);
    test_value("value 500 -> ON", 500u, 1, WIPER_ON);
    test_value("value 1000 -> ON", 1000u, 1, WIPER_ON);
    test_value("value 1001 -> reject", 1001u, 0, WIPER_OFF);
    test_value("high-byte bit7 set 0x8000 -> reject", 0x8000u, 0, WIPER_OFF);
    test_value("uint16 max 65535 -> reject", UINT16_MAX, 0, WIPER_OFF);

    test_invalid("NULL input", NULL, 2u, &rain, &command);
    test_invalid("length 0 with valid address", valid, 0u, &rain, &command);
    test_invalid("length 1 with 1-byte array", one_byte, sizeof one_byte, &rain, &command);
    test_invalid("length 3 with 3-byte array", three_bytes, sizeof three_bytes, &rain, &command);
    test_invalid("NULL rain output", valid, sizeof valid, NULL, &command);
    test_invalid("NULL command output", valid, sizeof valid, &rain, NULL);

    /* 기존 상태가 OFF인데 정상값도 OFF여도 새 명령 1개를 생성해야 한다. */
    {
        const uint8_t zero[2] = {0x00, 0x00};
        int before = sent_commands;
        command = WIPER_OFF;
        report("same OFF state still sends a new command",
               handle_input(zero, sizeof zero, &rain, &command) == 1 &&
               rain == 0u && command == WIPER_OFF &&
               sent_commands == before + 1 && last_sent == WIPER_OFF);
    }

    printf("SUMMARY: %d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
