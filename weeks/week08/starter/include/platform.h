#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Host 설계 표기. O0/O5는 Datasheet의 출력 선택. 실제 Register 인코딩과 구분. */
typedef enum { PORT_GPIO = 0, PORT_ALT5 = 5 } OutputSelect;
typedef enum { PUSH_PULL, OPEN_DRAIN } OutputDrive;
typedef struct {
    uint8_t port, pin;
    OutputSelect output;
    OutputDrive drive;
} I2cPin;
typedef struct {
    uint8_t module;
    I2cPin sda;
    I2cPin scl;
} I2cPins;

/* 선택한 I2C module과 두 신호의 Port/Pin·출력 선택·구동 방식을 적용. */
bool bus_configure(I2cPins pins);

/* 과제용 Host API. address7에 R/W bit를 포함하지 않음.
 * bus_read 1회 = sub 전송 + repeated START + len Byte 읽기.
 * 실패하면 false. 실패한 읽기 결과 사용 금지. */
bool bus_read(uint8_t address7, uint8_t sub, uint8_t *dst, size_t len);
bool bus_write(uint8_t address7, uint8_t reg, uint8_t value);
/* 동기식 Host 출력. 유효한 value(0..4095)면 한 줄 전체 수용. */
bool uart_record(uint16_t seq, uint16_t value);
#endif
