# Week 04 — MCU 요구값을 C 코드에 반영하기

이 폴더는 **STM32F030R8 / NUCLEO-F030R8용 요구사항을 구현하는 교육용 TODO Starter**입니다. STM32CubeF0의 공식 `GPIO_IOToggle` 예제를 참고하여 새로 작성했습니다. 공식 예제 원본이나 완성 Firmware를 그대로 복사한 자료가 아닙니다.

제출할 구현은 `src/*.c`의 Diff로 검토합니다. `include/mission.h`의 Public API와 고정 Test를 유지하고, `tests/test_student.c`에 최소 2개의 경계 또는 실패 Case를 추가하세요. Fork·Branch·PR 제출 경로는 이 주차 과제의 `SUBMISSION.md` 안내를 따르세요.

## 파일별 구현 요구사항

| 파일 | 함수 | 코드에 반영할 요구사항 |
|---|---|---|
| `src/memory_map.c` | `memory_region(uint32_t address)` | 아래 4개 영역의 시작·마지막 주소를 포함하여 분류. 그 외 `MEMORY_UNKNOWN`. 단일 주소 숫자만 판정하며 Pointer 역참조 금지. |
| `src/clock.c` | `board_clock_init()` | HSI 8 MHz / 2 × 12 = 48 MHz. HSI ON, 기본 Calibration, PLL ON, PREDIV DIV1. AHB/APB1 DIV1. `HAL_RCC_OscConfig` → `HAL_RCC_ClockConfig(..., FLASH_LATENCY_1)`. 어느 호출이든 `HAL_OK` 이외면 즉시 `false`. |
| `src/led.c` | `board_led_init()` | GPIOA Clock Enable → `HAL_GPIO_WritePin`으로 PA5 RESET → Output PP / No Pull / Low Speed 설정. 초기 LED OFF. |
| `src/led.c` | `board_led_toggle()` | `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)`으로 PA5만 한 번 Toggle. Delay는 이 함수의 책임이 아님. |
| `src/app.c` | `app_init()` | 매 호출 시작 시 성공 상태 해제. `HAL_Init` → `board_clock_init` → `board_led_init`. HAL/Clock 실패 시 이후 초기화를 중단하고 `false`. 모두 성공하면 `true`. |
| `src/app.c` | `app_step()` | 초기화 성공 상태에서만 LED Toggle 1회 → `HAL_Delay(250)`. 성공 상태를 Module 내부에서 관리. 실패 뒤 호출하면 아무 HAL 동작도 하지 않음. |
| `tests/test_student.c` | `run_student_tests()` | 최소 2개의 경계/실패 Case를 `assert`로 작성. 요구사항·입력·예상 결과·검증 이유를 주석으로 기록. |

`RCC_OscInitTypeDef`, `RCC_ClkInitTypeDef`, `GPIO_InitTypeDef`는 초기화한 뒤 필요한 필드를 채우세요. `HAL_ERROR`, `HAL_BUSY`, `HAL_TIMEOUT`은 모두 실패입니다. `app_init()` 실패 시 기존 Hardware 설정을 되돌리는 기능은 이번 범위에 포함하지 않으며, 실패 이후 새 GPIO 초기화나 `app_step()` 동작을 진행하지 않는 것이 계약입니다.

| 영역 | 첫 주소 | 마지막 주소 | 반환값 |
|---|---|---|---|
| Flash, 64 KiB | `0x08000000` | `0x0800FFFF` | `MEMORY_FLASH` |
| SRAM, 8 KiB | `0x20000000` | `0x20001FFF` | `MEMORY_SRAM` |
| GPIOA, 1 KiB Window | `0x48000000` | `0x480003FF` | `MEMORY_GPIOA` |
| RCC, 1 KiB Window | `0x40021000` | `0x400213FF` | `MEMORY_RCC` |

영역 판정은 모든 Offset의 Register 접근 가능 여부를 보장하지 않습니다. 예약 주소도 해당 Window 안에서는 같은 영역으로 분류합니다. Boot Alias `0x00000000`, 다른 GPIO Port 등 위 표에 없는 주소는 이 API에서 `MEMORY_UNKNOWN`입니다.

