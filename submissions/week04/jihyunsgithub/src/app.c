#include "mission.h"
#include "stm32f0xx_hal.h"

static bool initialized = false; //app_init(void)와 app_step(void)는 성공 상태를 공유해야 하므로 전역변수 선언

bool app_init(void)
{
    /* TODO A1: 매 호출 시 성공 상태 해제 -> HAL_Init -> Clock -> LED.
       HAL/Clock 실패 시 GPIO 설정 없이 false. 모두 성공하면 true. */

    initialized = false; //이전 성공 상태를 다시 초기화 -> 이전 성공을 또 적용하지 않음

    if (HAL_Init() != HAL_OK) {
        return false;
    }

    if (!board_clock_init()) {
        return false;
    }

    board_led_init(); // board_led_init()은 반환값이 없어 확인 불가
    
    initialized = true;
    return true;
}

void app_step(void)
{
    /* TODO A2: 초기화 성공 시에만 Toggle -> HAL_Delay(250).
       호출 1회당 Toggle 1회. 성공 상태는 이 Module 안에서 관리하세요. */

    if (!initialized) { //초기화가 성공시에만 진행 되어야 하므로
        return;
    }

    board_led_toggle();
    HAL_Delay(250U);
}
