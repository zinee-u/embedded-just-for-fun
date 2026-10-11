# 4주차 Datasheet · Build · Startup 추적

## 1. Datasheet 근거와 MCU 관계

| 구현한 값 | Datasheet 페이지·표·그림 또는 BSP 근거 | C File·함수와 사용 의미 |
| --- | --- | --- |
| Flash 영역 | DS9773 Rev 5, Table 2 (p.10): STM32F030R8 Flash 64 KiB. Figure 10 (p.38): Flash 주소 영역. 공식 `STM32F030R8Tx_FLASH.ld`: 시작 주소 `0x08000000`, 크기 64K | `src/memory_map.c`의 `memory_region()`: `0x08000000` ~ `0x0800FFFF`를 `MEMORY_FLASH`로 분류 |
| SRAM 영역 | DS9773 Rev 5, Table 2 (p.10): STM32F030R8 SRAM 8 KiB. Figure 10 (p.38): SRAM 주소 영역. 공식 Linker Script: 시작 주소 `0x20000000`, 크기 8K | `src/memory_map.c`의 `memory_region()`: `0x20000000` ~ `0x20001FFF`를 `MEMORY_SRAM`으로 분류 |
| GPIOA 영역 / LED2 Port·Pin | DS9773 Rev 5, Figure 10 (p.38), Table 17 (p.39): GPIOA 시작 주소 `0x48000000`. STM32F0xx Nucleo BSP 헤더 `stm32f0xx_nucleo.h`: LED2는 GPIOA의 PA5 사용 | `src/memory_map.c`: `0x48000000` ~ `0x480003FF`를 `MEMORY_GPIOA`로 분류. `src/led.c`: GPIOA의 PA5를 LED 출력으로 초기화하고 Toggle |
| RCC 영역 | DS9773 Rev 5, Figure 10 (p.38), Table 17 (p.39): RCC 시작 주소 `0x40021000` | `src/memory_map.c`: `0x40021000` ~ `0x400213FF`를 `MEMORY_RCC`로 분류. `src/clock.c`: HAL RCC API를 통해 Clock 설정 |
| HSI·PLL·SYSCLK·Bus Clock | DS9773 Rev 5, Figure 2 (p.15): Clock Tree. 공식 `GPIO_IOToggle/Src/main.c`의 `SystemClock_Config()`: HSI 8 MHz, HSI/2, PLL ×12, SYSCLK 48 MHz, AHB/APB1 분주 1 | `src/clock.c`의 `board_clock_init()`: HSI 활성화 및 PLL 설정, SYSCLK 48 MHz 선택, HCLK/PCLK1 48 MHz 설정, `FLASH_LATENCY_1` 지정 |


CPU·Flash·SRAM·RCC·GPIOA 관계도를 아래에 작성합니다. 주소 접근과 Clock 공급·Enable 화살표의 의미를 구분하세요.

```text
                        STM32F030R8
+--------------------------------------------------+
|                                                  |
|   +---------------------------+                  |
|   | ARM Cortex-M0 CPU         |                  |
|   | 명령어 실행 및 주소 접근    |                  |
|   +-------------+-------------+                  |
|                 |                                |
|                 | 주소 버스를 통한 메모리 접근     |
|                 v                                |
|         +------------------+                     |
|         | Bus / Interconnect|                    |
|         +--------+---------+                     |
|                  |                               |
|        +---------+----------+                    |
|        |         |          |                    |
|        v         v          v                    |
|   +---------+ +---------+ +---------+            |
|   | Flash   | | SRAM    | | GPIOA   |            |
|   | 64 KiB  | | 8 KiB   | | PA5     |----> LED2   |
|   | 0x080.. | | 0x200.. | | 0x480.. |            |
|   +---------+ +---------+ +----+----+            |
|                               ^                  |
|                               | Clock Enable     |
|                               |                  |
|                          +----+----+             |
|                          | RCC     |             |
|                          | Clock   |             |
|                          | Control |             |
|                          +----+----+             |
|                               |                  |
|                SYSCLK / HCLK / PCLK 공급·분주      |
|                                                  |
+--------------------------------------------------+
```

