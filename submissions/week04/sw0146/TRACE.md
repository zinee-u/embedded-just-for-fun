# 4주차 Datasheet · Build · Startup 추적

## 1. Datasheet 근거와 MCU 관계

| 구현한 값 | Datasheet 페이지·표·그림 또는 BSP 근거 | C File·함수와 사용 의미 |
| --- | --- | --- |
| Flash 영역 `0x08000000`~`0x0800FFFF` | Table 2 p10: STM32F030R8 열 Flash (Kbytes) **64**. Figure 10 p38: Flash memory 시작 `0x0800 0000`. 단 Figure 10의 Flash 칸 위쪽 경계 `0x0804 0000`은 Family 공통 그림이라 xC의 256 KB 기준이므로 R8의 끝으로 쓰지 않음. 끝 = `0x08000000 + 64×1024 − 1` = `0x08000000 + 0x10000 − 1` | `src/memory_map.c` `FLASH_BASE_ADDR`·`FLASH_SIZE_BYTES` → `memory_region`이 `MEMORY_FLASH` 반환 |
| SRAM 영역 `0x20000000`~`0x20001FFF` | Table 2 p10: STM32F030R8 열 SRAM (Kbytes) **8** (같은 표의 F030RC는 32이므로 Family 최대값과 구분). Figure 1 p11도 `4/8/32 KB`로 Family 전체를 표기. Figure 10 p38: SRAM 시작 `0x2000 0000`. 끝 = `0x20000000 + 0x2000 − 1` | `SRAM_BASE_ADDR`·`SRAM_SIZE_BYTES` → `MEMORY_SRAM` |
| GPIOA 영역 `0x48000000`~`0x480003FF` / LED2 Port·Pin | Table 17 p39: Bus **AHB2**, `0x4800 0000 - 0x4800 03FF`, Size **1 KB**, GPIOA. Figure 10 p38: AHB2 구간 시작 `0x4800 0000`. 끝 = `0x48000000 + 0x400 − 1`. LED2: [BSP `stm32f0xx_nucleo.h`](https://github.com/STMicroelectronics/stm32f0xx-nucleo-bsp/blob/9c947f2311227fab2c158350d541ad7ad29cce3d/stm32f0xx_nucleo.h#L95-L97) `LED2_PIN = GPIO_PIN_5`, `LED2_GPIO_PORT = GPIOA`, `LED2_GPIO_CLK_ENABLE() = __HAL_RCC_GPIOA_CLK_ENABLE()`. 예제 [readme.txt](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/readme.txt#L26-L27): "PA.05 … connected to LED2" (Rev C Board) | `GPIOA_BASE_ADDR`·`GPIOA_SIZE_BYTES` → `MEMORY_GPIOA`. LED2는 이 1 KB GPIOA Register 영역으로 제어되는 Port A의 Pin 5이며, `src/led.c`의 `LED_PORT = GPIOA`, `LED_PIN = GPIO_PIN_5`(bit mask `0x0020`) |
| RCC 영역 `0x40021000`~`0x400213FF` | Table 17 p39: Bus **AHB1**, `0x4002 1000 - 0x4002 13FF`, Size **1 KB**, RCC. Figure 10 p38: AHB1 구간 `0x4002 0000`~`0x4002 43FF`. 같은 구간의 `0x4002 0000 - 0x4002 03FF`는 Table 17상 DMA이므로 RCC 시작과 구분. 끝 = `0x40021000 + 0x400 − 1` | `RCC_BASE_ADDR`·`RCC_SIZE_BYTES` → `MEMORY_RCC` |
| HSI·PLL·SYSCLK·Bus Clock | Table 35 p56: f_HSI **8 MHz** (25 °C ±1 % 사용자 보정 시, -40~85 °C ±5 %). Figure 2 p15: HSI → `/2` → PLLSRC → PLLMUL, `PREDIV`는 HSE(OSC_IN) 경로에만 있음. HCLK가 "AHB, core, memory, DMA"로 공급. Figure 1 p11: CPU `fMAX = 48 MHz`. p47: "1 wait state … above 24 MHz" | `src/clock.c` `board_clock_init`: PLLSource HSI, PREDIV DIV1, PLLMUL ×12, SYSCLK = PLLCLK, AHB /1, APB1 /1, `FLASH_LATENCY_1` |

CPU·Flash·SRAM·RCC·GPIOA 관계도를 아래에 작성합니다. 주소 접근과 Clock 공급·Enable 화살표의 의미를 구분하세요.

범위 판정 방법: 끝 주소를 표에서 옮겨 적지 않고 `시작 + 크기 − 1`로 계산해 표의 끝 주소와 일치하는지 확인했습니다. 코드에서는 `address - base <= size - 1`(uint32_t)로 비교합니다. `base + size`를 직접 더하지 않으므로 `UINT32_MAX` 근처에서도 overflow가 없고, `address < base`이면 뺄셈이 큰 값으로 wrap-around되어 범위 밖으로 판정됩니다. Boot alias(Figure 10 p38의 `0x0000 0000`~`0x0004 0000`, "Flash, system memory or SRAM, depending on BOOT configuration")와 다른 Peripheral은 요구 범위 밖이므로 `MEMORY_UNKNOWN`입니다.

화살표 의미:

- `==>` **CPU 주소 접근 경로**: CPU가 Bus를 통해 해당 주소를 읽고/쓰는 경로 (Data·명령 이동)
- `-->` **Clock 공급**: RCC가 만든 Clock 신호가 전달되는 방향 (Data가 아님)
- `-.->` **Clock Enable 제어**: RCC Register(`RCC_AHBENR` 등)의 Enable bit가 해당 주변장치의 Clock gate를 여는 관계 (신호가 아닌 On/Off 제어)

```text
[A] CPU 주소 접근 경로 (==>)

  CPU (Cortex-M0)
     ==> Bus matrix ==> Flash  0x08000000~0x0800FFFF  (명령 Fetch, 상수 읽기)
     ==> Bus matrix ==> SRAM   0x20000000~0x20001FFF  (Stack, .data, .bss)
     ==> Bus matrix ==> AHB1 ==> RCC    0x40021000~0x400213FF  (Clock 설정 Register)
     ==> Bus matrix ==> AHB2 ==> GPIOA  0x48000000~0x480003FF  (Pin 설정·출력 Register)

[B] Clock 공급 (-->)

  HSI 8 MHz --> (PLL 등 RCC 내부 경로) --> SYSCLK --> AHB prescaler --> HCLK
  RCC가 만든 HCLK --> CPU, Bus matrix
                  --> Flash interface
                  --> SRAM
                  --> GPIOA   (단, [C]의 Enable gate를 통과해야 함)

[C] Clock Enable 제어 (-.->)

  CPU ==> RCC_AHBENR (IOPAEN bit = 1)      ... [A] 경로로 RCC Register에 쓰기
  RCC_AHBENR.IOPAEN -.-> GPIOA Clock gate   ... 그 결과 HCLK --> GPIOA 공급 시작
```

읽는 법:

1. **주소 접근(`==>`)**: CPU는 Flash에서 명령을 가져오고, SRAM·RCC·GPIOA Register에 Bus matrix를 거쳐 접근합니다. RCC와 GPIOA도 결국 "주소가 할당된 Register 묶음"이므로 CPU 입장에서는 같은 Load/Store 경로입니다. (Figure 1 p11, Figure 10 p38)
2. **Clock 공급(`-->`)**: RCC가 SYSCLK → AHB prescaler를 거쳐 만든 HCLK를 CPU·Bus·Flash interface·SRAM·AHB 주변장치에 공급합니다. Figure 2 p15에는 HCLK의 공급 대상이 "AHB, core, memory, DMA"로 적혀 있고, Figure 1 p11에는 이 역할을 맡는 블록이 "RESET & CLOCK CONTROL"(System and peripheral clocks)로 나옵니다. Figure 2에 따로 그려진 FLITFCLK(→ Flash memory)는 Flash 프로그래밍/지우기용 Clock이며, CPU가 Flash를 읽을 때 쓰는 HCLK와는 다른 경로입니다.
3. **Clock Enable(`-.->`)**: GPIOA는 Reset 직후 Clock이 꺼져 있어, CPU가 RCC Register에 Enable bit를 써야 GPIOA에 Clock이 들어갑니다. 이 화살표는 "CPU가 RCC에 쓰는 주소 접근"과 "그 결과 GPIOA Clock gate가 열리는 제어"가 서로 다른 단계임을 나타냅니다. Enable 전에는 GPIOA 주소가 Memory map에 존재하더라도 GPIO 설정이 동작하지 않습니다. 즉 `memory_region`이 `MEMORY_GPIOA`를 반환하는 것은 주소 소속만 뜻하며 접근 가능 상태를 뜻하지 않습니다.

> 확인 범위: 위 표·그림 번호와 페이지는 DS9773 Rev 5(November 2021) PDF 원문에서 대조했습니다 (Table 2 p10, Figure 1 p11, Figure 2 p15, Figure 10 p38, Table 17 p39). Enable bit 이름(`IOPAEN`)의 Register 위치는 Datasheet가 아니라 Reference manual(RM0360)·HAL의 `__HAL_RCC_GPIOA_CLK_ENABLE` 정의에서 확인하는 내용입니다.

Clock 계산식과 공개 예제에서 선택한 경로:

```text
HSI 8 MHz --(F030x8 고정 /2)--> 4 MHz --PLLMUL x12--> PLLCLK 48 MHz
PLLCLK --SW=PLL--> SYSCLK 48 MHz --AHB /1--> HCLK 48 MHz --APB1 /1--> PCLK1 48 MHz
Flash: HCLK 48 MHz > 24 MHz 이므로 wait state 1 (FLASH_LATENCY_1)
```

- **고정 /2의 Source 근거**: [system_stm32f0xx.c:285-286](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/system_stm32f0xx.c#L285-L286)의 F030x8 분기 `SystemCoreClock = (HSI_VALUE >> 1) * pllmull;` ("HSI used as PLL clock source : SystemCoreClock = HSI/2 * PLLMUL"). 바로 위 `HSI/PREDIV` 분기는 F030xC 등 다른 제품에만 해당합니다.
- **공개 예제의 경로**: [main.c:102-127](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c#L102-L127) `SystemClock_Config`가 "No HSE Oscillator on Nucleo, Activate PLL with HSI/2 as source"로 같은 경로(PLLMUL ×12, SYSCLK = PLLCLK, AHB·APB1 /1, `FLASH_LATENCY_1`)를 선택합니다. [readme.txt:29](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/readme.txt#L29): "HCLK is configured at 48 MHz".
- **최대 주파수와 Reset 직후 상태는 다릅니다**: Figure 1의 `fMAX = 48 MHz`는 CPU가 **동작할 수 있는 상한**입니다. Reset 직후에는 HSI 8 MHz가 SYSCLK이고 PLL은 꺼져 있습니다. [system_stm32f0xx.c:119](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/system_stm32f0xx.c#L119) `SystemCoreClock = 8000000`, Datasheet p56 "SYSCLK takes the default setting: HSI 8 MHz" (Stop/Standby 복귀 시). 48 MHz는 소프트웨어가 PLL을 켜고 SYSCLK를 전환해야 도달하는 값이며, 그 전에 Flash latency를 1로 올려야 합니다.

## 2. 공개 예제와 제품 요구의 차이

| 항목 | 공개 예제의 설정·근거 | 이번 제품의 요구·변경한 C 위치 |
| --- | --- | --- |
| Clock | [main.c:108-112](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c#L108-L112): `OscillatorType = RCC_OSCILLATORTYPE_NONE`(HSI는 Reset 후 이미 켜져 있다고 보고 따로 설정하지 않음), PLL HSI/2 ×12, `FLASH_LATENCY_1`(123행). 구조체를 0으로 초기화하지 않음 | 같은 48 MHz 경로이지만 `OscillatorType = HSI`, `HSIState = ON`, `HSICalibrationValue = DEFAULT`를 **명시**. 구조체는 `= {0}`으로 초기화 (`src/clock.c` `board_clock_init`) |
| LED 초기 상태·Mode·Pull·Speed | [main.c:68-76](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c#L68-L76): Clock Enable → `HAL_GPIO_Init` (초기 출력값 설정 없음). `GPIO_PULLUP`, `GPIO_SPEED_FREQ_HIGH`, Output PP | Clock Enable → `HAL_GPIO_WritePin(RESET)`로 **OFF 먼저** → `HAL_GPIO_Init`. `GPIO_NOPULL`, `GPIO_SPEED_FREQ_LOW`, Output PP (`src/led.c` `board_led_init`). Output 전환 순간 LED가 의도치 않게 켜지지 않도록 출력값을 먼저 정함 |
| Toggle 간격 | [main.c:79-84](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c#L79-L84): `while (1)` 안에서 Toggle → `HAL_Delay(100)` | `app_step` 1회당 `board_led_toggle()` → `HAL_Delay(250)` (`src/app.c`). 반복 Loop는 호출자가 담당 |
| 실패 처리 | [main.c:113-126·133-139](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c#L133-L139): Osc/Clock 실패 시 `Error_Handler()`의 `while(1)`로 정지. `HAL_Init` 반환값은 확인하지 않음 | `HAL_Init`, Osc, Clock 실패를 각각 `false`로 **호출자에게 전달**하고 남은 단계를 중단 (`src/clock.c`, `src/app.c`). 실패하면 성공 상태를 해제해 `app_step`이 동작하지 않음 |

## 3. 공개 STM32 Project의 Build

| 확인할 내용 | 실제 File·설정·근거 |
| --- | --- |
| Target / Compiler 계열 / Version 명시 여부 | [.cproject](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/SW4STM32/STM32F030R8-Nucleo/.cproject) 19-25행: Toolchain "Ac6 STM32 MCU GCC", Prefix `arm-none-eabi-`, Mcu `STM32F030R8Tx`, Board `NUCLEO-F030R8`, Thumb, FPU 없음, soft float ABI. 34-35행: 최적화 `-Os`(size), Debug level max. **Compiler version은 설정 File에 명시되어 있지 않음** (설치된 SW4STM32 Toolchain에 따라 달라짐) |
| Source 입력 / Include / Define | [.project](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/SW4STM32/STM32F030R8-Nucleo/.project) 27-70행 Linked resource: `main.c`, `stm32f0xx_it.c`, `system_stm32f0xx.c`, `startup_stm32f030x8.s`, BSP `stm32f0xx_nucleo.c`, HAL `stm32f0xx_hal.c`·`_hal_cortex.c`·`_hal_gpio.c`·`_hal_rcc.c`·`_hal_rcc_ex.c`. .cproject 36행 Include: `Inc`, CMSIS Device, HAL `Inc`, BSP, CMSIS `Include`. 37행 Define: `STM32F030x8`, `USE_HAL_DRIVER`, `USE_STM32F0XX_NUCLEO` |
| Startup Object / Linker script / Library | Startup: `SW4STM32/startup_stm32f030x8.s` (.project 56-58행). Linker script: .cproject 47행 `-T ../STM32F030R8Tx_FLASH.ld`. Library: 50행 `-specs=nosys.specs -specs=nano.specs` (newlib-nano, System call stub). 별도 `-l` Library 없음 (48행 비어 있음) |
| 실행 Image와 후처리 산출물 | .cproject 17행: `artifactName="STM32F030R8-Nucleo"`, `artifactExtension="elf"` → `STM32F030R8-Nucleo.elf`. Post-build: `arm-none-eabi-objcopy -O binary … .elf … .bin && arm-none-eabi-size …` → `.bin` 생성과 Size 출력 |

Header를 포함하는 것과 `.c`를 Build 입력에 넣는 것의 차이:

- `#include "stm32f0xx_hal.h"`는 **선언**(함수 원형, typedef, `#define`)만 전처리 단계에서 붙여 넣습니다. Compiler는 이것으로 호출 문법과 타입을 검사하지만, 함수의 기계어는 만들지 않습니다.
- 함수의 **정의**는 해당 `.c`가 Build 입력에 들어가 `.o`가 되어야 생깁니다. 공개 예제는 `.project`에 HAL `.c` 다섯 개만 Link 대상으로 넣었으므로, Include 경로에 있는 다른 HAL Header(예: UART)를 포함만 하고 함수를 호출하면 Link 단계에서 `undefined reference`가 납니다.
- 이번 Host Build에서도 `src/led.c`는 `stm32f0xx_hal.h`의 `HAL_GPIO_Init` 선언만 보고 Compile되며, 실제 정의는 `host/fake_hal.c`가 Build 입력(`SUPPORT_SRC`)에 있어서 Link 때 연결됩니다 (`nm build/src/led.o`에서 `U HAL_GPIO_Init`, `nm build/host/fake_hal.o`에서 `T HAL_GPIO_TogglePin`).

## 4. 공개 예제의 Reset부터 main까지

| 실제 순서 | File·함수·label | 준비하는 상태와 다음 단계의 관계 |
| --- | --- | --- |
| 0. Vector table | [startup_stm32f030x8.s:117-126](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/SW4STM32/startup_stm32f030x8.s#L117-L126) `.isr_vector`의 `g_pfnVectors`: 첫 word `_estack`, 둘째 word `Reset_Handler`. Linker script 52-55행이 `.isr_vector`를 Flash 맨 앞에 `KEEP` | Reset 시 Core가 0번 word를 MSP 초기값으로, 1번 word를 시작 주소로 읽음 |
| 1. SP 설정 | startup 51-52행 `ldr r0, =_estack` / `mov sp, r0` | `_estack = 0x20002000` (ld 36행, SRAM 8 KB의 끝 다음 주소). 이후 함수 호출에 Stack 사용 가능 |
| 2. `.data` 복사 | startup 54-69행 `CopyDataInit` / `LoopCopyDataInit` | Flash의 초기값(`_sidata = LOADADDR(.data)`, ld 114행)을 SRAM `_sdata`~`_edata`로 복사 (ld 126행 `>RAM AT> FLASH`). 예: `SystemCoreClock = 8000000`이 SRAM에 올바르게 놓임 |
| 3. `.bss` 0 채우기 | startup 70-82행 `FillZerobss` / `LoopFillZerobss` | `_sbss`~`_ebss`(ld 134·141행)를 0으로. C의 "초기값 없는 전역 변수 = 0" 규칙 충족 |
| 4. `SystemInit` | startup 85행 `bl SystemInit` → [system_stm32f0xx.c:146-205](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/system_stm32f0xx.c#L146-L205) | RCC를 Reset 기본 상태로 되돌림 (HSION, SW/HPRE/PPRE/PLL 관련 bit Clear, RCC 인터럽트 Off). SYSCLK = HSI 8 MHz |
| 5. C Library 초기화 | startup 87행 `bl __libc_init_array` | `.preinit_array`/`.init_array`(정적 생성자 등) 실행. newlib-nano가 제공 |
| 6. `main` | startup 89행 `bl main` → [main.c:48](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c#L48) | Application 시작. 반환하면 91-92행 `LoopForever` |
| 7. `HAL_Init` | main.c 62행 → [stm32f0xx_hal.c:141-157](https://github.com/STMicroelectronics/stm32f0xx_hal_driver/blob/115eb1dc87e26ea29f4f2ca58003650381aabba2/Src/stm32f0xx_hal.c#L141-L157) | Flash prefetch, SysTick 1 ms(`HAL_InitTick`, 현재 `SystemCoreClock` 기준), `HAL_MspInit`. 이후 `HAL_Delay`와 RCC Timeout 판정이 가능해짐 ([stm32f0xx_it.c:92-95](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/stm32f0xx_it.c#L92-L95) `SysTick_Handler` → `HAL_IncTick`) |
| 8. `SystemClock_Config` | main.c 65행 | PLL 48 MHz로 전환. HAL이 SysTick을 새 Clock에 맞게 재설정 |
| 9. LED 설정·Loop | main.c 68-84행 | GPIOA Clock Enable → GPIO Init → Toggle·Delay 반복 |

`SystemInit`과 `SystemClock_Config`의 호출 위치·역할 차이:

| | `SystemInit` | `SystemClock_Config` |
| --- | --- | --- |
| 호출 위치 | Startup `Reset_Handler`, `main` 이전 (85행) | `main` 안, `HAL_Init` 다음 (65행) |
| 정의 File | `system_stm32f0xx.c` (CMSIS System) | 예제 `main.c` (Application) |
| 역할 | RCC를 Reset 기본값으로 **정리**. Clock을 올리지 않음 (HSI 8 MHz) | 제품이 원하는 Clock으로 **설정**. PLL 48 MHz, 분주, Flash latency |
| HAL 의존 | 없음 (Register 직접 접근) | `HAL_RCC_OscConfig`, `HAL_RCC_ClockConfig` 사용. SysTick Timeout을 위해 `HAL_Init` 이후여야 함 |
| 이번 과제의 대응 | Host에는 Startup이 없어 실행되지 않음 | `board_clock_init()` |

## 5. 내가 실행한 Host Build

| 단계 | 실제 입력 File | 실제 출력 File | Log의 명령·옵션과 역할 |
| --- | --- | --- | --- |
| Preprocess | `src/led.c` (+ `include/mission.h`, `host/stm32f0xx_hal.h`) | `build/src/led.i` | `cc -Iinclude -Ihost -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g -E src/led.c -o build/src/led.i`. 전처리기: `-I`로 Header 경로 지정, `#include` 삽입·`#define` 치환·주석 제거. 예: `HAL_GPIO_TogglePin(LED_PORT, LED_PIN)` → `HAL_GPIO_TogglePin((&mission_host_gpioa), ((uint16_t)0x0020U))`. Header가 붙어 약 30줄 → 약 330줄 |
| Compile | `build/src/led.i` | `build/src/led.s` | `cc -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g -S build/src/led.i -o build/src/led.s`. Compiler: C를 Host(x86-64) 어셈블리 텍스트로 번역. 문법·타입 검사와 경고(`-Werror`로 오류 처리)가 이 단계에서 발생 |
| Assemble | `build/src/led.s` | `build/src/led.o` | `cc -g -c build/src/led.s -o build/src/led.o`. Assembler: 기계어 Relocatable Object(ELF 64-bit relocatable) 생성. 외부 함수는 미해결 Symbol로 남음 (`nm`: `U HAL_GPIO_Init`) |
| Link | `build/src/{memory_map,clock,led,app}.o`, `build/host/fake_hal.o`, `build/tests/{test_mission,test_student}.o` | `build/mission-tests` | `cc -std=c11 … -g build/src/memory_map.o … build/tests/test_student.o -o build/mission-tests`. Linker: 7개 Object의 Symbol을 연결(`led.o`의 `U HAL_GPIO_TogglePin` ↔ `fake_hal.o`의 `T`)하고 Host C Runtime과 묶어 실행 File 생성 (ELF 64-bit PIE, x86-64, GNU/Linux) |

위 명령과 순서는 `make clean` 후 `make stages`의 실제 출력이며, 같은 4단계가 7개 Source 각각에 반복됩니다. 전체 출력은 `logs/stages.log`에 있습니다. Build 환경: Ubuntu 22.04, `cc (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0` (자세한 내용은 `BUILD_ENV.md`).

Host에서 검증한 내용 / ARM Source로만 확인한 내용 / Board에서 아직 확인하지 않은 내용:

| 구분 | 내용 | 근거 |
| --- | --- | --- |
| **Host에서 실행해 검증** | `memory_region`의 경계·영역 밖 판정. Clock 설정값·호출 순서·Osc/Clock 실패 전달. LED Port·Pin·Mode·Pull·Speed·초기 OFF·초기화 순서·PA5만 Toggle. `app_init` 성공·실패·재초기화 실패·복구, `app_step`의 Toggle → 250 ms. 추가로 LED ON 상태 재초기화, `app_step` 4회 누적 | `make test` (`logs/test.log`), `tests/test_student.c` |
| **ARM Source를 읽어서만 확인** | Vector table·`Reset_Handler`의 `.data`/`.bss`/`SystemInit`/`__libc_init_array`/`main` 순서, Linker script의 Memory 배치와 `_estack`, 공개 Project의 Compiler·Define·Library 설정, HAL의 실제 Register 동작 | 위 §3·§4의 고정 Commit 링크와 행 번호. Host Build에는 Startup·Linker script·Vector table이 쓰이지 않으므로 Host 실행이 이를 대신 검증하지 않음 |
| **Board에서 확인하지 않음** | `arm-none-eabi-gcc` Build, Flash 기록, 실제 48 MHz 동작, LED2의 전기적 점등, Toggle 간격(명목 250 ms / 주기 약 500 ms)의 실측, HSI 오차 | ARM Build·Board 실행은 하지 않았음 (추가 실습 범위) |

## 6. Module과 초기화

네 `.c`의 책임과 호출 방향, 공개 API와 내부 상태를 설명합니다.

- memory_map.c: `memory_region(uint32_t)`만 공개합니다. 네 영역을 `{base, size, region}` 상수 표(`k_regions`)로 두고 내부 함수 `in_range`로 양 끝 포함 판정을 합니다. 주소 숫자만 비교하며 Pointer 변환·역참조가 없으므로 Host에서도 안전하게 실행됩니다. HAL이나 다른 Module을 호출하지 않는 순수 함수입니다.
- clock.c: `board_clock_init(void) → bool`만 공개합니다. 내부에서 `RCC_OscInitTypeDef`, `RCC_ClkInitTypeDef`를 `{0}`으로 만들고 HSI·PLL(×12)·SYSCLK·분주·Flash latency를 채워 `HAL_RCC_OscConfig` → `HAL_RCC_ClockConfig` 순서로 호출합니다. 어느 하나라도 `HAL_OK`가 아니면 즉시 `false`. 내부 상태는 없습니다.
- led.c: `board_led_init`, `board_led_toggle`을 공개합니다. LED Port·Pin(`LED_PORT`, `LED_PIN`)과 GPIO 설정값은 이 File 안에만 있어 Application은 Pin 번호를 모릅니다. 초기화는 GPIOA Clock Enable → `WritePin(RESET)`(OFF) → `HAL_GPIO_Init`, Toggle은 `HAL_GPIO_TogglePin`만 호출합니다. 시간 지연은 이 Module의 책임이 아닙니다. 내부 상태는 없습니다.
- app.c: `app_init`, `app_step`을 공개하고 내부 상태 `static bool s_initialized`를 가집니다. HAL과 `clock.c`·`led.c`의 공개 API만 호출하며, `GPIO_PIN_5` 같은 하드웨어 세부값을 쓰지 않습니다. 반복 간격 250 ms(`APP_TOGGLE_INTERVAL_MS`)는 이 Module이 관리합니다.

호출 방향과 Build 참여:

```text
tests/test_mission.c, tests/test_student.c
        │ (mission.h 선언으로 호출)
        ▼
     app.c ──> clock.c ──> HAL_RCC_OscConfig / HAL_RCC_ClockConfig ┐
       │  └──> led.c   ──> __HAL_RCC_GPIOA_CLK_ENABLE / HAL_GPIO_* ├─> host/fake_hal.c (Host 대역)
       └────> HAL_Init / HAL_Delay ─────────────────────────────────┘
memory_map.c: 다른 Module을 호출하지 않는 독립 함수 (Test가 직접 호출)
```

`include/mission.h`는 네 Module 공개 함수의 **선언**만 가지며 각 `.c`와 Test가 `#include`해 서로의 원형을 공유합니다. 각 `.c`는 Makefile의 `MISSION_SRC`에 있어 따로 `.i → .s → .o`가 되고, Link 단계에서 하나의 실행 File로 합쳐집니다. `app.o`는 `board_clock_init`을 `U`(미해결)로 가지고 `clock.o`가 `T`로 제공합니다.

초기화 성공·실패·재초기화 실패 시 호출 순서와 `app_step` 동작:

| 상황 | `app_init` 호출 순서 (Fake HAL trace) | 반환 | 이후 `app_step` |
| --- | --- | --- | --- |
| 초기화 전 | (호출 없음) | — | 아무 호출 없음 |
| 성공 | `HAL_Init` → `OscConfig` → `ClockConfig` → GPIOA Clock Enable → `WritePin(RESET)` → `GPIO_Init` | `true` | 1회당 `TogglePin(PA5)` → `HAL_Delay(250)` |
| `HAL_Init` 실패 | `HAL_Init` | `false` | 아무 호출 없음 |
| Osc 실패 | `HAL_Init` → `OscConfig` | `false` | 아무 호출 없음 |
| Clock 실패 | `HAL_Init` → `OscConfig` → `ClockConfig` | `false` | 아무 호출 없음 |
| 성공 후 재초기화 실패 | 시작 시 `s_initialized = false` 후 위 실패 순서 중 하나 | `false` | 이전 성공 상태가 지워졌으므로 아무 호출 없음 |
| 실패 후 다시 성공 | 성공과 같음 | `true` | 다시 Toggle → 250 ms |

실패 시 GPIO Clock Enable·GPIO 설정까지 가지 않으므로 Clock이 요구값이 아닌 상태에서 LED를 설정하지 않습니다.
