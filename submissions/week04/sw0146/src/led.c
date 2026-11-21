#include "mission.h"
#include "stm32f0xx_hal.h"

/* LED2 (NUCLEO-F030R8): GPIOA Pin 5. Pin 인자는 번호가 아니라 bit mask다. */
#define LED_PORT GPIOA
#define LED_PIN  GPIO_PIN_5

void board_led_init(void)
{
    /* 순서: GPIOA Clock Enable -> OFF 출력값 설정 -> GPIO 설정.
       Output으로 바뀌는 순간 Pin이 OFF 값을 내도록 출력값을 먼저 정한다. */
    GPIO_InitTypeDef init = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);

    init.Pin = LED_PIN;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &init);
}

void board_led_toggle(void)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}
