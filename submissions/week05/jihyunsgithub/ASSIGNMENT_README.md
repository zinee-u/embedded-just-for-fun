# 05회차 TC387QP 주기 수집 요청서

**주제: GPIO 입출력 · STM Timer · Polling · 주기 수집**

스누피는 TC387QP 기반 상태 수집 장치의 Firmware 개선을 요청합니다. 기존 시제품은 센서 값을 읽은 뒤 기다리는 동안 입력 확인과 LED 동작도 함께 늦어집니다. 제공된 C 초안의 모든 결함을 찾아 수정하고, 요구사항을 만족하는 코드와 실제 Host test 결과를 Pull Request로 제출해 주세요.

## 학습 목표

1. GPIO 입출력 설정과 Timer의 주기 계산을 요구사항에 연결한다.
2. Polling 방식으로 모의 센서 값을 주기적으로 수집하는 흐름을 만든다.
3. 대기 시간이 다른 작업에 미치는 영향을 살펴보고, 실측값과 예상 Timing을 구분한다.

각 목표는 `board_io.c`, `timebase.c`, `collector.c`의 변경과 `TIMING.md`의 근거로 확인합니다. **학습 목표 → 공식 자료의 위치와 조건 → 확인한 사실 → 설계 판단 → Test**가 이어지도록 작성해 주세요. 문서에 답만 적고 C 코드가 그대로인 제출은 완료로 인정하지 않습니다.

## 공개 해설을 활용하는 순서

**05회차는 예외적으로 [한국어 해설 코드](reference/README.md)를 함께 공개합니다.** 주석과 구현을 직접 읽으며 공부한 뒤, 기존과 같은 방식으로 원본 Starter의 개선안을 Pull Request로 제출합니다. 다른 회차의 해설 공개 정책이 바뀌는 것은 아닙니다.

1. 이 요청서의 요구사항과 [공개 해설](reference/README.md)을 읽고, 각 함수가 어떤 실패를 막는지 확인합니다.
2. 개선 전 [Starter](starter/)에서 실패 입력이나 호출 순서를 직접 선택합니다. 예상 결과와 실제 결과를 비교하고, 실패 원인·수정 이유·공식 자료 근거를 기록합니다.
3. `starter/`를 자신의 `submissions/week05/YOUR_GITHUB_ID/`로 복사한 뒤 세 C Module을 개선합니다. `weeks/week05/starter/`와 `weeks/week05/reference/`는 수정하지 않습니다.
4. 해설의 Test를 그대로 복사하는 대신 자신이 확인하려는 조건을 골라 `tests/test_student.c`에 Test를 최소 2개 추가합니다. 왜 그 입력을 골랐는지, 무엇이 실패해야 하고 무엇이 통과해야 하는지 설명합니다.
5. 최종 코드로 실행한 Log와 모든 근본 원인에 대한 설명을 남기고, 원본 저장소의 `main`으로 PR을 보냅니다.

구현을 작성하거나 검토할 때 공개 해설을 계속 참고해도 됩니다. 코드의 모양이 해설과 같거나 다르다는 사실만으로 완료를 판단하지 않습니다. **자신이 선택한 입력, 수정 이유, 근거와 실제 Test 결과로 동작을 설명하는 것**이 제출의 핵심입니다.

## 근거를 찾고 기록하는 방법

먼저 [공식 자료 안내](REFERENCES.md)에서 자료 ID와 Version을 확인하세요. Datasheet의 사양, Board의 연결, User Manual의 Register 동작, 예제의 실제 설정을 구분해 읽습니다. 같은 계열의 다른 제품이나 다른 Board Revision의 내용을 그대로 적용하지 마세요.

| 확인할 내용 | 먼저 찾을 자료 ID | 기록할 항목 |
| --- | --- | --- |
| 제품·Package·Clock 사양의 적용 조건 | D0 Addendum, D1 Datasheet | 제품명, Package, Version, 표 또는 절, 적용 조건 |
| Pin 연결과 전기적 동작 | D1, U1 TC38x Appendix, U3 Family Part 1, B1 Board | Pin·Port, 회로 위치, Register 설명, 연결 조건 |
| STM의 Clock·Counter·읽기 동작 | U1, U2 STM 설명, U3 Family Part 1, U4 Family Part 2 | 절·표·그림, Clock 경로와 조건, 사용한 Counter 범위 |
| 예제와 Driver가 선택한 실제 설정 | S1 Blinky 예제, S2 STM Header, S3 Port Header, S4 Scu Header | 고정 Commit, File·함수 또는 Symbol, 근거와 설정의 대응 |

