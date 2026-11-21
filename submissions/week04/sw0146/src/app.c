#include "mission.h"
#include "stm32f0xx_hal.h"

#define APP_TOGGLE_INTERVAL_MS 250U

/* app_init이 모든 단계를 성공했을 때만 true. 이 Module 밖에서는 바꿀 수 없다. */
static bool s_initialized = false;

bool app_init(void)
{
    /* 재초기화가 실패하면 이전 성공 상태로 반복 동작을 계속하면 안 되므로
       시작할 때 먼저 성공 상태를 해제한다. */
    s_initialized = false;

    if (HAL_Init() != HAL_OK) {
        return false;
    }
    if (!board_clock_init()) {
        /* Clock이 요구값으로 설정되지 않았으므로 GPIO 설정을 진행하지 않는다. */
        return false;
    }
    board_led_init();

    s_initialized = true;
    return true;
}

void app_step(void)
{
    if (!s_initialized) {
        return;
    }
    board_led_toggle();
    HAL_Delay(APP_TOGGLE_INTERVAL_MS);
}
