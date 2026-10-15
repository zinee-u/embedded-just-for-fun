#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RAIN_MAX        1000
#define RAIN_ON_THRESH  500
#define RAIN_FRAME_LEN  2

/*
성공: out_rain에 0~1000을 저장하고 true 반환
실패: out_rain을 변경하지 않고 false 반환

호출자는 bytes의 실제 읽기 가능한 길이를 len으로 전달한다.
out_rain은 쓰기 가능한 별도의 uint16_t 변수를 가리켜야 한다.
*/
bool decode_rain(const uint8_t* bytes,
                 size_t len,
                 uint16_t* out_rain)
{
    // 배열을 읽거나 결과를 저장하기 전에 검사
    if (bytes == NULL ||
        len != RAIN_FRAME_LEN ||
        out_rain == NULL) {
        return false;
    }

    // little-endian: 첫 바이트가 하위, 둘째 바이트가 상위
    uint16_t rain =
        (uint16_t)(bytes[0] | (bytes[1] << 8));

    // 센서의 유효 범위 검사
    if (rain > RAIN_MAX) {
        return false;
    }

    *out_rain = rain;

    return true;
}

void set_wiper(bool on)
{
    // 테스트용 출력
    printf("와이퍼 %s\n", on ? "ON" : "OFF");
}

void process_rain(const uint8_t* bytes, size_t len)
{
    uint16_t rain = 0;

    if (!decode_rain(bytes, len, &rain)) {
        return;
    }

    set_wiper(rain >= RAIN_ON_THRESH);
}