자료를 찾은 뒤 아래 표를 `TIMING.md`에 채워 주세요. 긴 문장을 옮기는 대신, 찾은 사실이 자신의 변경과 검증에 어떤 영향을 주는지 설명합니다. 공식 자료가 정하는 사실과 제품 요구사항이 정한 정책을 구분해 적으세요.

| 학습 목표·확인 질문 | 자료 ID·Version·절/표/페이지 또는 File·Symbol | 적용 제품·조건 | 찾은 사실 | 설계·변경한 함수 | 검증 입력·예상 결과·실행 증거 |
| --- | --- | --- | --- | --- | --- |
| 목표 1 / GPIO 설정 |D1 V1.2, §2.2 Table 2-25 p160, Figure 2-2 p13. B1 V2.0, §3.15 p3-6, Figure 6-1 p6-1. U1 V2.0.0, Table 3 p2-11. U3 V2.0.0, §14.4.2 p14-8, §14.4.8 p14-21. S3 IfxPort.h |TC387QP-160F300S, PG-LFBGA-292, 참조 Board KIT_A2G_TC387_5V_TFT. LED D107=P13.0, ENABLE=P00.0 |B1에서 D107의 Active LOW 동작과 X103 Pin 22의 P00.0 연결을 확인했다. U1·U3 자료 안내에는 Port 주소, IOCR0의 Pin Mode, OMR의 Bit별 제어가 제시되어 있다. 주소·Pin·극성은 각각 Register 위치·물리적 연결·논리 동작을 의미한다 |board_io_init(): HIGH Latch 설정 후 Push-pull Output으로 변경, ENABLE Pull-up 설정. board_collection_enabled(): LOW일 때 활성. board_led_toggle(): LED_MASK만 Toggle |초기 LED OFF, ENABLE HIGH/LOW, GPIO 호출 순서, 다른 Bit 보존 확인. 고정 Host Test 포함. make test: 361 checks, 0 failures |
| 목표 1 / STM와 Tick 계산 |D0 v1.9, Table 1 p3. U3 V2.0.0, §10.5.1.3 p10-29, §10.5.1.2 Table 280 p10-28. U1 V2.0.0, §25.2 Table 273 p25-1. U4 V2.0.0, §27.4.2 p27-9. S2 IfxStm.h, IfxStm_getLower(). S4 IfxScuCcu.h |SAK-TC387QP-160F300S AE, PG-LFBGA-292. 과제의 fSTM=100MHz; 200MHz는 Host 가상 산술 입력 |D0의 제품 최대 CPU 주파수 300MHz를 STM Clock으로 그대로 사용할 수 없다. 자료 안내에 따르면 STM Clock은 fsource0 / STMDIV 경로를 사용한다. STM 전체 Counter는 64-bit이고 이번 과제는 TIM0의 하위 32-bit를 사용한다 |ticks_from_ms(): 전달된 Hz로 계산, uint64_t 곱셈, 정수 Tick·범위 검사, 실패 시 출력 보존. period_due(): unsigned 경과 차이와 >= 비교, 만료 시 last=now |100MHz·10ms → 1,000,000 Tick, 250ms → 25,000,000 Tick. 직접 추가한 5MHz·1000ms → 5,000,000 Tick 테스트 통과. Wraparound 및 524/525 경계 테스트 포함. 361 checks, 0 failures |
| 목표 2 / 주기 수집 동작 |U2 System Timer 공식 설명, HTML. U4 V2.0.0, §27.4.2 p27-9. S2 고정 Commit 6759d284, IfxStm_getLower() 및 IfxStm_waitTicks(). 수집 주기와 ENABLE 정책은 과제 요구사항 |Single-core Polling Host 모델. 센서 10ms 수집, LED 250ms Toggle, ENABLE LOW 활성 |S2에서 Timer 하위 값 조회와 unsigned 차이 비교 예시를 확인했다. 10ms·250ms 주기, 비활성 중 기준 갱신, 호출당 최대 1회 수행은 Hardware 사양이 아니라 이번 과제의 정책이다 |collector_init(): 전달된 stm_hz로 두 주기 계산, State 초기화, 실패 시 GPIO 초기화 방지. collector_step(): ENABLE 한 번 조회, 수집 기준 갱신, 수집과 LED 독립 판정 |0ms 초기화 후 35ms 호출 시 수집 1회, last=35ms; 40ms 미수집, 45ms 재수집. Demo 250ms에서 ENABLE 비활성·수집 3회 유지·LED ON. logs/demo.csv, Host Test |
| 목표 3 / 지연과 검증 범위 |S1 Blinky_LED.c, 공식 예제 고정 Commit 6759d284(대기 방식 검토 대상). S2 IfxStm.h, IfxStm_waitTicks(). U2 STM 공식 설명. 처리 시간과 Polling 간격은 과제에서 제시한 분석 조건 |macOS 14.5, Apple Clang 15.0.0, C11 Host. 실제 MCU 미실행. 과제의 가상 처리 시간: 센서 2ms, UART 15ms, 대기 10ms |단순 순차 대기 방식에서는 센서 읽기 시작 간격이 27ms이다. 대기만 제거해도 17ms로 10ms 목표를 만족하지 못한다. Polling 시작 간격이 최대 5ms라면 만료 발견 지연은 최대 5ms 수준으로 제한되지만 실제 센서 읽기 시각에는 처리 시간도 영향을 준다 |collector_step(): Delay 제거, 매 호출마다 입력 확인, LED와 센서 주기 독립 수행. 실제 처리 시간 측정이나 UART 성능 개선은 수행하지 않음|make test: 361 checks, 0 failures. make demo: 모든 행의 delay_calls=0. 27ms·17ms는 계산한 예상값이며 CSV는 주입한 가상 시각의 Host Simulation 결과. MCU 실측 아님 |

