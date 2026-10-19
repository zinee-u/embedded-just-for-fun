#include "mission.h"
#include "fake_platform.h"
#include "test_support.h"
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

static unsigned checks;
static unsigned failures;

void test_check(bool result, const char *expression, const char *file, int line)
{
    ++checks;
    if (!result) {
        ++failures;
        fprintf(stderr, "FAIL %s:%d: %s\n", file, line, expression);
    }
}

static void conversion(uint32_t hz, uint32_t ms, bool valid, uint32_t expected)
{
    uint32_t out = UINT32_C(0x13579bdf);
    CHECK(ticks_from_ms(hz, ms, &out) == valid);
    CHECK(out == (valid ? expected : UINT32_C(0x13579bdf)));
}

static void test_timebase(void)
{
    conversion(100000000U, 10U, true, 1000000U);
    conversion(100000000U, 250U, true, 25000000U);
    conversion(200000000U, 10U, true, 2000000U);
    conversion(200000000U, 250U, true, 50000000U);
    conversion(500U, 2U, true, 1U);
    conversion(1000U, 1U, true, 1U);
    conversion((uint32_t)INT32_MAX, 1000U, true, (uint32_t)INT32_MAX);
    conversion(1000U, (uint32_t)INT32_MAX, true, (uint32_t)INT32_MAX);
    conversion(0U, 10U, false, 0U);
    conversion(100000000U, 0U, false, 0U);
    conversion(1U, 1U, false, 0U);
    conversion(1500U, 1U, false, 0U);
    conversion(1001U, 10U, false, 0U);
    conversion(1000U, (uint32_t)INT32_MAX + 1U, false, 0U);
    conversion(UINT32_MAX, 1000U, false, 0U);
    conversion(UINT32_MAX, UINT32_MAX, false, 0U);
    CHECK(!ticks_from_ms(100000000U, 10U, NULL));

    uint32_t last = 100U;
    CHECK(!period_due(109U, 10U, &last));
    CHECK(last == 100U);
    CHECK(period_due(110U, 10U, &last));
    CHECK(last == 110U);
    CHECK(!period_due(110U, 10U, &last));
    CHECK(last == 110U);
    last = 100U;
    CHECK(period_due(135U, 10U, &last));
    CHECK(last == 135U);
    CHECK(!period_due(136U, 10U, &last));
    CHECK(last == 135U);
    CHECK(period_due(145U, 10U, &last));
    CHECK(last == 145U);

    last = UINT32_MAX - 4U;
    CHECK(!period_due(UINT32_MAX - 2U, 10U, &last));
    CHECK(last == UINT32_MAX - 4U);
    CHECK(!period_due(4U, 10U, &last));
    CHECK(last == UINT32_MAX - 4U);
    CHECK(period_due(5U, 10U, &last));
    CHECK(last == 5U);
    last = 10U;
    CHECK(!period_due(20U, 0U, &last));
    CHECK(last == 10U);
    CHECK(!period_due(20U, (uint32_t)INT32_MAX + 1U, &last));
    CHECK(last == 10U);
    CHECK(!period_due(20U, UINT32_MAX, &last));
    CHECK(last == 10U);
    CHECK(!period_due(20U, 10U, NULL));
    last = 0U;
    CHECK(period_due((uint32_t)INT32_MAX, (uint32_t)INT32_MAX, &last));
    CHECK(last == (uint32_t)INT32_MAX);
    /* Actual elapsed time may exceed INT32_MAX, but is below one full wrap. */
    last = 100U;
    CHECK(period_due(UINT32_C(2147483748), 10U, &last));
    CHECK(last == UINT32_C(2147483748));
}

