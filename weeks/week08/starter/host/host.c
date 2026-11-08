#include "host.h"
#include <stdio.h>
#include <string.h>

enum { TRACE_CAPACITY = 32 };
static BusEvent trace[TRACE_CAPACITY];
static uint8_t registers[128];
static size_t calls, fail_at, records;
static char frame[24];
static I2cPins last_pins;
static bool bus_connected;

bool bus_configure(I2cPins pins)
{
    last_pins = pins;
    /* 실습 회로의 물리 배선. 일치해야 LIS3DH가 응답. */
    bus_connected = pins.module == 0 &&
                    pins.sda.port == 2 && pins.sda.pin == 4 &&
                    pins.scl.port == 2 && pins.scl.pin == 5 &&
                    pins.sda.output == PORT_ALT5 && pins.scl.output == PORT_ALT5 &&
                    pins.sda.drive == OPEN_DRAIN && pins.scl.drive == OPEN_DRAIN;
    return bus_connected;
}
I2cPins host_last_pins(void) { return last_pins; }

void host_reset(void)
{
    bus_connected = false;
    last_pins = (I2cPins){0};
    memset(registers, 0, sizeof registers);
    registers[0x0F] = 0x33;
    calls = fail_at = records = 0;
    frame[0] = '\0';
}
void host_set_id(uint8_t id) { registers[0x0F] = id; }
void host_set_x(uint8_t low, uint8_t high)
{
    registers[0x28] = low;
    registers[0x29] = high;
}
void host_fail_on(size_t index) { fail_at = index; }
size_t host_bus_count(void) { return calls; }
BusEvent host_event(size_t index)
{
    if (index == 0 || index > calls || index > TRACE_CAPACITY) {
        return (BusEvent){0};
    }
    return trace[index - 1];
}
size_t host_uart_count(void) { return records; }
const char *host_last_frame(void) { return frame; }

static bool log_event(BusEvent event)
{
    ++calls;
    event.ok = bus_connected && event.ok && event.address7 == 0x18 && calls != fail_at;
    if (calls <= TRACE_CAPACITY) {
        trace[calls - 1] = event;
    }
    return event.ok;
}
bool bus_write(uint8_t address7, uint8_t reg, uint8_t value)
{
    bool valid = reg == 0x23 || reg == 0x20;
    if (!log_event((BusEvent){'W', address7, reg, value, 1, valid})) {
        return false;
    }
    registers[reg] = value;
    return true;
}
bool bus_read(uint8_t address7, uint8_t sub, uint8_t *dst, size_t len)
{
    uint8_t reg = sub & 0x7Fu;
    bool advance = (sub & 0x80u) != 0;
    bool valid = dst != NULL && ((reg == 0x0F && len == 1) ||
                                 (reg == 0x28 && len == 2));
    if (!log_event((BusEvent){'R', address7, sub, 0, len, valid})) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        dst[i] = registers[reg];
        if (advance) { ++reg; }
    }
    return true;
}
bool uart_record(uint16_t seq, uint16_t value)
{
    if (value > 4095) { return false; }
    /* 최대 15 Byte + C 문자열 끝. 부분 수용 없는 Host UART. */
    (void)snprintf(frame, sizeof frame, "S,%u,%u,0\n",
                   (unsigned)seq, (unsigned)value);
    ++records;
    return true;
}
