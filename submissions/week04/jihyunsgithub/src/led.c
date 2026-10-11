#include "mission.h"
#include "stm32f0xx_hal.h"

#include "mission.h"
#include "stm32f0xx_hal.h"

void board_led_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    gpio.Pin = GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &gpio);
}

void board_led_toggle(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}