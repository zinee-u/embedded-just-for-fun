#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int decode_rain(const uint8_t *bytes, size_t len);

typedef enum {
    WIPER_NO_COMMAND = -1,
    WIPER_OFF = 0,
    WIPER_ON = 1
} WiperCommand;

static int failures = 0;
static int command_count = 0;
static int send_count = 0;
static WiperCommand last_sent_command = WIPER_NO_COMMAND;


/* 일반적인 값 비교용 */
static void check_result(const char *name, int actual, int expected)
{
    if (actual == expected) {
        printf("PASS: %-20s expected=%d, actual=%d\n",
               name, expected, actual);
    } else {
        printf("FAIL: %-20s expected=%d, actual=%d\n",
               name, expected, actual);
        failures++;
    }
}


/* 성공한 센서값을 ON/OFF 명령으로 변환 */
static WiperCommand make_wiper_command(int rain)
{
    command_count++;

    if (rain >= 500) {
        return WIPER_ON;
    }

    return WIPER_OFF;
}


/* 실제 전송 대신 전송 횟수만 기록하는 mock 함수 */
static void send_wiper_command(WiperCommand command)
{
    last_sent_command = command;
    send_count++;
}


/*
 * 호출자 동작을 흉내 낸 함수
 *
 * decode_rain()이 실패하면 명령을 만들지도 않고,
 * 전송하지도 않는다.
 */
static void process_sensor_input(const uint8_t *bytes, size_t len)
{
    int rain = decode_rain(bytes, len);

    if (rain == -1) {
        return;
    }

    WiperCommand command = make_wiper_command(rain);
    send_wiper_command(command);
}


int main(void)
{
    /* 정상값과 경계값 */
    uint8_t case_0[2]    = {0x00, 0x00};
    uint8_t case_128[2]  = {0x80, 0x00};
    uint8_t case_255[2]  = {0xFF, 0x00};
    uint8_t case_499[2]  = {0xF3, 0x01};
    uint8_t case_500[2]  = {0xF4, 0x01};
    uint8_t case_1000[2] = {0xE8, 0x03};

    /* 오류값 */
    uint8_t case_1001[2]     = {0xE9, 0x03};
    uint8_t case_high_msb[2] = {0x00, 0x80};
    uint8_t case_max[2]      = {0xFF, 0xFF};

    /* 길이 검사용 실제 배열 */
    uint8_t one_byte[1] = {0xF4};
    uint8_t three_bytes[3] = {0xF4, 0x01, 0x00};


    printf("=== decode_rain value tests ===\n");

    check_result("value 0",
                 decode_rain(case_0, 2),
                 0);

    check_result("value 128",
                 decode_rain(case_128, 2),
                 128);

    check_result("value 255",
                 decode_rain(case_255, 2),
                 255);

    check_result("value 499",
                 decode_rain(case_499, 2),
                 499);

    check_result("value 500",
                 decode_rain(case_500, 2),
                 500);

    check_result("value 1000",
                 decode_rain(case_1000, 2),
                 1000);


    printf("\n=== invalid value tests ===\n");

    check_result("value 1001",
                 decode_rain(case_1001, 2),
                 -1);

    check_result("high byte MSB",
                 decode_rain(case_high_msb, 2),
                 -1);

    check_result("value 65535",
                 decode_rain(case_max, 2),
                 -1);


    printf("\n=== pointer / length tests ===\n");

    check_result("NULL input",
                 decode_rain(NULL, 2),
                 -1);

    check_result("length 0",
                 decode_rain(case_500, 0),
                 -1);

    check_result("length 1",
                 decode_rain(one_byte, 1),
                 -1);

    check_result("length 3",
                 decode_rain(three_bytes, 3),
                 -1);


    printf("\n=== input preservation test ===\n");

    {
        uint8_t input[2] = {0xF4, 0x01};

        uint8_t before0 = input[0];
        uint8_t before1 = input[1];

        (void)decode_rain(input, 2);

        if (input[0] == before0 &&
            input[1] == before1) {
            printf("PASS: input unchanged\n");
        } else {
            printf("FAIL: input changed\n");
            failures++;
        }
    }


    printf("\n=== command tests ===\n");

    check_result("499 -> OFF",
                 make_wiper_command(decode_rain(case_499, 2)),
                 WIPER_OFF);

    check_result("500 -> ON",
                 make_wiper_command(decode_rain(case_500, 2)),
                 WIPER_ON);


    printf("\n=== failure command/send test ===\n");

    /*
     * 이전 명령이 무엇이었는지와 관계없이
     * 오류 입력에서는 새 명령 생성/전송이 없어야 한다.
     */
    command_count = 0;
    send_count = 0;
    last_sent_command = WIPER_ON;

    process_sensor_input(case_1001, 2);

    check_result("failure command count",
                 command_count,
                 0);

    check_result("failure send count",
                 send_count,
                 0);

    check_result("previous command kept",
                 last_sent_command,
                 WIPER_ON);


    printf("\n=== valid command/send test ===\n");

    command_count = 0;
    send_count = 0;
    last_sent_command = WIPER_NO_COMMAND;

    process_sensor_input(case_499, 2);

    check_result("valid command count",
                 command_count,
                 1);

    check_result("valid send count",
                 send_count,
                 1);

    check_result("sent OFF",
                 last_sent_command,
                 WIPER_OFF);

    /*
     * 현재 상태가 이미 OFF였다고 가정해도
     * 정상 입력이면 새 명령을 만들 수 있어야 한다.
     */
    process_sensor_input(case_499, 2);

    check_result("second command count",
                 command_count,
                 2);

    check_result("second send count",
                 send_count,
                 2);


    printf("\n=============================\n");
    printf("Total failures: %d\n", failures);

    return failures == 0 ? 0 : 1;
}