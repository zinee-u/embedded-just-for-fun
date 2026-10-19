#include "mission.h"
#include <stddef.h>

/*
 * Milliseconds -> STM ticks
 *
 * T1: 32-bit 곱셈 Overflow 방지
 * T2: 정수 Tick만 허용
 * T3: Tick 범위 1 ~ INT32_MAX 검사
 * T4: 실패 시 출력값 보존
 */

/* Review draft: builds successfully but does not meet the requirements. */
bool ticks_from_ms(uint32_t hz, uint32_t ms, uint32_t *out)
{
    // 입력값 검증
    if (out == NULL || hz == 0U || ms == 0U) {
        return false;
    }

    // T1: 곱셈 전에 64-bit로 변환
    uint64_t product = (uint64_t)hz * ms;

    // T2: 나머지가 있으면 정확한 정수 Tick이 아님
    if (product % 1000U != 0U) {
        return false;
    }

    uint64_t ticks = product / 1000U;

    // T3: 허용 범위 검사
    if (ticks == 0U || ticks > INT32_MAX) {
        return false;
    }

    // T4: 모든 검사에 통과한 경우에만 출력 저장
    *out = (uint32_t)ticks;

    return true;
}

bool period_due(uint32_t now, uint32_t period, uint32_t *last)
{
    // NULL, 0, 최대 허용 주기 검사
    if (last == NULL || period == 0U || period > INT32_MAX) {
        return false;
    }

    // P1, P2: 경과 시간을 계산해서 주기와 비교
    if ((uint32_t)(now - *last) >= period) {

        // P3: 다음 주기는 현재 시각부터 계산
        *last = now;

        return true;
    }

    return false;

}
