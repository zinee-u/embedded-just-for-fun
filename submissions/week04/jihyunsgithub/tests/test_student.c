#include "mission.h"
#include "fake_hal.h"
#include <assert.h>

void run_student_tests(void)
{
    /*
     * Case 1: SRAM 시작 주소 바로 이전 경계 검사
     *
     * 요구사항: SRAM 영역 밖의 주소는 MEMORY_UNKNOWN을 반환한다.
     * 입력: 0x1FFFFFFF (SRAM 시작 주소 0x20000000 바로 이전)
     * 예상 결과: MEMORY_UNKNOWN
     * 검증 이유: SRAM 하한 경계에서 범위 밖 주소가 잘못 포함되지
     *           않는지 확인한다.
     */
    MemoryRegion region = memory_region(0x1FFFFFFFU);
    assert(region == MEMORY_UNKNOWN);

    /*
     * Case 2: 실패한 재초기화 이후 Application 동작 차단
     *
     * 요구사항: app_init()이 실패하면 이전 초기화가 성공했더라도
     *           app_step()은 LED를 Toggle하거나 Delay하지 않는다.
     * 입력: 초기화 성공 후 HAL_Init에 HAL_BUSY 오류 주입
     * 예상 결과: 두 번째 app_init()은 false를 반환하며,
     *           이후 app_step()에서 Toggle 및 Delay가 발생하지 않는다.
     * 검증 이유: 이전 성공 상태가 실패한 재초기화 이후에도
     *           남아 있는 오류를 방지하는지 확인한다.
     */
    fake_hal_reset();
    bool first_result = app_init();
    assert(first_result == true);

    fake_hal_reset();
    fake_hal.hal_result = HAL_BUSY;

    bool second_result = app_init();
    assert(second_result == false);

    app_step();

    assert(fake_hal.toggle_count == 0U);
    assert(fake_hal.delay_count == 0U);
}