#include "mission.h"
#include "stm32f0xx_hal.h"

static bool init = false;

bool app_init(void)
{
    /* TODO A1: 매 호출 시 성공 상태 해제 -> HAL_Init -> Clock -> LED.
       HAL/Clock 실패 시 GPIO 설정 없이 false. 모두 성공하면 true. */
    init = false;

    if (HAL_Init() != HAL_OK)
    {
        return false;
    }
    if(board_clock_init() != true)
    {
        return false;
    }
    
    board_led_init();

    init = true;

    return true;
}

void app_step(void)
{
    /* TODO A2: 초기화 성공 시에만 Toggle -> HAL_Delay(250).
       호출 1회당 Toggle 1회. 성공 상태는 이 Module 안에서 관리하세요. */
    if (init == true)
    {
        board_led_toggle();
        HAL_Delay(250);
    }
}
