#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
rain.c에 구현된 함수
*/
bool decode_rain(const uint8_t* bytes,
                 size_t len,
                 uint16_t* out_rain);

void process_rain(const uint8_t* bytes, size_t len);

int main(void)
{
    // little-endian: {하위 바이트, 상위 바이트}
    const uint8_t frames[][2] = {
        {0x00, 0x00},  // 0
        {0x80, 0x00},  // 128
        {0xF3, 0x01},  // 499
        {0xF4, 0x01},  // 500
        {0xE8, 0x03},  // 1000
        {0xE9, 0x03},  // 1001: 범위 초과
        {0xFF, 0xFF}   // 65535: 범위 초과
    };

    const unsigned int values[] = {
        0, 128, 499, 500, 1000, 1001, 65535
    };

    /*
     * 1. 정상값 / 경계값 / 범위 초과 테스트
     */
    size_t count = sizeof(frames) / sizeof(frames[0]);

    for (size_t i = 0; i < count; ++i) {
        printf("[센서값 %u]\n", values[i]);

        process_rain(frames[i], sizeof(frames[i]));
    }

    /*
     * 2. NULL 및 잘못된 길이 테스트
     */
    const uint8_t short_frame[] = {0xF4};
    const uint8_t long_frame[] = {0xF4, 0x01, 0x00};

    printf("[NULL 입력]\n");
    process_rain(NULL, 2);

    printf("[길이 0]\n");
    process_rain(short_frame, 0);

    printf("[길이 1]\n");
    process_rain(short_frame, sizeof(short_frame));

    printf("[길이 3]\n");
    process_rain(long_frame, sizeof(long_frame));

    /*
     * 3. 실패했을 때 기존 결과값 유지 확인
     */
    uint16_t saved_rain = 123;

    bool ok = decode_rain(
        frames[5],              // 1001: 실패해야 함
        sizeof(frames[5]),
        &saved_rain
    );

    printf(
        "[실패 시 값 보존] 성공=%d, 저장값=%u\n",
        ok,
        (unsigned int)saved_rain
    );

    /*
     * 4. 출력 포인터가 NULL인 경우
     */
    ok = decode_rain(
        frames[3],
        sizeof(frames[3]),
        NULL
    );

    printf("[NULL 출력 포인터] 성공=%d\n", ok);

    return 0;
}