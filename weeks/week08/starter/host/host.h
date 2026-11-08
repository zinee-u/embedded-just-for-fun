#ifndef HOST_H
#define HOST_H
#include "platform.h"

typedef struct {
    char op;             /* R: 결합 읽기, W: 단일 Register 쓰기 */
    uint8_t address7, sub, value;
    size_t len;
    bool ok;
} BusEvent;

I2cPins host_last_pins(void); /* 마지막 설정 시도의 실제 인자 */
void host_reset(void);  /* 미설정 Bus, 기본 WHO_AM_I=0x33, 측정값 0, 기록 없음 */
void host_set_id(uint8_t id);
void host_set_x(uint8_t low, uint8_t high);
void host_fail_on(size_t index); /* 호출 순번은 1부터 시작. 0은 실패 주입 해제. */
size_t host_bus_count(void);
BusEvent host_event(size_t index); /* 기록 조회 순번도 1부터 시작 */
size_t host_uart_count(void);
const char *host_last_frame(void); /* 관찰용 C 문자열. LF까지만 실제 Record에 포함. */
#endif