PA5는 Active HIGH LED입니다. `app_step()`을 연속 호출하면 명목상 250 ms마다 Toggle하므로 ON/OFF 한 주기는 약 500 ms입니다. Host Test의 `HAL_Delay`는 시간 값을 기록할 뿐 실제로 기다리지 않습니다. 실제 보드의 실행 시간이나 오차를 측정한 값이 아닙니다.

## Host에서 실행

C11 Compiler(`cc` 또는 `gcc`)와 GNU Make가 필요합니다. 이 폴더로 이동한 뒤 실행합니다.

```sh
make
make test
make stages
```

- `make`: TODO 상태에서도 Compile·Link가 성공해야 합니다.
- `make test`: TODO가 남은 최초 상태에서는 의도적으로 FAIL입니다. 구현 후 모든 Public Check와 학생 Test를 통과해야 합니다.
- `make stages`: `build/src/*.i`, `*.s`, `*.o`와 `build/mission-tests`를 생성합니다. Host Compiler의 Preprocess·Compile·Assemble·Link 산출물을 확인하세요. `make test`와 `make stages`는 모두 `.c → .i → .s → .o → Host Executable` 의존성을 사용합니다. 생성된 `.i`를 Compile하고, 생성된 `.s`를 Assemble하며, 중간 파일은 자동 삭제하지 않습니다.
- `build/`는 `.gitignore`에 포함되어 있습니다. Binary나 중간 산출물을 Commit하지 마세요.
- Compiler나 Flag를 바꾸면 `make clean` 후 다시 Build하세요.

Windows 10에서는 Git Bash만 설치해도 `make`와 `gcc`가 자동 제공되는 것은 아닙니다. 설치된 Compiler/Make를 확인하고, 필요한 경우 MSYS2/MinGW 개발 환경 등에서 실행하세요. 해당 도구가 준비된 Bash에서는 다음과 같이 Compiler와 확장자를 지정할 수 있습니다. Windows 실행은 이번 자료 작성 환경에서 검증하지 않았습니다.

```sh
gcc --version
make --version
make CC=gcc EXEEXT=.exe test
make CC=gcc EXEEXT=.exe stages
```

Test 출력을 PR에 남길 때는 성공 문구와 Exit Status를 함께 확인하세요. Bash에서 Pipe 없이 저장하는 예입니다.

```sh
make test > test.log 2>&1
result=$?
cat test.log
printf 'Exit status: %s\n' "$result"
```

## 고정 지원 코드와 검증 범위

`include/mission.h`, `host/*`, `tests/test_mission.c`, `Makefile`은 고정 지원 코드입니다. 결과를 맞추기 위해 수정하면 안 됩니다. 학생은 네 개의 `src/*.c`, `tests/test_student.c`, 제출 문서를 변경합니다.

`host/stm32f0xx_hal.h`는 이 과제에서 사용하는 STM32F030x8 HAL API 일부의 이름·형식을 제공하는 Test Adapter입니다. `GPIOA`는 Host의 일반 메모리 객체이며 실제 MCU 주소가 아닙니다. `fake_hal`은 함수 인자, 호출 순서, PA5 출력 상태, 실패 Return을 기록합니다. `fake_hal_reset()`은 Adapter 기록만 초기화하며 `app.c`의 내부 상태는 초기화하지 않습니다.

Host Test는 `.data/.bss` 초기화, 실제 Clock Lock, Interrupt, SysTick 주기, 전기적 LED 동작을 실행하지 않습니다. Host Executable의 형식도 OS에 따라 Mach-O / PE / ELF로 달라집니다. `make stages`는 MCU Firmware Build가 아닙니다. 실제 ARM Startup과 Linker는 고정된 공식 예제에서 별도로 추적하고 근거를 제출 문서에 적으세요. Target Build에서는 이 Host Header/Adapter를 사용하지 않습니다.

## 공식 근거

