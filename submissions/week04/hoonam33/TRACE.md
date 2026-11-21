# 4주차 Datasheet · Build · Startup 추적

## 1. Datasheet 근거와 MCU 관계

| 구현한 값 | Datasheet 페이지·표·그림 또는 BSP 근거 | C File·함수와 사용 의미 |
| --- | --- | --- |
| Flash 영역 | DS9773 Figure 10 |주소별 메모리 영역 반환 |
| SRAM 영역 |DS9773 Figure 10 |주소별 메모리 영역 반환 |
| GPIOA 영역 / LED2 Port·Pin |DS9773 Figure 10 |주소별 메모리 영역 반환, GPIOA의 Pin 5의 초기화 및 동작 |
| RCC 영역 |DS9773 Figure 10 |주소별 메모리 영역 반환 |
| HSI·PLL·SYSCLK·Bus Clock |DS9773 Figure 2 |분주 가능 clock확인 |

CPU·Flash·SRAM·RCC·GPIOA 관계도를 아래에 작성합니다. 주소 접근과 Clock 공급·Enable 화살표의 의미를 구분하세요.

```text
RCC ---> CPU
    ------ㄴ-------> Flash
    ------ㄴ-------> sRAM
    ------ㄴ-------> GPIOA
```

Clock 계산식과 공개 예제에서 선택한 경로:
HSI / 2 * 12 = 48MHz


## 2. 공개 예제와 제품 요구의 차이

| 항목 | 공개 예제의 설정·근거 | 이번 제품의 요구·변경한 C 위치 |
| --- | --- | --- |
| Clock |OscillatorType은 NONE 사용, RCC 설정이 실패하면 Error_Handler()를 호출하여 무한 루프에 머무름 |HSI를 설정 대상으로 선택하고 HSI ON·기본 Calibration을 명시,두 HAL 호출에서 HAL_OK 이외의 결과를 받으면 false 반환. OscConfig 실패 시 ClockConfig를 호출하지 않음 |
| LED 초기 상태·Mode·Pull·Speed |초기상태 명시X, Output Push-Pull, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH |GPIO_PIN_RESET(OFF), Output Push-Pull, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW |
| Toggle 간격 |main() 무한루프에서 토글 후 100ms 딜레이 |app.c의 app_step()한번 마다 토글 후 250ms 딜레이. |
| 실패 처리 |클럭 설정 실패 시, error_handler()를 호출. |app_init()이 HAL, CLOCK 실패 시 초기화 중단. 호출마다 성공상태를 해제하여 초기화 실패 시 반복동작 차단 |

## 3. 공개 STM32 Project의 Build

| 확인할 내용 | 실제 File·설정·근거 |
| --- | --- |
| Target / Compiler 계열 / Version 명시 여부 |.cproject: MCU STM32F030R8Tx, Board NUCLEO-F030R8, Ac6 STM32 MCU GCC, arm-none-eabi-. 정확한 컴파일러 버전은 명시되지 않음 |
| Source 입력 / Include / Define |.project의 연결 소스는 아래 목록. .cproject의 Include는 예제 Inc, CMSIS Core·Device Include, HAL Inc, Nucleo BSP 경로. Define은 STM32F030x8, USE_HAL_DRIVER, USE_STM32F0XX_NUCLEO |
| Startup Object / Linker script / Library |Startup 입력은 startup_stm32f030x8.s이며 어셈블 후 오브젝트로 링크됨. 스크립트는 ../STM32F030R8Tx_FLASH.ld. 추가 링크 옵션은 -specs=nosys.specs -specs=nano.specs. 명시적인 추가 -l·-L 값은 없음 |
| 실행 Image와 후처리 산출물 |설정상 STM32F030R8-Nucleo.elf 생성. 이후 arm-none-eabi-objcopy -O binary로 .bin 생성, arm-none-eabi-size로 크기 출력. 실제 ARM 빌드는 미실행 |

Header를 포함하는 것과 `.c`를 Build 입력에 넣는 것의 차이:

헤더 포함은 전처리 단계에서 함수 선언, 자료형, 매개변수를 제공하고 .c 파일을 빌드 입력에 넣으면 함수 구현이 컴파일되어 .o 파일이 생성된다. 이후 다른 파일에서 호출한 함수의 구현은 링크 단계에서 연결된다.

## 4. 공개 예제의 Reset부터 main까지