## 대상과 개발 범위

- MCU는 Infineon AURIX TC387QP입니다. 사양 비교 대상은 **TC387QP-160F300S, PG-LFBGA-292**로 한정하고, D0·D1의 제품별 조건과 일치하는지 확인합니다. STM32의 RCC·HAL·PA5 설정을 그대로 사용하지 않습니다.
- Hardware 참조 Board는 `KIT_A2G_TC387_5V_TFT`입니다. 필수 과제는 PC에서 실행하는 C11 Host test이며, 실제 Board와 AURIX Development Studio 설치는 필수가 아닙니다.
- STM은 System Timer입니다. 이번 과제는 Free-running Counter의 하위 32-bit Tick을 시간 기준으로 사용합니다. STM 자체가 32-bit Timer라는 뜻은 아닙니다.
- 기본 검증 입력은 **fSTM = 100 MHz**입니다. 이는 과제가 제공하는 Clock 조건이며 CPU Clock이나 Reset 기본값을 뜻하지 않습니다. 구현에는 전달된 `stm_hz`를 사용합니다. 200 MHz Case는 고정값 사용을 검출하는 범용 산술의 Host 가상 입력이며 TC387QP의 허용 STM 설정을 뜻하지 않습니다.
- 센서는 ADC 대신 `platform_sensor_read()`가 제공하는 0~4095의 모의 값입니다. 실제 ADC 설정·전압 변환·통신은 이번 범위에 없습니다.
- 하나의 Core에서 순차적으로 실행합니다. ISR·RTOS·다른 Core와의 공유 데이터는 다음 단계의 주제입니다.
- 한 번의 초기화 이후 실제 수집 횟수는 `2^32`회 미만입니다. 수집 횟수 Counter의 장기 누적 Overflow 정책은 추가 실습입니다.
- 입력은 이미 안정된 논리값이라고 가정합니다. 기계식 Button의 Debounce는 추가 실습으로 구분합니다.

제공된 `platform_*`는 과제용 Host 대역입니다. Infineon iLLD API가 아니며 Hardware 주소를 PC에서 역참조하지 않습니다. 실제 iLLD와의 연결은 [자료 안내](REFERENCES.md)에서 확인하세요.

## 제품 요구사항

