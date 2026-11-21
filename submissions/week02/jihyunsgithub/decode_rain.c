#include "decode_rain.h"

int decode_rain(const uint8_t *bytes, size_t len,
                uint16_t *rain_out, WiperCommand *command_out)
{
    uint16_t rain;

    /* 어떤 배열 원소도 읽기 전에 포인터와 길이를 확인한다. */
    if (bytes == NULL || rain_out == NULL || command_out == NULL || len != 2u) {
        return 0;
    }

    /* uint8_t의 부호 없는 바이트를 little-endian 16비트 값으로 조립한다. */
    rain = (uint16_t)((uint16_t)bytes[0] |
                      (uint16_t)((uint16_t)bytes[1] << 8));

    if (rain > 1000u) {
        return 0;
    }

    /* 모든 검사에 성공한 뒤 출력 객체를 갱신한다. */
    *rain_out = rain;
    *command_out = (rain >= 500u) ? WIPER_ON : WIPER_OFF;
    return 1;
}