| 실제 순서 | File·함수·label | 준비하는 상태와 다음 단계의 관계 |
| --- | --- | --- |
|1 |Startup의 g_pfnVectors |첫 항목은 초기 SP인 _estack, 두 번째는 Reset 진입 주소인 Reset_Handler |
|2 |Reset_Handler |_estack을 SP에 설정하여 스택 준비
|3 |CopyDataInit / LoopCopyDataInit |Flash의 _sidata에서 SRAM의 _sdata~`_edata범위로.data` 초기값 복사
|4 |FillZerobss / LoopFillZerobss |SRAM의 _sbss~`_ebss` 범위를 0으로 채움
|5 |SystemInit() |RCC를 HSI 기반 기본 상태로 정리
|6 |__libc_init_array() |런타임 초기화 배열·생성자 처리를 위한 함수 호출
|7 |main() |C Application으로 진입. 공식 예제는 HAL 초기화 → 시스템 클록 설정 → GPIO 초기화 → 반복 동작 수행

`SystemInit`과 `SystemClock_Config`의 호출 위치·역할 차이:

SystemInit(): main() 이전 Startup에서 호출. HSI 활성화와 PLL·분주 설정 초기화 등 RCC를 기본 상태로 정리합니다.

SystemClock_Config(): 공식 예제의 main()에서 HAL_Init() 다음에 호출. HSI/2 ×12, PLL SYSCLK, 버스 /1, Flash latency 1로 제품 동작 클록을 설정합니다.

## 5. 내가 실행한 Host Build

| 단계 | 실제 입력 File | 실제 출력 File | Log의 명령·옵션과 역할 |
| --- | --- | --- | --- |
| Preprocess |src/app.c와 포함 헤더 |build/src/app.i |gcc … -E src/app.c -o build/src/app.i: 헤더 포함·매크로·조건부 컴파일 처리 |
| Compile |build/src/app.i |build/src/app.s |gcc … -S build/src/app.i -o build/src/app.s: 전처리된 C 코드를 Host 어셈블리로 변환 |
| Assemble |build/src/app.s |build/src/app.o |gcc -g -c build/src/app.s -o build/src/app.o: 오브젝트 생성 |
| Link |.o |build/mission-tests.exe |gcc … [오브젝트 목록] -o build/mission-tests.exe: 함수·심볼을 연결하고 실행 파일 생성 |

Host에서 검증한 내용 / ARM Source로만 확인한 내용 / Board에서 아직 확인하지 않은 내용:

Host에서 make test로 공개 검사 319개와 학생 테스트의 통과를 확인했고, make -B stages로 전처리·컴파일·어셈블·링크를 실행했다. 두 명령의 종료 코드는 0이며 중간 파일 21개와 Windows PE32+ 실행 파일을 확인했다. 공식 ARM 프로젝트의 메모리 배치와 Startup 순서는 소스를 읽어 확인했다. 실제 ARM 빌드·보드 실행·PLL Lock·전기적 LED 상태·실제 시간 정확도는 검증하지 않았다.

## 6. Module과 초기화

네 `.c`의 책임과 호출 방향, 공개 API와 내부 상태를 설명합니다.

- memory_map.c:주소입력 시, 메모리 영역에 대한 값 반환.
- clock.c:
클럭 설정을 담당.

board_clock_init()
  → HAL_RCC_OscConfig()
      실패: false 반환
      성공: HAL_RCC_ClockConfig()
              실패: false 반환
              성공: true 반환

- led.c:LED의 포트,핀과 GPIO 설정을 담당.

board_led__init()은 GPIOA 클럭 활성화 및 설정값 초기화
board_led_toggle()은 PA5의 1회 반전

board_led_init()
  → __HAL_RCC_GPIOA_CLK_ENABLE()
  → HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)
  → HAL_GPIO_Init(GPIOA, &gpio)

- app.c: HAL, clock, led 초기화 및 확인을 담당.

app_init()은 호출할 때마다 init을 false로 변경한 뒤, HAL, CLOCK, LED 순서로 초기화 함. HAL과 CLOCK의 초기화 실패 시 FALSE반환. 모든 단계 완료 시 init을 true로 변경하고 true반환

app_step()은 init이 true일 때만 led를 한번 토글하고 250ms 딜레이 함. 초기화 전이나 실패 시에는 동작하지 않음.

초기화 성공·실패·재초기화 실패 시 호출 순서와 `app_step` 동작:

app_init()
  → init = false
  → HAL_Init()
      실패: false 반환
  → board_clock_init()
      실패: false 반환
  → board_led_init()
  → init = true
  → true 반환

app_step()
  → init이 false: 동작 없음
  → init이 true: board_led_toggle() → HAL_Delay(250)
