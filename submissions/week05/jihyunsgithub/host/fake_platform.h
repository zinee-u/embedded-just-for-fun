#ifndef FAKE_PLATFORM_H
#define FAKE_PLATFORM_H

#include "platform.h"
#include <stddef.h>

typedef enum {
    EVENT_WRITE, EVENT_OUTPUT, EVENT_INPUT, EVENT_READ,
    EVENT_TOGGLE, EVENT_SENSOR, EVENT_DELAY
} FakeEventKind;
typedef struct {
    FakeEventKind kind;
    PlatformPort port;
    uint32_t mask;
    uint32_t value;
} FakeEvent;
#define FAKE_EVENT_CAPACITY 256U
typedef struct {
    uint32_t output[PORT_COUNT];
    uint32_t direction[PORT_COUNT]; /* 1 = output */
    uint32_t pullup[PORT_COUNT];
    uint32_t input[PORT_COUNT];    /* externally driven input level */
    uint16_t sensor_value;
    uint32_t read_calls;
    uint32_t sensor_calls;
    uint32_t toggle_calls;
    uint32_t delay_calls;
    uint32_t delay_ms;
    bool led_was_off_at_output;
    FakeEvent events[FAKE_EVENT_CAPACITY];
    size_t event_count;
} FakePlatform;

extern FakePlatform fake;
void fake_platform_reset(void);
void fake_platform_clear_log(void); /* Retains port images and input values. */
void fake_set_enabled(bool enabled);

#endif
