#include "mission.h"
#include "stm32f0xx_hal.h"

/* 목표 경로: HSI 8 MHz -> PLL 입력 (HSI 고정 /2) 4 MHz -> x12 -> 48 MHz
   = SYSCLK, HCLK (AHB /1), PCLK1 (APB1 /1).
   STM32F030x8에서 PLLSource가 HSI이면 Hardware가 고정으로 /2 하며
   PREDIV는 HSE 경로에만 적용되므로 DIV1을 명시한다. */
bool board_clock_init(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PREDIV = RCC_PREDIV_DIV1;
    osc.PLL.PLLMUL = RCC_PLL_MUL12;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        /* PLL이 준비되지 않았으므로 SYSCLK 전환을 시도하지 않는다. */
        return false;
    }

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    /* 24 MHz < HCLK <= 48 MHz 구간은 Flash wait state 1이 필요하다. */
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1) != HAL_OK) {
        return false;
    }

    return true;
}