Clock 계산식과 공개 예제에서 선택한 경로:

- HSI = 8 MHz
- PLL 입력 = HSI / 2 = 8 MHz / 2 = 4 MHz
- PLL 출력 = 4 MHz × 12 = 48 MHz
- SYSCLK = PLLCLK = 48 MHz
- HCLK = SYSCLK / 1 = 48 MHz
- PCLK1 = HCLK / 1 = 48 MHz

공개 `GPIO_IOToggle` 예제의 `SystemClock_Config()`는 HSI를 PLL 입력으로 사용하고, HSI/2와 PLL 배율 12를 통해 48 MHz를 설정한다. SYSCLK 소스로 PLLCLK를 선택하며 AHB 및 APB1 분주값은 DIV1이다.

이번 과제의 `src/clock.c`에서도 동일한 48 MHz Clock 경로를 사용하되, HSI ON, 기본 Calibration, PREDIV DIV1 및 HAL 실패 시 `false` 반환을 명시적으로 구현했다.

## 2. 공개 예제와 제품 요구의 차이

| 항목 | 공개 예제의 설정·근거 | 이번 제품의 요구·변경한 C 위치 |
| --- | --- | --- |
| Clock | 공식 `main.c`의 `SystemClock_Config()`: HSI/2 × 12로 48 MHz 설정. PLL Source는 HSI, AHB/APB1 DIV1, `FLASH_LATENCY_1` 사용. OscillatorType은 `RCC_OSCILLATORTYPE_NONE`으로 설정하고 실패 시 `Error_Handler()` 호출 | `src/clock.c`의 `board_clock_init()`: HSI ON, 기본 Calibration, PLL ON, PREDIV DIV1, PLL ×12, SYSCLK/HCLK/PCLK1 48 MHz를 명시. HAL 호출 실패 시 `false` 반환 |
| LED 초기 상태·Mode·Pull·Speed | 공식 `main.c`: GPIO Clock 활성화 후 LED2를 Output Push-Pull, Pull-up, High Speed로 설정하고 Toggle. 초기 출력 OFF를 위한 별도 RESET 호출은 없음 | `src/led.c`의 `board_led_init()`: GPIOA Clock Enable → PA5 RESET → Output Push-Pull / NOPULL / LOW Speed 설정. 초기 LED OFF를 명시적으로 구현 |
| Toggle 간격 | 공식 `main.c`: `while(1)`에서 LED Toggle 후 `HAL_Delay(100)` 호출 | `src/led.c`의 `board_led_toggle()`은 PA5 Toggle만 수행. `src/app.c`의 `app_step()`이 Toggle 후 `HAL_Delay(250)` 호출. 명목상 Toggle 간격 250 ms, ON/OFF 한 주기 약 500 ms |
| 실패 처리 | 공식 `SystemClock_Config()`는 HAL Clock 설정 실패 시 `Error_Handler()`를 호출하여 무한 루프에 진입 | `src/clock.c`는 HAL 실패 시 `false` 반환. `src/app.c`는 실패 시 이후 초기화를 중단하고 내부 성공 상태를 해제하여 `app_step()` 동작을 차단 |
|

## 3. 공개 STM32 Project의 Build

