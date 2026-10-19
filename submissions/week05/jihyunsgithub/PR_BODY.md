# [05회차] YOUR_GITHUB_ID TC387QP 주기 수집 개선

05회차에 한해 공개된 [한국어 해설 코드](https://github.com/zinee-u/embedded-just-for-fun/tree/main/weeks/week05/reference)를 참고할 수 있습니다. 아래 내용은 자신의 제출 코드와 실제 실행 결과를 기준으로 작성합니다. 경로는 과제 폴더 `weeks/week05/`를 기준으로 합니다.

## 요청을 해석한 내용

- GPIO의 극성·Mode·Bit 보존:
  참조 Board의 D107 LED는 P13.0에 연결되어 있고 Active LOW로 동작한다. 따라서 초기 OFF 상태는 HIGH 출력이며, 출력 Latch를 HIGH로 준비한 뒤 Push-pull Output으로 설정한다.

    ENABLE은 P00.0 외부 입력으로 가정하고 Input Pull-up으로 설정한다. HIGH는 수집 중지, LOW는 수집 허용이다.

    LED를 Toggle할 때는 LED_MASK에 해당하는 Bit만 변경하고 다른 Bit는 보존한다.

- 사용한 `stm_hz` 조건과 10 ms·250 ms의 Tick:
  기본 Host 검증 입력은 stm_hz = 100,000,000Hz이다.

    10ms: 100,000,000 × 10 / 1000 = 1,000,000 Tick

    250ms: 100,000,000 × 250 / 1000 = 25,000,000 Tick

    200MHz는 주파수 고정값 사용을 검사하는 Host 가상 입력이지 실제 보드의 STM Clock 설정값이 아니다.

    collector_init()은 고정 주파수를 사용하지 않고 전달받은 stm_hz로 두 주기를 계산한다.

- 지연·비활성·Wraparound·재초기화 정책:
    Polling 호출이 늦어져도 한 번의 호출에서 센서 수집과 LED Toggle을 각각 최대 한 번만 수행한다. 만료 시 기준 시각은 현재 now로 갱신한다.

    ENABLE이 비활성화된 동안 센서는 읽지 않지만 만료된 수집 기준 시각을 갱신하며, LED는 독립적으로 동작한다.

    32-bit Counter Wraparound는 unsigned 뺄셈으로 계산한다. 단, 기준 시각부터 실제 경과가 2^32 Tick 미만이어야 한다.

    재초기화 시 이전 수집 기록과 ready를 포함한 모든 State 필드를 초기화한다. 주기 계산이 실패하면 GPIO를 초기화하지 않는다.

    collector_step()에 Blocking Delay, Sleep, Busy-wait, UART 출력을 넣지 않는다.

## 학습 목표에서 설계와 Test까지

사양 비교 대상은 TC387QP-160F300S, PG-LFBGA-292입니다. 제품·Board Revision·Clock 조건이 달라지면 적용 가능성을 따로 설명합니다. 자료의 상세 위치와 Version은 `TIMING.md`에 기록하고 아래 표에서 변경 및 검증과 연결해 주세요.

| 학습 목표·확인 질문 | 자료 ID·Version·절/표/페이지 또는 File·Symbol | 적용 조건·찾은 사실 | 설계·변경한 함수 | 검증 입력·예상 결과·Log 위치 |
| --- | --- | --- | --- | --- |
| 목표 1 / GPIO·STM 설정 |D0 v1.9 Table 1 p3; D1 V1.2 Table 2-25 p160·Figure 2-2 p13; B1 V2.0 §3.15 p3-6·Figure 6-1 p6-1; U3 V2.0.0 §14.4.2 p14-8·§10.5.1.3 p10-29; U4 V2.0.0 §27.4.2 p27-9; S2 IfxStm_getLower(); S3 IfxPort.h; S4 IfxScuCcu.h TC387QP-160F300S, PG-LFBGA-292, KIT_A2G_TC387_5V_TFT 참조. D107=P13.0 Active LOW, ENABLE=P00.0. STM은 64-bit Counter이며 TIM0는 하위 32-bit를 제공한다. CPU 최대 Clock과 STM Clock은 구분해야 한다. |board_io_init(), board_collection_enabled(), board_led_toggle(), ticks_from_ms() |GPIO 초기화 순서·LOW 활성·다른 Bit 보존 및 100MHz에서 10ms=1,000,000 Tick 확인. logs/test.log |
| 목표 2 / 주기 수집 |U2 System Timer 공식 설명; U4 V2.0.0 §27.4.2 p27-9; S2 IfxStm.h의 하위 Counter 조회 및 Tick 대기 관련 API; 10ms·250ms와 비활성 정책은 과제 요구사항 |Single-core Polling Host 모델. 센서는 ENABLE LOW이고 주기가 만료되었을 때만 읽는다. LED는 ENABLE과 독립적으로 250ms마다 Toggle한다. |period_due(), collector_init(), collector_step() |35ms 첫 호출에서 수집 1회, 40ms 미수집, 45ms 수집. Demo 250ms에서 ENABLE 비활성·samples=3 유지·LED ON. logs/test.log, logs/demo.csv |
| 목표 3 / 지연·검증 범위 |S1 Blinky_LED.c, 공식 예제 Commit 6759d2845feaac0107b785fe6bc57ed785c3f4fc; S2 IfxStm.h; U2 STM 공식 설명; 처리 시간과 Polling 조건은 과제에서 주어진 분석 시나리오 센서 2ms+UART 15ms+대기 10ms이면 읽기 시작 간격은 27ms이고, 대기만 제거해도 17ms이다. Host Simulation은 실제 MCU 실측이 아니다. |collector_step()에서 Delay 제거, ENABLE Polling과 센서·LED 작업 독립 처리 |make test: 361 checks, 0 failures. Demo 모든 행에서 delay_calls=0. logs/test.log, logs/demo.csv |

자료 ID는 `REFERENCES.md`를 따릅니다: D0 Addendum, D1 Datasheet, U1 TC38x Appendix, U2 STM 설명, U3 Family Part 1, U4 Family Part 2, B1 Board, S1 Blinky 예제, S2 STM Header, S3 Port Header, S4 Scu Header.

- 공식 자료가 정한 사실과 과제의 제품 정책을 구분한 설명:
    공식 자료는 STM Clock 경로, TIM0의 Counter 폭, Port Register 기능, MCU Pin 기능과 Board 연결 등을 설명한다.

    10ms 수집, 250ms LED Toggle, ENABLE LOW 수집 허용, 늦은 호출에서 Catch-up을 하지 않는 동작은 과제에서 정한 정책이다.

    INT32_MAX 범위 제한과 실패 시 출력 보존도 이번 함수 계약에 따른다.

    100MHz는 과제의 검증 조건이며, 실제 보드에서 측정한 STM 주파수가 아니다.

- 확인하지 못한 근거 또는 실제 Board에서 추가 검증할 조건:
    실제 장치의 Clock 초기화 설정과 fSTM

    보드 Revision, 실제 배선, GPIO 전압과 출력 파형

    실제 센서 읽기 및 UART 처리 시간

    Startup, Watchdog, Interrupt, Multi-core 환경에서의 동작

    Host Adapter와 실제 iLLD API 연결 및 보드 실행

## 발견한 모든 근본 원인

| File과 함수 | 실패 입력 또는 호출 순서 | 초안의 실제 결과 | 근본 원인 | 변경 내용 | 확인한 Test |
| `timebase.c` / `ticks_from_ms()` | `hz=5000000`, `ms=1000` | 32-bit 중간 곱셈 Overflow로 잘못된 Tick 계산 | 중간 곱셈을 32-bit로 수행 | 곱셈 전에 `uint64_t`로 변환 | 직접 추가 Test 1 통과 |
| `timebase.c` / `ticks_from_ms()` | 정수가 아닌 Tick 발생 | 소수 부분을 버리고 성공 처리 | 나머지 검사 누락 | `product % 1000 == 0` 확인 | 고정 Test 통과 |
| `timebase.c` / `ticks_from_ms()` | 결과가 `INT32_MAX` 초과 | 허용 범위 밖의 Tick 처리 | 최종 결과의 상한 검사 누락 | 1~`INT32_MAX` 범위만 허용 | 고정 Test 통과 |
| `timebase.c` / `ticks_from_ms()` | 잘못된 입력과 기존 출력값 | 실패 후에도 출력값이 바뀔 수 있음 | 검증 완료 전 출력값 변경 | 모든 검증 성공 후 `*out` 저장 | 고정 Test 통과 |
| `timebase.c` / `period_due()` | 정확히 주기만큼 경과 | 만료 경계에서 실행하지 않음 | `>` 조건 사용 | 경과 Tick을 `>= period`로 비교 | 직접 추가 Test 2 통과 |
| `timebase.c` / `period_due()` | `UINT32_MAX` 전후의 시각 | Counter Wraparound에서 만료 판정 오류 | `last + period` 방식에 의존 | unsigned `(now - *last)`로 경과 Tick 계산 | 고정 Test 통과 |
| `timebase.c` / `period_due()` | 35ms에 늦게 호출 | 과거 기준 시각이 남아 조기 재실행 가능 | 만료 후 기준 시각 갱신 정책 오류 | 만료 시 `*last = now` | 고정 Test 및 Demo 통과 |
| `timebase.c` / `period_due()` | `period=0` 또는 `INT32_MAX` 초과 | 무효한 Period 처리 | Period 유효성 검사 부족 | 무효한 Period는 `false` 반환 | 고정 Test 통과 |
| `board_io.c` / `board_io_init()` | LED 초기화 | 출력 활성화 전 HIGH Latch 준비 누락 | GPIO 초기화 순서 오류 | HIGH Latch 설정 후 Push-pull Output 전환 | 고정 Test 통과 |
| `board_io.c` / `board_io_init()` | Active LOW LED 초기 상태 | 초기 OFF 상태가 보장되지 않음 | Active LOW 극성 미반영 | 초기 출력값 HIGH 설정 | 고정 Test 통과 |
| `board_io.c` / `board_io_init()` | ENABLE 입력 초기화 | 요구된 Pull-up 상태 미설정 | 입력 Mode 설정 오류 | Input Pull-up 설정 | 고정 Test 통과 |
| `board_io.c` / `board_collection_enabled()` | ENABLE HIGH/LOW 입력 | 수집 허용 논리가 반대로 적용 | Active LOW 해석 오류 | LOW일 때 `true` 반환 | 고정 Test 통과 |
| `board_io.c` / `board_led_toggle()` | 다른 Bit가 설정된 Port | LED 외 Bit까지 변경될 수 있음 | LED Mask 제한 누락 | `LED_MASK`만 Toggle | 고정 Test 통과 |
| `collector.c` / `collector_init()` | 기본값과 다른 `stm_hz` 입력 | 잘못된 수집·LED Tick 계산 | 주파수 고정값 사용 | 전달된 `stm_hz` 사용 | 고정 Test 통과 |
| `collector.c` / `collector_init()` | 기존 수집 기록이 있는 상태에서 재초기화 | 이전 State가 남을 수 있음 | 전체 State 초기화 누락 | 모든 State 필드 초기화 | 고정 Test 통과 |
| `collector.c` / `collector_init()` | 주기 계산 실패 | 초기화 실패에도 GPIO 상태 변경 가능 | 주기 검증 완료 전 GPIO 호출 | 두 주기 계산 성공 후 GPIO 초기화 | 고정 Test 통과 |
| `collector.c` / `collector_step()` | `NULL` 또는 `ready=false` | 준비되지 않은 상태에서 I/O 가능 | 준비 상태 검사 부족 | 초기 Guard 조건 추가 | 고정 Test 통과 |
| `collector.c` / `collector_step()` | ENABLE 비활성 중 수집 주기 만료 | 수집 기준 시각 갱신 누락 | 기준 갱신을 ENABLE 상태에 종속 | 비활성 중에도 만료된 기준 시각 갱신 | 고정 Test 통과 |
| `collector.c` / `collector_step()` | ENABLE 비활성 중 LED 주기 만료 | LED 동작까지 중단될 수 있음 | LED 판정을 센서 수집 조건에 종속 | LED 주기 독립 판정 | 고정 Test 및 Demo 통과 |
| `collector.c` / `collector_step()` | 센서를 여러 번 읽는 상황 | 수집 횟수 누적 오류 | Count 갱신 로직 오류 | 센서 읽기마다 `sample_count` 증가 | 고정 Test 및 Demo 통과 |
| `collector.c` / `collector_step()` | 매 Step의 Delay 호출 | 다른 입력 확인과 LED 처리 지연 | Blocking Delay 사용 | Delay 호출 제거 | 고정 Test 및 Demo 통과 |


한 행의 수정으로 다른 원인을 가리지 않았는지 전체 초안을 다시 확인했습니다.

## 공개 해설을 활용해 확인한 내용

- 참고한 해설 File·함수와 이해한 동작:
    timebase.c: 중간 곱셈 범위와 주기 경계 판정이 중요함을 확인했다.

    board_io.c: Active LOW 극성과 Latch 설정 순서, 다른 Bit 보존을 확인했다.

    collector.c: 주기 판단과 ENABLE 상태를 분리하고 LED를 독립적으로 실행해야 함을 확인했다.

- 직접 고른 실패 입력 또는 호출 순서, 예상 결과와 초안의 실제 결과:
    ticks_from_ms(5000000U, 1000U, &out): 예상 결과는 5,000,000 Tick이다. 32-bit 중간 곱셈으로는 Overflow가 발생하는 입력이다.

    last=500, period=25, now=524 → 525: 예상 결과는 524에서 미만료, 525에서 만료이다.

- 수정이 필요한 이유와 연결한 공식 자료 또는 제품 요구사항:
    STM의 Counter 폭과 Clock 경로는 U3·U4, 실제 Board의 LED 극성과 Pin 연결은 B1을 근거로 확인했다.

    10ms 수집, 250ms LED Toggle, 초기 OFF, 비활성 중 독립 동작은 과제의 제품 요구사항을 근거로 적용했다.

- 해설을 읽은 뒤 추가로 확인하고 싶었던 조건:
  중간 곱셈이 32-bit 범위를 넘어도 최종 Tick이 유효한 경우를 별도로 확인하고자 했다.

    정확한 만료 시각 직전과 해당 시각에서 반환값 및 기준 시각 변경을 확인하고자 했다.

해설과 코드의 모양을 비교하는 데 그치지 않고, 각 변경이 어떤 실패를 막는지 근거와 실행 결과로 설명합니다.

## 직접 추가한 Test

해설의 Test를 그대로 복사하는 대신 자신이 확인하려는 조건을 선택해 최소 2개 작성합니다. 제공 Test의 계약은 유지합니다.

| Test 이름·위치 | 선택한 입력·호출 순서 | 이 조건을 고른 이유 | 예상 결과 | 실제 결과·Log 위치 |
| --- | --- | --- | --- | --- |
| Test 1 / `tests/test_student.c` | `ticks_from_ms(5000000U, 1000U, &out)` | 중간 곱셈 결과가 5,000,000,000으로 32-bit 범위를 초과하지만 최종 Tick은 유효하므로, 64-bit 중간 계산이 올바르게 수행되는지 검증하기 위해 선택했다. | `true` 반환, `out=5000000` | CHECK 2개 통과, `logs/test.log` |
| Test 2 / `tests/test_student.c` | `last=500`, `period=25`, `now=524` 호출 후 `now=525` 호출 | 만료 직전에는 기준 시각을 보존하고 정확한 만료 시점에는 현재 시각으로 갱신하는지 확인하기 위해 선택했다. | `now=524`: `false`, `last=500`; `now=525`: `true`, `last=525` | CHECK 4개 통과, `logs/test.log` |

## Timing과 검증

- `make test` 결과와 Exit code:
    361 checks, 0 failures
    Exit code 0
    증빙: logs/test.log

- `make demo` 결과: 가상 시각을 주입한 Host 결과입니다.
    10ms: 센서 수집 1회
    5ms: 누적 수집 3회
    250ms: ENABLE 비활성, 누적 수집 3회 유지, LED ON
    270ms: ENABLE 활성, 누적 수집 4회
    500ms: 누적 수집 5회, LED OFF
    모든 출력에서 delay_calls=0
    증빙: logs/demo.csv

- 추가 Test 최소 2개의 목적:
    Test 1: 32-bit 중간 곱셈 Overflow 상황에서 정확한 Tick 계산 확인
    Test 2: 정확한 주기 만료 경계와 last 보존·갱신 확인

- Timing 시나리오의 센서 읽기 시작 간격과 처리 시간의 합:
    센서 읽기 2ms + UART 출력 15ms + 대기 10ms = 27ms
    대기만 제거하면 2ms + 15ms = 17ms이므로 10ms 목표를 만족하지 못한다.

- 예상 주기와 측정값을 구분한 설명:
    10ms는 센서 수집의 목표 만료 기준이고 250ms는 LED Toggle 간격이다.
    최대 5ms Polling 시작 간격이 유지된다면 만료 발견 지연은 최대 약 5ms로 제한할 수 있다.
    실제 센서 읽기 시각을 보장하려면 선행 처리 시간과 센서 읽기 소요 시간도 고려해야 한다.
    Demo의 time_ms는 Host에서 주입한 가상 시각이다. 실제 보드에서 수집 간격을 실측한 값이 아니다.

- 실행하지 않은 범위: 실제 TC387QP Build, Flash, 실측 등 해당 사항을 정확히 기재
    실제 TC387QP용 Firmware Build 및 Flash 미실행
    실제 Board의 STM Clock 측정, GPIO 전압 및 LED 파형 측정 미실행
    실제 센서 통신, Interrupt, Multi-core, Watchdog 환경 검증 미실행

## Commit과 Review

- 구현 Commit SHA와 목적:
    `5af4ce2`: C Module 개선 및 직접 추가한 테스트 구현

- 검증·문서 Commit SHA와 목적:
    `e9e46c4`: README, BUILD_ENV, TIMING 및 검증 Log 기록
    `ecddd15` — `additional .md`

- 변경 경로가 `submissions/week05/YOUR_GITHUB_ID/` 안인지 확인:
    제출 경로: submissions/week05/jihyunsgithub/

- 같은 Branch에 수정 Commit을 추가해 Review에 대응합니다.
