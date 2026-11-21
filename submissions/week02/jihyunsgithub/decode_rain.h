#ifndef DECODE_RAIN_H
#define DECODE_RAIN_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    WIPER_OFF = 0,
    WIPER_ON = 1
} WiperCommand;

/*
 * 성공 시 1 반환: rain_out 및 command_out을 갱신한다.
 * 실패 시 0 반환: 두 출력값 모두 변경하지 않는다.
 * 입력 버퍼는 읽기만 하고 주소를 저장하지 않는다.
 * 모든 포인터는 NULL 또는 호출 중 해당 객체에 접근 가능한 유효 포인터여야 한다.
 */
int decode_rain(const uint8_t *bytes, size_t len,
                uint16_t *rain_out, WiperCommand *command_out);

#endif