| 확인할 내용 | 실제 File·설정·근거 |
| --- | --- |
| Target / Compiler 계열 / Version 명시 여부 | `.cproject`: Target MCU `STM32F030R8Tx`, Board `NUCLEO-F030R8`. Toolchain은 `Ac6 STM32 MCU GCC`, 명령 접두어는 `arm-none-eabi-`. Thumb Instruction Set과 Soft Float ABI 사용. 실제 Compiler 버전 번호는 `.cproject`에 명시되지 않았으며 ARM 컴파일러를 실행해 확인하지 않았다. |
| Source 입력 / Include / Define | `.project`: `main.c`, `system_stm32f0xx.c`, `stm32f0xx_it.c`, `startup_stm32f030x8.s`, HAL RCC/GPIO/Core 관련 `.c`, Nucleo BSP `.c`가 프로젝트에 연결되어 있다. `.cproject` Include는 예제 `Inc`, CMSIS Device, HAL Driver, BSP Nucleo, CMSIS Core 경로를 사용한다. Define은 `STM32F030x8`, `USE_HAL_DRIVER`, `USE_STM32F0XX_NUCLEO`이다. |
| Startup Object / Linker script / Library | Startup Source `startup_stm32f030x8.s`가 프로젝트에 연결되며, 빌드 시 Object 파일로 변환되어 링크 대상이 된다. Linker Script는 `STM32F030R8Tx_FLASH.ld`이고, 링커 옵션은 `-specs=nosys.specs -specs=nano.specs`이다. |
| 실행 Image와 후처리 산출물 | `.cproject`의 Artifact는 `STM32F030R8-Nucleo.elf`이다. Post-build 명령은 `arm-none-eabi-objcopy -O binary`로 `.bin` 파일을 생성하고 `arm-none-eabi-size`로 크기를 출력하도록 설정되어 있다. 실제 ARM 빌드는 수행하지 않았으므로 해당 파일이 생성된 것을 직접 확인한 것은 아니다. |


Header를 포함하는 것과 `.c`를 Build 입력에 넣는 것의 차이:

- **Header 포함 (`#include`):** 함수 선언, 자료형, 구조체 및 매크로 정의를 컴파일러가 참조할 수 있게 한다. 예를 들어 `stm32f0xx_hal.h`를 포함하면 HAL API의 선언을 사용할 수 있다.
- **Source 빌드 (`.c`):** 함수의 실제 구현을 컴파일하여 Object 파일(`.o`)로 만든 뒤 Link 단계에서 실행 파일에 포함한다.

따라서 Header를 포함했다고 해서 함수 구현까지 실행 파일에 자동으로 포함되는 것은 아니다. 실제 HAL 함수가 사용되려면 해당 구현을 제공하는 `.c` 파일 또는 라이브러리가 빌드 및 링크 과정에 포함되어야 한다.

## 4. 공개 예제의 Reset부터 main까지

| 실제 순서 | File·함수·label | 준비하는 상태와 다음 단계의 관계 |
| --- | --- | --- |
| 1. MCU Reset 및 Vector 참조 | `startup_stm32f030x8.s`의 `.isr_vector`, `g_pfnVectors` | Vector Table에 초기 Stack Pointer인 `_estack`과 `Reset_Handler` 주소가 정의되어 있다. CPU는 이를 사용해 Reset 이후 실행을 시작한다. |
| 2. Stack Pointer 설정 | `startup_stm32f030x8.s`의 `Reset_Handler` | `_estack` 값을 Stack Pointer에 설정하여 이후 함수 실행에 사용할 Stack 환경을 준비한다. |
| 3. `.data` 초기화 | `Reset_Handler`의 `CopyDataInit`, `LoopCopyDataInit` | Flash의 초기값 저장 위치 `_sidata`에서 RAM의 `_sdata` ~ `_edata` 영역으로 초기화 데이터를 복사한다. |
| 4. `.bss` 초기화 | `Reset_Handler`의 `FillZerobss`, `LoopFillZerobss` | `_sbss` ~ `_ebss` 영역을 0으로 초기화한다. |
| 5. 기본 시스템 초기화 | `Reset_Handler`에서 `SystemInit()` 호출, 구현은 `system_stm32f0xx.c` | RCC Clock 설정을 Reset 상태에 맞추고 기본 HSI Clock 상태를 준비한다. 아직 예제의 48 MHz PLL 설정 단계는 아니다. |
| 6. 런타임 초기화 | `Reset_Handler`에서 `__libc_init_array()` 호출 | C 런타임 초기화 함수 및 정적 초기화 관련 처리를 수행한 뒤 `main()`을 호출할 준비를 한다. |
| 7. `main()` 진입 | `Reset_Handler`에서 `main()` 호출 | 공식 `GPIO_IOToggle/Src/main.c`의 Application 실행이 시작된다. |
| 8. HAL 초기화 | `main.c`의 `HAL_Init()` | HAL 및 기본 시간 기준 등의 초기화를 수행한다. |
| 9. 시스템 Clock 설정 | `main.c`의 `SystemClock_Config()` | HSI/2를 PLL 입력으로 사용해 SYSCLK 48 MHz를 설정한다. |
| 10. GPIO 및 LED 설정 | `main.c`의 GPIO Clock Enable, `HAL_GPIO_Init()` | LED2에 사용할 GPIO Clock을 활성화하고 출력 모드를 설정한다. |
| 11. 반복 동작 | `main.c`의 `while(1)` | LED2 Toggle과 `HAL_Delay(100)`을 반복 실행한다. |


