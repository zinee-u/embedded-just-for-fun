# 05회차 - TC387QP 주기 수집 기능 Host Starter

TC387QP의 STM과 Port 동작을 학습하기 위한 **Single-core Polling 모델**입니다. `platform.h`와 `fake_platform`은 이 과제용 Host Adapter이며 Infineon iLLD, TC387QP SFR Header 또는 실제 Firmware가 아닙니다. MCU의 MMIO 주소를 PC에서 역참조하지 않습니다. 실제 MCU·Board 근거와 PR 제출 방법은 상위 과제 문서를 확인하세요.

## 수정할 파일

| 파일 | 책임 |
| --- | --- |
| `src/timebase.c` | 주어진 STM Frequency로 ms를 Tick으로 변환하고, Wrap을 고려하여 주기 도달을 판정 |
| `src/board_io.c` | Active LOW LED 초기화·Toggle, Active LOW 수집 Enable 입력 읽기 |
| `src/collector.c` | 초기화 상태, 10 ms 수집, 250 ms LED Toggle을 독립적으로 관리 |
| `tests/test_student.c` | 경계·실패 상황 Test 최소 2개 추가 |
| `BUILD_ENV.md`, `TIMING.md` | 실제 환경·Timing 근거·실행 결과 기록 |

`include/*`, `host/*`, `tests/test_mission.c`, `tests/test_support.h`, `Makefile`은 고정 지원 코드입니다. Test를 통과시키기 위해 고정 계약을 수정하지 마세요. Starter는 Compile되지만 요구사항을 만족하지 않는 검토용 초안입니다. 증상의 근본 원인을 모두 찾아 세 개의 `.c` 파일을 수정하세요.

## 함수 계약

- `ticks_from_ms(hz, ms, out)`: `hz × ms / 1000`으로 Tick을 계산합니다. `NULL`, 0 Hz, 0 ms, 1 Tick 미만, 정수가 아닌 Tick, `INT32_MAX` 초과는 `false`입니다. 실패하면 `*out`은 호출 전 값을 유지합니다. 두 `uint32_t` 입력의 곱이 32-bit 범위를 넘을 수 있습니다.
- `period_due(now, period, last)`: `last == NULL`, `period == 0`, `period > INT32_MAX`이면 `false`이며 값을 바꾸지 않습니다. 실제 경과 Tick이 주기 이상이면 `true`를 반환하고 `*last = now`로 재시작합니다. 아직 도달하지 않았으면 `false`이고 `*last`를 보존합니다. 정확한 경계에서도 실행합니다. 지연 시 과거 횟수를 따라잡지 않고 현재 시각에 한 번만 실행합니다.
- `board_io_init()`: LED 출력 Latch를 HIGH로 만들어 OFF 상태를 준비한 다음 Push-pull Output으로 전환합니다. 이어서 수집 Enable을 Input Pull-up으로 설정합니다. 각 기능과 관계없는 Port Bit를 보존합니다.
- `board_collection_enabled()`: `PORT_ENABLE`의 `ENABLE_MASK` 입력을 한 번 읽어 LOW이면 `true`, HIGH이면 `false`를 반환합니다.
- `board_led_toggle()`: `PORT_LED`의 `LED_MASK`에 해당하는 출력만 한 번 Toggle합니다. 다른 Bit나 다른 Port를 바꾸지 않습니다.
- `collector_init(state, stm_hz, now)`: `NULL`이면 `false`입니다. 그 외 모든 State 필드를 먼저 0/`false`로 초기화합니다. 10 ms와 250 ms를 각각 검증하여 둘 다 성공할 때만 State에 반영하고 GPIO를 초기화한 뒤 `ready = true`로 만듭니다. 두 기준 시각은 `now`, 누적 수집 수와 마지막 값은 0입니다. 실패하면 모든 State 필드가 0/`false`이며 GPIO 호출이 없습니다. 재초기화도 같은 계약입니다.
- `collector_step(state, now)`: `NULL` 또는 `!ready`이면 아무 Platform 호출도 하지 않습니다. 정상 호출마다 Enable을 정확히 한 번 읽습니다. 수집 주기를 먼저 갱신하고, 도달했으며 Enabled일 때만 Sensor를 한 번 읽어 `sample_count`를 1 증가시키고 `last_value`에 저장합니다. Disabled 상태에서도 도달한 수집 기준 시각을 갱신합니다. LED 주기는 독립적으로 판정하며 Disabled 중에도 계속 Toggle합니다. 최초 `now`와 같은 시각의 Step에서는 수집·Toggle하지 않습니다. Delay, Sleep, Busy-wait, UART 출력은 정상 Step에 넣지 않습니다.

