
#include "mission.h"
#include "fake_platform.h"
#include "test_support.h"

/*
 * Test A: 32-bit 중간 곱셈 Overflow 방지
 *
 * Requirement:
 * - 곱셈 전에 64-bit 형변환을 수행해야 한다.
 *
 * Input:
 * - hz = 5,000,000
 * - ms = 1,000
 *
 * Expected:
 * - 성공(true)
 * - out = 5,000,000
 *
 * Reason:
 * - 중간 곱셈값 5,000,000,000은 UINT32_MAX보다 크다.
 * - 그러나 나눈 결과는 허용 범위에 있으므로 성공해야 한다.
 */
static void test_large_intermediate_product(void)
{
    uint32_t out = 123U;

    CHECK(ticks_from_ms(5000000U, 1000U, &out));
    CHECK(out == 5000000U);
}

/*
 * Test B: 주기의 정확한 경계 검사
 *
 * Requirement:
 * - 경계 직전에는 false를 반환하고 last를 보존한다.
 * - 정확히 주기가 만료되면 true를 반환하고 last를 now로 갱신한다.
 *
 * Input:
 * - last = 500
 * - period = 25
 * - now = 524, 525
 *
 * Expected:
 * - now=524: false, last=500
 * - now=525: true, last=525
 *
 * Reason:
 * - 24 Tick과 25 Tick의 차이를 확인하여
 *   >= 비교 및 상태 갱신을 검증한다.
 */
static void test_exact_period_boundary(void)
{
    uint32_t last = 500U;

    CHECK(!period_due(524U, 25U, &last));
    CHECK(last == 500U);

    CHECK(period_due(525U, 25U, &last));
    CHECK(last == 525U);
}

void run_student_tests(void)
{
    test_large_intermediate_product();
    test_exact_period_boundary();
}