`SystemInit`과 `SystemClock_Config`의 호출 위치·역할 차이:

- **`SystemInit()`:** `startup_stm32f030x8.s`의 `Reset_Handler`에서 `main()` 실행 전에 호출된다. 구현은 `system_stm32f0xx.c`에 있으며, Reset 이후 기본 시스템 Clock 관련 설정을 준비한다.
- **`SystemClock_Config()`:** 공식 예제의 `main.c`에서 `HAL_Init()` 이후 호출된다. HSI/PLL 및 Bus Clock을 설정하여 Application에서 사용할 SYSCLK 48 MHz를 구성한다.

즉, `SystemInit()`은 `main()` 진입 전의 기본 시스템 초기화이고, `SystemClock_Config()`는 `main()` 내부에서 수행하는 Application 요구 Clock 설정이다.

## 5. 내가 실행한 Host Build

| 단계 | 실제 입력 File | 실제 출력 File | Log의 명령·옵션과 역할 |
| --- | --- | --- | --- |
| Preprocess | `src/*.c`, `host/fake_hal.c`, `tests/*.c` 및 관련 Header | `build/src/*.i`, `build/host/*.i`, `build/tests/*.i` | `gcc-16 -Iinclude -Ihost -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g -E`를 사용하여 Header 포함 및 매크로 전처리 수행 |
| Compile | `build/src/*.i`, `build/host/*.i`, `build/tests/*.i` | `build/src/*.s`, `build/host/*.s`, `build/tests/*.s` | `gcc-16 -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g -S`를 사용하여 전처리된 C 코드를 Assembly로 변환 |
| Assemble | `build/src/*.s`, `build/host/*.s`, `build/tests/*.s` | `build/src/*.o`, `build/host/*.o`, `build/tests/*.o` | `gcc-16 -g -c`를 사용하여 Assembly를 Object 파일로 변환 |
| Link | `build/src/*.o`, `build/host/*.o`, `build/tests/*.o` | `build/mission-tests` | `gcc-16 -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g ... -o build/mission-tests`로 Object 파일을 연결하여 Host 실행 파일 생성 |

Host에서 검증한 내용 / ARM Source로만 확인한 내용 / Board에서 아직 확인하지 않은 내용:

**Host에서 검증한 내용**
- Flash, SRAM, GPIOA, RCC의 주소 범위 및 경계 판정
- HSI/PLL/SYSCLK 및 Bus Clock 설정값과 HAL 호출 순서
- HAL 실패 시 반환값 및 이후 초기화 중단
- GPIOA Clock Enable, PA5 초기 출력 상태 및 GPIO 설정값
- 초기화 성공 후 LED Toggle 및 `HAL_Delay(250)` 호출
- 재초기화 실패 이후 LED Toggle과 Delay 동작 차단
- 공개 테스트 319개 검사, 실패 0개 및 학생 테스트 정상 종료

**공식 ARM Source로 확인한 내용**
- `.cproject`의 Target, Include, Define, Linker 설정
- `.project`의 Source 및 HAL/CMSIS/BSP 의존성
- Linker Script의 Flash/SRAM 메모리 배치
- Startup 코드의 Vector Table, `.data`, `.bss` 초기화 과정
- `SystemInit()`과 `main()` 호출 순서
- 공식 예제의 `SystemClock_Config()` 및 LED Toggle 구현