static void test_board(void)
{
    fake_platform_reset();
    uint32_t led_output = fake.output[PORT_LED];
    uint32_t other_output = fake.output[PORT_ENABLE];
    uint32_t led_direction = fake.direction[PORT_LED];
    uint32_t other_direction = fake.direction[PORT_ENABLE];
    uint32_t other_pull = fake.pullup[PORT_ENABLE];
    board_io_init();
    CHECK(fake.event_count == 3U);
    CHECK(fake.events[0].kind == EVENT_WRITE);
    CHECK(fake.events[0].port == PORT_LED);
    CHECK(fake.events[0].mask == LED_MASK);
    CHECK(fake.events[1].kind == EVENT_OUTPUT);
    CHECK(fake.events[2].kind == EVENT_INPUT);
    CHECK(fake.led_was_off_at_output);
    CHECK(fake.output[PORT_LED] == (led_output | LED_MASK));
    CHECK(fake.output[PORT_ENABLE] == other_output);
    CHECK(fake.direction[PORT_LED] == (led_direction | LED_MASK));
    CHECK(fake.direction[PORT_ENABLE] == (other_direction & ~ENABLE_MASK));
    CHECK(fake.pullup[PORT_ENABLE] == (other_pull | ENABLE_MASK));

    fake_set_enabled(false);
    fake_platform_clear_log();
    CHECK(!board_collection_enabled());
    CHECK(fake.read_calls == 1U);
    CHECK(fake.events[0].port == PORT_ENABLE);
    CHECK(fake.events[0].mask == ENABLE_MASK);
    fake_set_enabled(true);
    CHECK(board_collection_enabled());
    CHECK(fake.read_calls == 2U);

    /* Both initial LED levels, with unrelated bits set and cleared. */
    const uint32_t images[] = {0U, UINT32_MAX, 0x12345678U, 0x87654321U};
    for (size_t i = 0U; i < sizeof images / sizeof images[0]; ++i) {
        fake.output[PORT_LED] = images[i];
        fake_platform_clear_log();
        board_led_toggle();
        CHECK(fake.output[PORT_LED] == (images[i] ^ LED_MASK));
        CHECK(fake.output[PORT_ENABLE] == other_output);
        CHECK(fake.toggle_calls == 1U);
        CHECK(fake.event_count == 1U);
        CHECK(fake.events[0].mask == LED_MASK);
        CHECK(fake.delay_calls == 0U);
    }
}

static CollectorState stale_state(void)
{
    return (CollectorState){true, 11U, 22U, 33U, 44U, 55U, 66U};
}

static void check_zero_state(const CollectorState *state)
{
    CHECK(!state->ready);
    CHECK(state->sample_period == 0U);
    CHECK(state->led_period == 0U);
    CHECK(state->last_sample == 0U);
    CHECK(state->last_led == 0U);
    CHECK(state->sample_count == 0U);
    CHECK(state->last_value == 0U);
}

static void test_init(void)
{
    /* Generic Host arithmetic inputs, not TC387QP clock configuration limits. */
    const uint32_t frequencies[] = {100000000U, 200000000U, 1000U};
    for (size_t i = 0U; i < sizeof frequencies / sizeof frequencies[0]; ++i) {
        CollectorState state = stale_state();
        fake_platform_reset();
        CHECK(collector_init(&state, frequencies[i], 123U));
        CHECK(state.ready);
        CHECK(state.sample_period == frequencies[i] / 100U);
        CHECK(state.led_period == frequencies[i] / 4U);
        CHECK(state.last_sample == 123U);
        CHECK(state.last_led == 123U);
        CHECK(state.sample_count == 0U);
        CHECK(state.last_value == 0U);
        CHECK(fake.event_count == 3U);
        CHECK(fake.led_was_off_at_output);
    }
    const uint32_t invalid[] = {0U, 1U, 1001U};
    for (size_t i = 0U; i < sizeof invalid / sizeof invalid[0]; ++i) {
        CollectorState state = stale_state();
        fake_platform_reset();
        CHECK(!collector_init(&state, invalid[i], 456U));
        check_zero_state(&state);
        CHECK(fake.event_count == 0U);
        fake_set_enabled(true);
        collector_step(&state, 1000U);
        CHECK(fake.event_count == 0U);
        check_zero_state(&state);
    }
    fake_platform_reset();
    CHECK(!collector_init(NULL, 100000000U, 0U));
    collector_step(NULL, 1000U);
    CHECK(fake.event_count == 0U);

    CollectorState state = stale_state();
    state.ready = false;
    collector_step(&state, 1000U);
    CHECK(fake.event_count == 0U);
    CHECK(state.sample_count == 55U);
    CHECK(state.last_sample == 33U);
}

static CollectorState ready_state(uint32_t start)
{
    /* Isolates scheduler checks from bugs in collector_init: 1 tick = 1 ms. */
    return (CollectorState){true, 10U, 250U, start, start, 0U, 0U};
}