| 항목 | 요구 동작 |
| --- | --- |
| 수집 주기 | 10 ms, 활성 입력이 LOW일 때만 수집 |
| 상태 LED | 250 ms마다 Toggle, 수집 비활성 상태에서도 동작 |
| LED 초기 상태 | OFF, Active LOW이므로 출력값 HIGH |
| 입력 초기 설정 | Input Pull-up, HIGH는 수집 중지, LOW는 수집 허용 |
| 첫 동작 | 초기화 직후 즉시 수집하지 않고 각 주기가 지난 뒤 수행 |
| 늦은 호출 | 한 번의 호출에서 각 작업을 최대 1회만 수행 |
| 다음 기준 | 작업 판정이 만료되면 해당 기준 시각을 현재 `now`로 갱신 |
| 재초기화 | 이전 성공 여부와 수집 기록을 초기화하고 새 조건으로 시작 |
| 실행 방식 | Polling, `collector_step()` 안에 Delay·Sleep·대기 Loop·UART 출력 없음 |

LED의 250 ms는 Toggle 간격입니다. 이상적인 ON/OFF 한 주기는 500 ms입니다. 과제에서 말하는 10 ms는 만료를 판단하는 기준이며 실제 Board에서 항상 정확히 10 ms 간격으로 수집한다는 보장이 아닙니다.

## 개발 요청 1 Timer 계산과 만료 판정

`src/timebase.c`의 두 함수를 수정해 주세요.

### ticks_from_ms

```c
bool ticks_from_ms(uint32_t hz, uint32_t ms, uint32_t *out);
```

`Tick = fSTM × ms / 1000`으로 계산합니다. 성공 시 `true`를 반환하고 `*out`에 결과를 저장합니다. 실패 시 `false`를 반환하며, `out`이 유효한 경우 기존 값을 보존합니다.

- `out == NULL`, `hz == 0`, `ms == 0`은 실패입니다.
- 계산 결과가 정확한 정수 Tick이 아니거나 1 Tick 미만이면 실패입니다. 버림이나 반올림으로 숨기지 않습니다.
- 허용 결과는 1~`INT32_MAX`입니다. 그보다 크면 실패입니다.
- 곱셈의 중간 결과가 32-bit 범위를 넘는 정상 요청도 정확히 처리해야 합니다.

100 MHz에서 10 ms와 250 ms에 필요한 Tick을 직접 계산해 `TIMING.md`에 적으세요. TC387QP-160F300S의 CPU 사양에서 확인한 300 MHz라는 숫자를 STM 주파수에 넣어도 되는지 공식 Clock 자료를 근거로 설명합니다. 이 숫자를 모든 TC387QP의 동작 조건이나 실제 `fSTM`으로 일반화하지 마세요.

### period_due

```c
bool period_due(uint32_t now, uint32_t period, uint32_t *last);
```

- 경과 Tick이 `period` 이상이면 `true`를 반환하고 `*last = now`로 갱신합니다.
- 아직 주기가 지나지 않았으면 `false`이며 `*last`는 보존합니다.
- `last == NULL`, `period == 0`, `period > INT32_MAX`는 실패이며 저장값을 바꾸지 않습니다.
- Counter가 `UINT32_MAX`에서 0으로 넘어가는 Wraparound도 처리해야 합니다.
- 마지막 기준 시각 이후 실제 경과 시간이 `2^32` Tick 미만인 조건에서 사용합니다. Counter가 한 바퀴 이상 돈 시간을 하위 32-bit 두 값만으로 복구할 수 있다고 가정하지 않습니다.
- `now`는 같은 STM Counter를 순서대로 관측한 값입니다. 실행 중 Clock 변경·Counter Reset·Debug 정지 후 재개 시에는 시간 기준을 다시 초기화하는 것으로 범위를 정합니다.

지연된 횟수만큼 연속 실행하는 Catch-up은 이번 요구사항이 아닙니다. 예를 들어 초기화 0 ms, 다음 호출 35 ms라면 수집은 최대 한 번이고 새 기준은 35 ms입니다. 그 뒤 40 ms에는 수집하지 않고 45 ms에 다음 수집이 가능합니다. 이 정책은 구현을 단순하게 하지만 누락된 시점의 값을 복원하지 않으며 기준 시각도 밀릴 수 있습니다.

## 개발 요청 2 GPIO 설정과 제어

`src/board_io.c`의 `board_io_init()`, `board_collection_enabled()`, `board_led_toggle()`을 수정해 주세요.