- [STM32F030R8 Datasheet, DS9773](https://www.st.com/resource/en/datasheet/stm32f030r8.pdf): Table 2, Figure 2, Figure 10, Table 17. 자료 기준 DS9773 Rev 5.
- [공식 GPIO_IOToggle 예제 main.c](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c): STM32CubeF0 v1.11.5 Commit `e220bfb12a162cfbf3bb65663e987e0d6550e9b4`.
- [공식 SW4STM32 Startup](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/SW4STM32/startup_stm32f030x8.s).
- [공식 HAL RCC Header](https://github.com/STMicroelectronics/stm32f0xx_hal_driver/blob/115eb1dc87e26ea29f4f2ca58003650381aabba2/Inc/stm32f0xx_hal_rcc.h): API의 구조체·상수 정의.

이번 제품 요구는 공식 예제와 차이가 있습니다. 공식 예제는 Delay 100 ms, Pull-up, High Speed를 사용하지만, 제출 코드는 **Delay 250 ms, No Pull, Low Speed, 초기 출력 OFF**를 구현해야 합니다. HSI 설정과 실패 Return 처리도 이번 API 계약에 맞춰 명시하세요.

## 제출자가 작성할 설명

- GitHub 아이디:jihyunsgithub
- 실제 PR URL: PR 생성 후 기입

### 모든 근본 원인과 변경 이유

| 요구사항·실패 상황 | 초안이 놓친 근본 원인 | 수정한 C File·함수 | 실제 검증 결과 |
| --- | --- | --- | --- |
| Memory 범위·경계 |초기 TODO 구현에서는 모든 주소를 MEMORY_UNKNOWN으로 반환하여 Flash, SRAM, GPIOA, RCC 영역을 구분하지 못했다.|src/memory_map.c의 memory_region()에서 각 영역의 시작 주소와 마지막 주소를 포함하는 범위 조건을 구현했다. 지정되지 않은 주소는 MEMORY_UNKNOWN으로 반환한다.|공개 테스트 M1 통과. 주소 경계 및 영역 분류 검사에서 실패가 보고되지 않았다.|
| Clock 값·실패 전달 |초기 TODO 구현은 false만 반환하여 HSI, PLL, SYSCLK 설정과 HAL 실패 처리를 수행하지 않았다.|src/clock.c의 board_clock_init()에서 HSI 8 MHz / 2 × 12 = 48 MHz 설정, AHB/APB1 DIV1, FLASH_LATENCY_1을 지정했다. Oscillator 설정 후 Clock 설정을 호출하며, HAL 반환값이 HAL_OK가 아니면 즉시 false를 반환한다.|공개 테스트 C1 통과. Clock 설정값, HAL 호출 순서, 실패 반환 처리에서 실패가 보고되지 않았다.|
| LED 설정·순서 |초기 TODO 구현에서는 GPIOA Clock 활성화, LED 초기 출력값 설정, GPIO 출력 모드 설정 및 Toggle 기능이 구현되지 않았다.|src/led.c의 board_led_init()에서 GPIOA Clock Enable → PA5 RESET → Push-Pull/NOPULL/LOW 설정 순서를 구현했다. board_led_toggle()에서는 PA5만 Toggle하도록 구현했다.|공개 테스트 L1/L2 통과. GPIO 설정값, 초기 OFF 상태, 호출 순서 및 PA5 Toggle 동작에서 실패가 보고되지 않았다.|
| 초기화·상태 수명·반복 동작 |초기 TODO 구현에서는 Application 초기화가 항상 실패하고, LED 반복 동작도 수행하지 않았다. 또한 초기화 성공 여부를 관리할 상태가 필요했다.|src/app.c의 app_init()에서 매 호출 시 성공 상태를 해제하고 HAL → Clock → LED 순서로 초기화하도록 구현했다. app_step()에서는 초기화가 성공한 경우에만 LED Toggle 후 250 ms Delay를 요청한다. 성공 상태는 파일 범위의 static 변수로 관리한다.|공개 테스트 A1/A2 통과. 초기화 성공·실패, 실패한 재초기화 이후 동작 차단, Toggle 및 Delay 호출 검증에서 실패가 보고되지 않았다.|

### 본인이 추가한 Test

**Case 1 — SRAM 하한 경계 검사**

- **요구사항:** 지정된 SRAM 영역 밖의 주소는 `MEMORY_UNKNOWN`으로 분류한다.
- **입력:** `0x1FFFFFFF` (SRAM 시작 주소 `0x20000000` 바로 이전)
- **예상값:** `MEMORY_UNKNOWN`
- **실제값:** `MEMORY_UNKNOWN` (학생 테스트의 `assert` 통과)
- **추가 이유:** SRAM 시작 주소보다 작은 값이 잘못 SRAM 영역에 포함되지 않는지 검증하기 위해 추가했다.

**Case 2 — 실패한 재초기화 이후 Application 동작 차단**

- **요구사항:** 이전 초기화가 성공했더라도 재초기화에 실패하면 이후 `app_step()`이 LED Toggle과 Delay를 수행하지 않아야 한다.
- **입력:** 첫 번째 `app_init()` 성공 후 `fake_hal_reset()`을 호출하고, `fake_hal.hal_result = HAL_BUSY`를 설정하여 재초기화 실패를 주입한다.
- **예상값:** 두 번째 `app_init()`은 `false`를 반환하고, 이후 `app_step()`을 호출해도 `toggle_count == 0`, `delay_count == 0`이어야 한다.
- **실제값:** 두 번째 `app_init()`이 `false`를 반환하고, `toggle_count == 0`, `delay_count == 0`에 대한 `assert`가 모두 통과했다.
- **추가 이유:** `fake_hal_reset()`은 Application 내부의 `static` 상태를 초기화하지 않으므로, 실패한 재초기화에서 이전 성공 상태가 해제되는지 검증하기 위해 추가했다.


### 실행 결과

- make test 결과와 Exit code:Public checks: 319, failures: 0, RESULT: PASS, Exit code 0
- make stages 결과와 Exit code:전처리(.i), 어셈블리(.s), 목적 파일(.o) 및 Host 실행 파일 생성 성공, Exit code 0
- 실제 Log: [test](logs/test.log), [stages](logs/stages.log)
- 미확인 사항:실제 ARM Target Build 및 STM32F030R8 보드 실행은 수행하지 않았다. 따라서 실제 SYSCLK 48 MHz 동작 여부, PLL 안정화, Flash 접근 타이밍, LED 점멸 주기와 오차는 측정하지 않았다. Host Fake HAL의 HAL_Delay()는 실제로 대기하지 않고 요청한 시간만 기록한다.

### Review에서 설명할 내용

문제의 원인 → C 코드에서 바꾼 내용 → 근거 → 실행 검증 순서로 작성합니다.

1. 문제의 원인

초기 Starter에는 Memory Map 분류, Clock 초기화, LED 제어, Application 실행 기능이 TODO로 남아 있었다. 따라서 각 요구사항에 필요한 주소 범위 판정, HAL 설정값, 함수 호출 순서 및 실패 처리가 구현되지 않은 상태였다.

2. C 코드에서 바꾼 내용

memory_map.c에는 주소 범위 분류를 구현하고, clock.c에는 48 MHz Clock 설정과 실패 처리를 구현했다. led.c는 LED 초기화와 Toggle만 담당하도록 구성했으며, app.c는 초기화 순서와 반복 동작 및 성공 상태를 관리하도록 구현했다.

3. 구현 근거

DS9773 Rev 5의 Table 2, Figure 2, Figure 10, Table 17을 참고하여 STM32F030R8의 메모리 용량, Clock 경로 및 주변장치 주소를 확인했다.

STM32CubeF0 v1.11.5의 GPIO_IOToggle 예제와 STM32F0xx Nucleo BSP를 참고하여 LED의 포트·핀 연결 및 HAL 사용 방식을 확인했다.

이번 과제의 제품 요구사항에 따라 공식 예제와 다른 NOPULL, LOW Speed, 초기 OFF, 250 ms Delay를 적용했다.

Host HAL 헤더 및 Fake HAL 구현을 확인하여 HAL API의 인자, 호출 기록과 테스트 구조를 이해했다.

4. 실행 검증

macOS 14.5 / Apple M1 환경에서 GNU GCC 16.2.0과 GNU Make 3.81을 사용했다.

make CC=gcc-16 test 실행 결과 공개 테스트 319개 검사에서 실패 0개를 확인했으며, 직접 작성한 학생 테스트도 정상 종료되었다. make CC=gcc-16 stages도 Exit code 0으로 완료되었다.

이는 Host API 계약에 대한 검증 결과이며 실제 MCU 하드웨어에서의 동작 검증은 아니다.