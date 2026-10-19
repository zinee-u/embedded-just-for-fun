
#include "mission.h"
#include "platform.h"
#include <stddef.h>

/*
 * I1: 전달받은 STM Clock 사용
 * I2: 초기화할 때 모든 상태 Field 초기화
 * I3: 두 주기 계산 성공 후에만 GPIO 초기화
 */
bool collector_init(CollectorState *state, uint32_t stm_hz, uint32_t now)
{
    if (state == NULL) {
        return false;
    }

    // I2: 이전 실행 상태를 완전히 초기화
    state->ready = false;
    state->sample_period = 0U;
    state->led_period = 0U;
    state->last_sample = 0U;
    state->last_led = 0U;
    state->sample_count = 0U;
    state->last_value = 0U;

    // I1: 고정된 100MHz 대신 실제 stm_hz 사용
    uint32_t sample_ticks = 0U;
    uint32_t led_ticks = 0U;

    // I3: 두 주기 계산이 모두 성공해야 초기화 진행
    if (!ticks_from_ms(stm_hz, 10U, &sample_ticks) ||
        !ticks_from_ms(stm_hz, 250U, &led_ticks)) {
        return false;
    }

    state->sample_period = sample_ticks;
    state->led_period = led_ticks;

    state->last_sample = now;
    state->last_led = now;

    board_io_init();

    state->ready = true;

    return true;
}

/*
 * C1: ready 검사
 * C2: ENABLE 비활성 시에도 수집 기준 시각 갱신
 * C3: LED 주기 독립 처리
 * C4: 센서 읽기 횟수 누적
 * C5: Delay 없이 반환
 */
void collector_step(CollectorState *state, uint32_t now)
{
    if (state == NULL || !state->ready) {
        return;
    }

    // 매 Step마다 ENABLE 입력을 정확히 한 번 확인
    bool enabled = board_collection_enabled();

    // ENABLE과 무관하게 수집 주기의 기준 시각 갱신
    if (period_due(now, state->sample_period, &state->last_sample)) {
        if (enabled) {
            state->last_value = platform_sensor_read();
            state->sample_count += 1U;
        }
    }

    // LED는 ENABLE 및 센서 수집과 독립적으로 동작
    if (period_due(now, state->led_period, &state->last_led)) {
        board_led_toggle();
    }
}