1. LED 출력 Latch를 HIGH로 준비한 뒤 Output Push-pull로 전환합니다.
2. 외부 ENABLE 입력을 Input Pull-up으로 설정합니다.
3. 입력이 LOW일 때만 `board_collection_enabled()`가 `true`를 반환합니다.
4. LED Toggle은 지정된 LED Bit만 바꾸고 같은 Port의 다른 Bit를 보존합니다.

Host에서 LED와 ENABLE은 `platform.h`의 논리 Pin으로 표현합니다. Hardware 참조 Board의 LED는 **P13.0에 연결된 D107, Active LOW**입니다. ENABLE은 외부 입력 **P00.0**으로 정의합니다. TC3X7 V2.0 문서의 X103 Pin 22에 해당하며 내부 Pull-up과 외부 GND 접점으로 LOW를 만듭니다. Board의 RESET 또는 WAKE Button을 일반 GPIO Button으로 사용하지 않습니다. 실제 연결은 Board Revision·회로도·추가 장착 장치를 확인하는 추가 실습입니다.

`TIMING.md`에는 GPIO Register 주소, Pin 번호, LED 극성이 각각 어떤 역할을 하는지 설명하세요. 주소를 안다는 것만으로 Pin Mode와 출력값이 자동으로 설정되지는 않습니다.

## 개발 요청 3 기다리지 않는 수집 흐름

`src/collector.c`의 `collector_init()`과 `collector_step()`을 수정해 주세요. 함수의 정확한 인자와 상태 구조체는 `include/mission.h`를 기준으로 합니다.

### 초기화 계약

- `NULL` 상태 Pointer는 실패입니다.
- 유효한 상태 객체는 초기화 시도마다 모든 Field를 초기값으로 되돌립니다. 주기 계산에 실패해도 이전 `ready`와 수집 기록을 남기지 않습니다.
- 10 ms 수집 주기와 250 ms LED 주기를 모두 계산합니다.
- 계산에 실패하면 `false`이며 GPIO 설정을 진행하지 않습니다.
- 성공 시 GPIO를 초기화하고, 두 기준 시각을 전달받은 `now`로 설정한 뒤 `ready = true`로 만듭니다.

### 반복 동작 계약

- `NULL` 또는 준비되지 않은 상태에서는 아무 I/O도 수행하지 않습니다.
- 정상 상태에서는 호출마다 ENABLE 입력을 한 번 확인합니다.
- 수집 주기가 만료되면 기준 시각을 갱신합니다. 활성 상태이면 센서를 한 번 읽고 `last_value`와 `sample_count`를 갱신합니다.
- 비활성 상태에서도 만료 판정과 기준 시각 갱신은 계속합니다. 다시 활성화되었을 때 비활성 기간의 Sample을 몰아서 만들지 않습니다.
- LED 만료 판정은 수집과 독립적입니다. 비활성 입력 때문에 LED까지 멈추면 안 됩니다.
- Sample 횟수는 실제 읽기 횟수입니다. 읽지 않은 과거 시점을 수집했다고 계산하지 않습니다.

상태 변경은 이 과제의 Single-core 순차 호출 안에서만 이뤄집니다. 실제 `platform_sensor_read()`에 긴 대기가 들어가면 전체 Loop가 다시 늦어지므로, 단순히 함수 이름을 나누는 것으로 Non-blocking이 보장되지는 않습니다.

## 개발 요청 4 Timing 근거와 처리 시간

`TIMING.md`의 표를 채우고 예상과 실행 결과를 구분해 주세요.

1. 초기화 0 ms 이후 0, 5, 10, 15, 20 ms에 호출할 때, 각 시각의 수집 여부를 적습니다.
2. 초기화 0 ms 이후 35, 40, 45 ms에 호출할 때 수집 횟수와 다음 기준 시각을 적습니다.
3. 입력 HIGH 상태로 10 ms에 호출한 뒤 LOW로 바꾸어 15, 20 ms에 호출할 때 수집 여부를 적습니다.
4. 초기화 0 ms 이후 250 ms에 처음 호출하면 수집과 LED가 각각 몇 번 동작하는지 적습니다.
5. Wraparound 직전·직후의 두 Counter 값으로 10 ms 경과를 검증하는 Case를 만듭니다.