**실제 Board에서 아직 확인하지 않은 내용**
- 실제 ARM 펌웨어 빌드 및 보드 Flash
- 실제 SYSCLK 48 MHz 동작과 PLL 안정화
- 실제 GPIO 출력과 LED 동작
- 실제 LED Toggle 간격 및 시간 오차

Host Test는 Fake HAL을 이용한 API 계약 검증이며, 실제 하드웨어 동작이나 주파수를 측정한 결과가 아니다.

## 6. Module과 초기화

네 `.c`의 책임과 호출 방향, 공개 API와 내부 상태를 설명합니다.

- **memory_map.c:** `memory_region()`에서 주소값을 Flash, SRAM, GPIOA, RCC 및 UNKNOWN으로 분류한다. 실제 메모리를 역참조하지 않으며 HAL 함수를 호출하지 않는다.
- **clock.c:** `board_clock_init()`에서 HSI, PLL, SYSCLK, AHB/APB1 Clock을 설정한다. `HAL_RCC_OscConfig()`와 `HAL_RCC_ClockConfig()`의 반환값을 확인하고 실패 시 `false`를 반환한다.
- **led.c:** `board_led_init()`에서 GPIOA Clock Enable, PA5 RESET 및 GPIO 설정을 수행한다. `board_led_toggle()`에서는 PA5만 한 번 Toggle하며 Delay는 수행하지 않는다.
- **app.c:** `app_init()`에서 HAL → Clock → LED 초기화를 순서대로 수행한다. `app_step()`에서는 초기화 성공 시에만 LED Toggle 후 250 ms Delay를 요청한다. 초기화 성공 여부는 파일 범위의 `static` 변수로 관리한다.

```text
app.c
  |
  +-- app_init()
  |     |
  |     +-- HAL_Init()
  |     +-- board_clock_init() --> clock.c --> HAL RCC
  |     +-- board_led_init() ---> led.c ----> HAL GPIO
  |
  +-- app_step()
        |
        +-- board_led_toggle() -> led.c ----> HAL GPIO
        +-- HAL_Delay(250)

memory_map.c
  |
  +-- memory_region() --> 주소 범위 판정 (HAL 호출 없음)
```

`include/mission.h`는 Module의 공개 API를 선언한다. `app.c`의 `static` 상태 변수는 해당 Source 파일 내부에서만 접근할 수 있으며, 다른 Module에 직접 공개되지 않는다.


초기화 성공·실패·재초기화 실패 시 호출 순서와 `app_step` 동작:

**1. 초기화 성공**
- `app_init()` 시작 시 내부 성공 상태를 `false`로 초기화한다.
- `HAL_Init()` 성공 → `board_clock_init()` 성공 → `board_led_init()` 수행
- 내부 성공 상태를 `true`로 변경하고 `true`를 반환한다.
- 이후 `app_step()` 호출 시 LED Toggle 1회와 `HAL_Delay(250)`을 수행한다.

**2. 초기화 실패**
- `HAL_Init()` 실패 시 Clock 및 LED 초기화를 수행하지 않고 `false`를 반환한다.
- `board_clock_init()` 실패 시 LED 초기화를 수행하지 않고 `false`를 반환한다.
- 실패 상태에서는 `app_step()`을 호출해도 LED Toggle과 Delay를 수행하지 않는다.

**3. 재초기화 실패**
- 이전 `app_init()`이 성공했더라도 새로운 `app_init()` 호출 시작 시 내부 성공 상태를 `false`로 변경한다.
- 이후 HAL 또는 Clock 초기화가 실패하면 `false`를 반환하고 성공 상태를 복구하지 않는다.
- 따라서 이전 성공 기록이 남아 있더라도 `app_step()`은 아무 동작도 하지 않는다.
- 학생 테스트에서는 최초 초기화 성공 후 `HAL_BUSY`를 주입하여 재초기화를 실패시키고, `toggle_count == 0`, `delay_count == 0`을 검증했다.