static void step(CollectorState *state, uint32_t now, uint32_t count,
                 uint32_t toggles, uint32_t sensor_reads)
{
    fake_platform_clear_log();
    collector_step(state, now);
    CHECK(state->sample_count == count);
    CHECK(fake.toggle_calls == toggles);
    CHECK(fake.sensor_calls == sensor_reads);
    CHECK(fake.read_calls == 1U);
    CHECK(fake.delay_calls == 0U);
}

static void test_schedule(void)
{
    CollectorState state = ready_state(0U);
    fake_platform_reset();
    fake_set_enabled(true);
    step(&state, 0U, 0U, 0U, 0U);
    step(&state, 9U, 0U, 0U, 0U);
    step(&state, 10U, 1U, 0U, 1U);
    CHECK(state.last_sample == 10U);
    CHECK(state.last_value == 1234U);
    fake.sensor_value = 4095U;
    step(&state, 35U, 2U, 0U, 1U);
    CHECK(state.last_sample == 35U);
    CHECK(state.last_value == 4095U);
    step(&state, 36U, 2U, 0U, 0U);
    step(&state, 44U, 2U, 0U, 0U);
    fake.sensor_value = 0U;
    step(&state, 45U, 3U, 0U, 1U);
    CHECK(state.last_value == 0U);
    step(&state, 249U, 4U, 0U, 1U);
    /* LED due even though the next sample is not due until 259 ms. */
    step(&state, 250U, 4U, 1U, 0U);
    CHECK(state.last_led == 250U);
    step(&state, 250U, 4U, 0U, 0U);
    step(&state, 500U, 5U, 1U, 1U);
    CHECK(fake.events[0].kind == EVENT_READ);
    CHECK(state.last_sample == 500U);
    CHECK(state.last_led == 500U);
}

static void test_disabled_and_wrap(void)
{
    CollectorState state = ready_state(0U);
    fake_platform_reset();
    fake_set_enabled(true);
    step(&state, 10U, 1U, 0U, 1U);
    fake_set_enabled(false);
    step(&state, 20U, 1U, 0U, 0U);
    CHECK(state.last_sample == 20U);
    CHECK(state.last_value == 1234U);
    step(&state, 250U, 1U, 1U, 0U);
    CHECK(state.last_sample == 250U);
    CHECK(state.last_led == 250U);
    fake_set_enabled(true);
    step(&state, 251U, 1U, 0U, 0U);
    step(&state, 260U, 2U, 0U, 1U);

    state = ready_state(UINT32_MAX - 4U);
    fake_platform_reset();
    fake_set_enabled(true);
    step(&state, UINT32_MAX - 2U, 0U, 0U, 0U);
    step(&state, 4U, 0U, 0U, 0U);
    step(&state, 5U, 1U, 0U, 1U);
    CHECK(state.last_sample == 5U);
    step(&state, 244U, 2U, 0U, 1U);
    step(&state, 245U, 2U, 1U, 0U);
    CHECK(state.last_led == 245U);
}

static void test_integration(void)
{
    CollectorState state = {0};
    fake_platform_reset();
    CHECK(collector_init(&state, 100000000U, 0U));
    fake_set_enabled(true);
    step(&state, 0U, 0U, 0U, 0U);
    step(&state, 999999U, 0U, 0U, 0U);
    step(&state, 1000000U, 1U, 0U, 1U);
    step(&state, 25000000U, 2U, 1U, 1U);
    CHECK((fake.output[PORT_LED] & LED_MASK) == 0U);
    CHECK(collector_init(&state, 200000000U, 100U));
    CHECK(state.sample_count == 0U);
    CHECK(state.last_value == 0U);
    CHECK((fake.output[PORT_LED] & LED_MASK) != 0U);
    step(&state, 2000099U, 0U, 0U, 0U);
    step(&state, 2000100U, 1U, 0U, 1U);
    fake_platform_clear_log();
    CHECK(!collector_init(&state, 0U, 777U));
    check_zero_state(&state);
    collector_step(&state, UINT32_MAX);
    CHECK(fake.event_count == 0U);
}

int main(void)
{
    test_timebase();
    test_board();
    test_init();
    test_schedule();
    test_disabled_and_wrap();
    test_integration();
    run_student_tests();
    printf("%u checks, %u failures\n", checks, failures);
    return failures == 0U ? 0 : 1;
}