다음은 Starter의 Delay 250 ms와 별개인 Timing 검토 시나리오입니다. 별도 시제품이 `센서 읽기 2 ms → UART 출력 15 ms → 대기 10 ms → 다음 센서 읽기`를 반복한다면, 센서 읽기 시작 간격은 얼마인지 계산하세요. 다른 오버헤드는 0으로 둡니다. UART를 그대로 둔 채 대기만 제거했을 때 10 ms 요구를 충족할 수 있는지도 설명합니다.

실제 제품에서 `collector_step()` 시작 간격을 최대 5 ms로 관리한다는 추가 조건을 검토합니다. 이 경우 만료를 발견하는 지연은 어떻게 제한되는지, 센서 값을 실제로 읽은 시각까지 같은 보장을 하려면 어떤 처리 시간도 알아야 하는지 적으세요. 목표 주기, STM 해상도, Polling 간격, 처리 시간, 실측 간격을 서로 구분합니다.

## 변경 범위와 검증

필수 변경은 `src/timebase.c`, `src/board_io.c`, `src/collector.c`입니다. `include/`, `host/`, 제공 `tests/test_mission.c`, 기본 `Makefile`의 검증 계약은 유지합니다. 자기 Test는 `tests/test_student.c`에 최소 2개 추가하세요. 경계와 실패 사례가 실제 `assert` 또는 검증식으로 검사되어야 합니다. 공개 해설의 Test 결과는 참고 자료이며, 자신의 제출 코드로 실행한 결과를 대신하지 않습니다.

```sh
make test
make demo
```

초안은 Compile되지만 의도된 결함으로 검증이 실패합니다. 정상값 하나만 통과시키지 말고 100/200 MHz, 경계 Tick, Wraparound, 늦은 호출, 비활성 상태, 재초기화 실패, 다른 GPIO Bit 보존을 모두 확인하세요.

`make demo`의 시간은 Test가 주입하는 가상 시각입니다. PC에서 출력된 CSV를 MCU 실측이라고 쓰지 않습니다. 실제 Board 실행을 했다면 Compiler·iLLD·Board 조건과 측정 방법을 별도로 기록하세요.

## Pull Request 제출

제출 대상은 `zinee-u/embedded-just-for-fun`의 `main`입니다. 제공 Starter를 자기 `submissions/week05/YOUR_GITHUB_ID/`로 복사해 수정합니다. 원본 `weeks/week05`와 다른 학생의 제출은 변경하지 않습니다.

```text
submissions/week05/YOUR_GITHUB_ID/
  src/                 # 개선한 세 C Module
  include/             # 제공 계약
  host/                # 제공 Host 대역
  tests/               # 제공 Test + 자신의 추가 Test
  Makefile
  README.md            # 모든 원인, 수정 원리, 검증 결과
  BUILD_ENV.md         # Compiler, OS, 명령, 검증 범위
  TIMING.md            # 계산, 호출표, 지연, 공식 자료 근거
  logs/test.log        # 실제 검증 결과
  logs/demo.csv        # 실제 Host Demo 결과
```

구현과 검증·환경 기록의 목적을 구분해 최소 2개 Commit을 남기세요. PR 제목은 `[05회차] YOUR_GITHUB_ID TC387QP 주기 수집 개선`입니다. 모든 결함에 대해 **실패 상황 → 근본 원인 → 수정 원리 → 검증 증거**를 작성합니다.

- [Fork부터 제출까지](SUBMISSION.md)
- [PR 본문 양식](PR_BODY.md)
- [공식 자료와 Hardware 연결](REFERENCES.md)
- [한국어 해설 코드와 검증 방법](reference/README.md)

작성일은 2026-10-11입니다. `week05`는 수업 회차이며 배포 날짜나 마감일을 뜻하지 않습니다.

## 변경 기록

- 2026-10-11: 05회차에 한해 한국어 해설 코드를 공개했습니다. 해설을 활용한 독학 후 원본 Starter를 자기 제출 폴더에서 개선하고, 직접 선택한 Test와 근거로 기존 PR 방식에 따라 제출하도록 안내했습니다. 기존 API, 제품 정책과 검증 계약은 유지합니다.
- 2026-10-11: 04회차와 문서 Theme를 통일하고, 학습 목표에서 공식 자료의 근거·설계 판단·Test로 이어지는 기록 방법을 명확히 했습니다. 기존 API, 제품 정책과 검증 계약은 유지합니다.
