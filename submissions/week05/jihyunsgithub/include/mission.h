#ifndef MISSION_H
#define MISSION_H

#include <stdbool.h>
#include <stdint.h>

/* Fixed public API. STM frequency is an input, not the CPU frequency. */
bool ticks_from_ms(uint32_t hz, uint32_t ms, uint32_t *out);
bool period_due(uint32_t now, uint32_t period, uint32_t *last);
void board_io_init(void);
bool board_collection_enabled(void);
void board_led_toggle(void);

typedef struct {
    bool ready;
    uint32_t sample_period;
    uint32_t led_period;
    uint32_t last_sample;
    uint32_t last_led;
    uint32_t sample_count;
    uint16_t last_value;
} CollectorState;

bool collector_init(CollectorState *state, uint32_t stm_hz, uint32_t now);
void collector_step(CollectorState *state, uint32_t now);

#endif
