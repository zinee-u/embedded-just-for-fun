#include "mission.h"
#include "fake_hal.h"
#include <assert.h>

/* fake_hal_reset() 직후 GPIOA 출력은 0xFFFF(모든 bit HIGH)다.
   LED2(PA5)만 바뀌고 나머지 bit는 1로 남아야 한다. */
#define OUTPUT_LED_OFF (0xFFFFU & ~(uint32_t)GPIO_PIN_5)
#define OUTPUT_LED_ON  0xFFFFU

static bool led_is_on(void)
{
    return (fake_hal.gpioa_output & GPIO_PIN_5) != 0U;
}

/* S1. LED가 켜진 상태에서 다시 초기화하면 OFF로 돌아가는가
   요구사항: LED 초기 상태 OFF, 순서 GPIOA Clock Enable -> OFF 출력 -> GPIO 설정.
   입력: board_led_init() -> board_led_toggle()로 LED ON -> board_led_init() 재호출.
         같은 상황을 app_init() 재호출로도 확인한다.
   예상: 재초기화 뒤 PA5만 0(OFF), 다른 bit는 1 유지. 재초기화 호출 순서도
         Clock Enable -> WritePin(RESET) -> GPIO_Init 이며 protocol error 없음.
   이유: 제공 테스트는 LED가 이미 OFF인 상태에서만 초기화를 확인한다.
         초기 OFF가 "처음 한 번"이 아니라 초기화할 때마다 적용되는지는
         LED가 ON인 상태에서 시작해야 구분된다. */
static void test_reinit_turns_led_off(void)
{
    size_t before;

    /* LED Module 단독 */
    fake_hal_reset();
    board_led_init();
    board_led_toggle();
    assert(led_is_on());
    assert(fake_hal.gpioa_output == OUTPUT_LED_ON);

    before = fake_hal.trace_count;
    board_led_init();
    assert(!led_is_on());
    assert(fake_hal.gpioa_output == OUTPUT_LED_OFF);
    assert(fake_hal.trace_count == before + 3U);
    assert(fake_hal.trace[before] == EVENT_GPIO_CLOCK_ENABLE);
    assert(fake_hal.trace[before + 1U] == EVENT_GPIO_WRITE);
    assert(fake_hal.trace[before + 2U] == EVENT_GPIO_INIT);
    assert(fake_hal.write_state == GPIO_PIN_RESET);
    assert(fake_hal.protocol_errors == 0U);

    /* Application 경로: app_step으로 LED를 켠 뒤 app_init을 다시 호출 */
    fake_hal_reset();
    assert(app_init());
    app_step();
    assert(led_is_on());

    assert(app_init());
    assert(!led_is_on());
    assert(fake_hal.gpioa_output == OUTPUT_LED_OFF);
    assert(fake_hal.protocol_errors == 0U);
}

/* S2. app_step을 여러 번 호출해도 매번 Toggle 1회 + 250 ms인가
   요구사항: 초기화 성공 후 app_step 한 번은 LED Toggle -> HAL_Delay(250).
   입력: app_init() 성공 후 app_step() 4회.
   예상: 매 호출마다 toggle_count, delay_count가 1씩 증가하고 last_delay_ms == 250.
         LED는 ON/OFF를 번갈아 하며 짝수 번 뒤 OFF. 총 Delay 1000 ms.
         초기화 6개 호출 뒤 Toggle, Delay가 번갈아 8개 기록된다.
   이유: 제공 테스트는 2회까지만 확인한다. 더 길게 돌려 Delay 값이 누적되거나
         한 번에 여러 번 Toggle하는 실수, 순서가 Delay -> Toggle로 바뀌는 실수를
         매 호출 단위로 잡는다. */
static void test_repeated_steps(void)
{
    static const unsigned int steps = 4U;
    const size_t init_calls = 6U; /* HAL_Init, Osc, Clock, GPIO Clock, Write, Init */
    unsigned int i;

    fake_hal_reset();
    assert(app_init());
    assert(fake_hal.trace_count == init_calls);
    assert(!led_is_on());

    for (i = 1U; i <= steps; ++i) {
        app_step();
        assert(fake_hal.toggle_count == i);
        assert(fake_hal.delay_count == i);
        assert(fake_hal.last_delay_ms == 250U);
        assert(fake_hal.delay_total_ms == (uint64_t)i * 250U);
        /* 홀수 번째 Toggle 뒤 ON, 짝수 번째 뒤 OFF */
        assert(led_is_on() == ((i % 2U) == 1U));
    }

    assert(fake_hal.delay_total_ms == 1000U);
    assert(fake_hal.gpioa_output == OUTPUT_LED_OFF);
    assert(fake_hal.trace_count == init_calls + 2U * steps);
    for (i = 0U; i < steps; ++i) {
        assert(fake_hal.trace[init_calls + 2U * i] == EVENT_GPIO_TOGGLE);
        assert(fake_hal.trace[init_calls + 2U * i + 1U] == EVENT_DELAY);
    }
    assert(fake_hal.protocol_errors == 0U);
}

void run_student_tests(void)
{
    /* 제공 테스트가 남긴 app.c 상태에 의존하지 않도록 각 Case가 app_init()을 직접 호출한다. */
    test_reinit_turns_led_off();
    test_repeated_steps();
}
