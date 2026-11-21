#include "fake_platform.h"
#include <string.h>

FakePlatform fake;

static void record(FakeEventKind kind, PlatformPort port,
                   uint32_t mask, uint32_t value)
{
    if (fake.event_count < FAKE_EVENT_CAPACITY) {
        fake.events[fake.event_count++] = (FakeEvent){kind, port, mask, value};
    }
}

void fake_platform_reset(void)
{
    memset(&fake, 0, sizeof fake);
    fake.output[PORT_LED] = UINT32_C(0xa5a50000);
    fake.output[PORT_ENABLE] = UINT32_C(0x5a5a0000);
    fake.direction[PORT_LED] = UINT32_C(0x80000000);
    fake.direction[PORT_ENABLE] = UINT32_C(0x40000000);
    fake.pullup[PORT_LED] = UINT32_C(0x20000000);
    fake.pullup[PORT_ENABLE] = UINT32_C(0x10000000);
    fake.input[PORT_ENABLE] = ENABLE_MASK;
    fake.sensor_value = 1234U;
}

void fake_platform_clear_log(void)
{
    fake.read_calls = 0U;
    fake.sensor_calls = 0U;
    fake.toggle_calls = 0U;
    fake.delay_calls = 0U;
    fake.delay_ms = 0U;
    fake.event_count = 0U;
}

void fake_set_enabled(bool enabled)
{
    if (enabled) {
        fake.input[PORT_ENABLE] &= ~ENABLE_MASK;
    } else {
        fake.input[PORT_ENABLE] |= ENABLE_MASK;
    }
}

void platform_port_write(PlatformPort port, uint32_t set, uint32_t clear)
{
    fake.output[port] = (fake.output[port] & ~clear) | set;
    record(EVENT_WRITE, port, set | clear, fake.output[port]);
}

void platform_port_output_pushpull(PlatformPort port, uint32_t mask)
{
    if (port == PORT_LED && (mask & LED_MASK) != 0U) {
        fake.led_was_off_at_output = (fake.output[port] & LED_MASK) != 0U;
    }
    fake.direction[port] |= mask;
    record(EVENT_OUTPUT, port, mask, fake.direction[port]);
}

void platform_port_input(PlatformPort port, uint32_t mask, bool pullup)
{
    fake.direction[port] &= ~mask;
    if (pullup) {
        fake.pullup[port] |= mask;
    } else {
        fake.pullup[port] &= ~mask;
    }
    record(EVENT_INPUT, port, mask, pullup ? 1U : 0U);
}

bool platform_port_read(PlatformPort port, uint32_t mask)
{
    ++fake.read_calls;
    record(EVENT_READ, port, mask, fake.input[port] & mask);
    return (fake.input[port] & mask) != 0U;
}

void platform_port_toggle(PlatformPort port, uint32_t mask)
{
    fake.output[port] ^= mask;
    ++fake.toggle_calls;
    record(EVENT_TOGGLE, port, mask, fake.output[port]);
}

uint16_t platform_sensor_read(void)
{
    ++fake.sensor_calls;
    record(EVENT_SENSOR, PORT_LED, 0U, fake.sensor_value);
    return fake.sensor_value;
}

void platform_delay_ms(uint32_t ms)
{
    ++fake.delay_calls;
    fake.delay_ms += ms;
    record(EVENT_DELAY, PORT_LED, 0U, ms);
}
