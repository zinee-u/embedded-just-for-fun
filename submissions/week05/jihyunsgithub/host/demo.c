#include "mission.h"
#include "fake_platform.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    static const uint32_t times_ms[] = {0U, 9U, 10U, 35U, 36U, 45U, 250U, 260U, 270U, 500U};
    static const bool enabled[] = {true, true, true, true, true, true, false, false, true, true};
    CollectorState state = {0};
    fake_platform_reset();
    if (!collector_init(&state, 100000000U, 0U)) {
        fputs("Initialization failed\n", stderr);
        return 1;
    }
    puts("time_ms,enabled,samples,last_value,led_on,delay_calls");
    for (size_t i = 0U; i < sizeof times_ms / sizeof times_ms[0]; ++i) {
        fake_set_enabled(enabled[i]);
        fake.sensor_value = (uint16_t)(100U + i);
        collector_step(&state, times_ms[i] * 100000U);
        printf("%" PRIu32 ",%u,%" PRIu32 ",%u,%u,%" PRIu32 "\n",
               times_ms[i], enabled[i] ? 1U : 0U, state.sample_count,
               (unsigned)state.last_value,
               (fake.output[PORT_LED] & LED_MASK) == 0U ? 1U : 0U,
               fake.delay_calls);
    }
    return 0;
}