`CollectorState`는 Single-core의 한 실행 흐름에서만 접근하며 Interrupt·다른 Core가 동시에 변경하지 않는 것이 전제입니다. `now`는 STM Counter의 하위 32-bit Snapshot을 전달한 값으로 가정합니다. 실제 시간은 앞으로 진행하고, 각 기준 시각부터 판정까지 32-bit Counter 한 바퀴 전체가 지나지 않아야 합니다. 100 MHz에서는 한 바퀴가 약 42.95초이므로 그 이상 관측을 중단하면 이 API만으로 경과 시간을 복원할 수 없습니다. `INT32_MAX` 상한은 이번 API 계약이며 한 바퀴 누락을 자동 해결하지 않습니다. 시험에서는 총 수집 횟수가 `2^32`보다 작습니다.

Sensor 값은 0~4095의 `uint16_t` 모의 값입니다. Sensor 통신 실패, Debounce, Interrupt, STM Compare Interrupt, Multi-core 동기화는 이번 구현 범위 밖입니다. STM Frequency는 초기화 입력입니다. 기본 Test의 100 MHz는 CPU Clock 또는 TC387QP의 모든 설정에서 고정되는 주파수라는 뜻이 아닙니다. 200 MHz는 주파수 Hardcode를 검출하기 위한 Generic Host Arithmetic Input이며, TC387QP에서 허용되는 STM Clock 설정값이라고 주장하는 것이 아닙니다.

## Host에서 실행

C11 Compiler와 Make가 필요합니다. 이 폴더에서 실행합니다.

```sh
make
make test
make demo
```

- `make`: Test와 Demo를 Compile·Link합니다. Starter에서도 성공해야 합니다.
- `make test`: 최초 Starter는 의도적으로 FAIL입니다. 구현 후 고정 Test와 추가 Test를 모두 통과해야 합니다. `CHECK`는 실패를 누적하며 Test 실행 파일은 실패가 있으면 Exit 1을 반환합니다. 이 실패를 보고받은 `make` 자체는 Exit 2를 반환할 수 있습니다.
- `make demo`: 정해진 시각과 Enable 입력을 순서대로 넣고 CSV 결과를 출력합니다. 실제로 500 ms를 기다리거나 MCU 전기 신호를 측정하지 않습니다. 정답으로 수정하기 전에는 결과도 잘못될 수 있습니다.
- `make -s demo > timing.csv`: 명령 Echo를 제외하고 Header부터 시작하는 CSV를 저장합니다.
- Compiler나 Flag를 변경하면 `make clean` 후 다시 Build하세요. `build/`는 제출하지 않습니다.

Windows 10에서는 C Compiler와 Make를 별도로 준비해야 합니다. MSYS2/MinGW 등의 환경에서 `make CC=gcc EXEEXT=.exe test`와 `make CC=gcc EXEEXT=.exe demo`를 사용할 수 있습니다. Git Bash만 설치하면 C Compiler가 자동 설치되는 것은 아닙니다. 이 자료 작성 시 macOS에서 Host Test를 실행했으며 Windows에서는 아직 실행하지 않았습니다. 제출자는 본인이 실제로 실행한 환경을 `BUILD_ENV.md`에 기록하세요.

## Host Adapter 읽는 법

`PORT_LED`와 `PORT_ENABLE`은 각각 별도 논리 Port ID입니다. 두 Mask의 Bit 0도 교육용 번호이며 실제 TC387QP Pin 번호가 아닙니다. 실제 보드 Mapping은 상위 과제의 Hardware 문서에서 확인합니다. `output`, `direction`, `pullup`, `input`은 Port를 단순화한 32-bit Host 메모리 Image이며 실제 Register 배열이 아닙니다.

`fake_platform_reset()`은 Adapter 상태만 초기화합니다. Collector State는 학생 코드의 `collector_init()`에서 관리합니다. `fake_platform_clear_log()`는 호출 Log와 Count만 지우고 Port Image, Input, Sensor 값을 보존합니다. `fake_set_enabled(true)`는 외부에서 LOW를 입력하는 상황을 만듭니다. `platform_delay_ms()`는 기존 초안의 지연 요청을 기록하기 위한 함수이며 Host에서는 실제 대기하지 않습니다. 최종 `collector_step()`에서는 호출하지 않아야 합니다.

이 과제는 Input을 Polling하고 두 작업을 Non-blocking으로 분리하는 연습입니다. 실제 Board의 Startup, Watchdog/ENDINIT, STM Clock 설정, Pad 전압, 외부 저항·배선, 읽기 시각의 정확도는 이 Host Test로 검증되지 않습니다.
