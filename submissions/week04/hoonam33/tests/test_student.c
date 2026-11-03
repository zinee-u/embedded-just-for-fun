#include "mission.h"
#include "fake_hal.h"
#include <assert.h>

void run_student_tests(void)
{
    /* TODO T1: 최소 2개의 경계/실패 Case를 assert로 추가하세요.
       각 Case에 요구사항, 입력, 예상 결과, 검증 이유를 주석으로 적으세요.
       필요하면 fake_hal_reset() 후 failure result를 주입하세요.
       fake_hal_reset()은 app.c의 성공 상태를 초기화하지 않습니다. */

    // Case 1: HAL_Init 실패 시 app_init()은 false를 반환해야 합니다.
    // 요구사항 : HAL_Init 실패 시 app_init()은 false를 반환해야 합니다.
    // 입력 : fake_hal_reset() 후 HAL_Init 실패를 주입
    // 예상 결과 : app_init()은 false를 반환해야 합니다.
    fake_hal_reset();
    fake_hal.hal_result = HAL_ERROR;
    assert(app_init() == false);
    assert(fake_hal.trace_count == 1); /* HAL_Init 이후 중단 */

    // Case 2: board_clock_init 실패 시 app_init()은 false를 반환해야 합니다.
    // 요구사항 : board_clock_init 실패 시 app_init()은 false를 반환해야 합니다.
    // 입력 : fake_hal_reset() 후 board_clock_init 실패를 주입
    // 예상 결과 : app_init()은 false를 반환해야 합니다.
    fake_hal_reset();
    fake_hal.osc_result = HAL_ERROR;
    assert(app_init() == false);
    assert(fake_hal.gpio_initialized == false);
}
