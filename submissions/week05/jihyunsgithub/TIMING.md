# Timing 분석 기록

## 학습 목표와 공식 근거

`REFERENCES.md`의 자료 ID를 사용해 **학습 목표 → 자료의 위치·조건 → 찾은 사실 → 설계 → Test**를 연결합니다. 제품 사양 비교 대상은 TC387QP-160F300S, PG-LFBGA-292입니다. 실제 사용한 제품·Board Revision·Clock 설정이 다르면 차이를 기록하세요.

자료 ID: D0 Addendum, D1 Datasheet, U1 TC38x Appendix, U2 STM 설명, U3 Family Part 1, U4 Family Part 2, B1 Board, S1 Blinky 예제, S2 STM Header, S3 Port Header, S4 Scu Header. 상세 제목·Version·링크는 배포된 자료 안내를 기준으로 합니다.

| 학습 목표·확인 질문 | 자료 ID·Version·절/표/페이지 또는 File·Symbol | 적용 제품·조건 | 찾은 사실 | 설계·변경한 함수 | 검증 입력·예상 결과·실행 증거 |
| --- | --- | --- | --- | --- | --- |
| 목표 1 / GPIO Mode·Pin·극성 |D1 Datasheet의 Package Pin 정보, B1 §3.15 Board 배선, S3 IfxPort.h (정확한 Version·표 번호는 REFERENCES.md 대조) |  Package Pin 정보, B1 §3.15 Board 배선, S3 IfxPort.h (정확한 Version·표 번호는 REFERENCES.md 대조)

TC387QP-160F300S, PG-LFBGA-292 및 참조 Board. 실제 검증은 C11 Host 모델| 참조 Board의 P13.0/D107 LED는 Active LOW 방식이므로 LOW일 때 켜지고 HIGH일 때 꺼진다.|board_io_init()에서 HIGH Latch를 먼저 설정하고 Output 모드로 전환했다. ENABLE Pull-up을 활성화하고, board_collection_enabled()에서 LOW를 true로 해석했다. board_led_toggle()은 LED_MASK만 변경한다. | make test: 361 checks, 0 failures. Host GPIO 테스트에서 초기화 순서, Pull-up, 입력 극성, 다른 Bit 보존 검사 통과|
| 목표 1 / STM Clock·Counter |U3 Family Part 1 §10.5.1.3, p10-29; U4 Family Part 2 §27.4.2, p27-9 (Version은 REFERENCES.md 대조) | TC387QP 관련 STM 설명. 100MHz와 200MHz는 과제 Host 입력| STM Clock은 Clock Source와 분주 설정에 따라 결정된다. STM은 64-bit Counter이며 TIM0는 하위 32-bit 값을 읽는다.|ticks_from_ms()에서 전달받은 주파수와 64-bit 중간 곱셈을 사용했다. period_due()는 unsigned 경과 차이로 Wraparound를 처리했다. |100MHz, 10ms → 1,000,000 Tick; 200MHz, 250ms → 50,000,000 Tick. make test 통과 |
| 목표 2 / 주기 수집·만료 판정 |U4 §27.4.2 (Counter 동작), 5주차 README의 Software 요구사항 |C11 Host, 32-bit Tick, 수집 10ms 및 LED Toggle 250ms |Counter 폭은 하드웨어 특성이고, 정확한 만료 경계 포함 및 지연 후 한 번 처리 정책은 과제 요구사항이다. |period_due()에서 now - last >= period를 사용하고 만료 시 last = now로 갱신했다. collector_step()에서는 센서 수집과 LED 만료를 독립적으로 검사한다. |ast=500, period=25: now=524 → false, now=525 → true. 추가 Test를 포함해 361 checks, 0 failures |
| 목표 3 / 대기·Polling·처리 시간 |S1 Blinky 예제, S2 STM Header, 5주차 README의 Non-blocking 요구사항 (예제의 정확한 Symbol은 REFERENCES.md에서 추가 확인)|C11 Host Simulation, 호출 시각은 host/demo.c에서 주입|Polling은 함수가 호출된 시각에 만료 여부를 확인한다. 주기가 지났더라도 정확한 실행 시각이나 지연 시간을 보장하지 않는다. |collector_step()에서 platform_delay_ms(250U) 호출을 제거하고, 각 Step에서 필요한 작업만 수행한 뒤 반환하도록 수정했다. |Host Demo 모든 출력에서 delay_calls=0. 250ms에서 ENABLE이 비활성인데도 LED Toggle 확인 |

- 공식 자료가 정한 사실과 과제에서 정한 정책을 구분한 설명:
공식 하드웨어 자료에서는 STM Clock의 설정 경로, Counter 구조, TIM0의 하위 32-bit 읽기 동작 및 Board의 LED 배선을 확인한다.

정수 Tick만 허용하는 조건, INT32_MAX 상한, 실패 시 출력값 보존, 정확한 만료 경계 포함, 지연 후 한 번만 처리하는 정책은 이번 과제의 Software 요구사항이다.

uint64_t 형변환과 unsigned 뺄셈은 C 정수 산술 규칙을 이용해 해당 요구사항을 구현한 것이다.

- 아직 확인하지 못한 조건과 추가로 필요한 근거:
실제 TC387QP Board의 Clock 설정, STM 주파수, Pin 전압 및 파형은 측정하지 않았다.

실제 Firmware의 Startup, Interrupt, Multi-core 동작과 실행 시간은 이번 Host Simulation에서 검증하지 않았다.

공식 자료의 세부 Version, 표 번호 및 S1 예제 Symbol은 REFERENCES.md와 원문 대조가 필요하다.

## Tick 계산

100 MHz는 과제에서 제공하는 `fSTM` 조건입니다. **200 MHz는 고정값 사용을 검출하는 범용 산술의 Host 가상 입력**이며, TC387QP의 허용 STM 설정이나 실제 Board Clock을 뜻하지 않습니다. 두 입력을 실제 제품 사양으로 해석하지 마세요.

| STM Frequency | 10 ms Tick | 250 ms Tick | 계산 과정 |
| --- | --- | --- | --- |
| 100 MHz |1,000,000 |25,000,000 |100,000,000 × 10 / 1000 = 1,000,000; 100,000,000 × 250 / 1000 = 25,000,000 |
| 200 MHz |2,000,000 |50,000,000 |200,000,000 × 10 / 1000 = 2,000,000; 200,000,000 × 250 / 1000 = 50,000,000 |

- CPU Clock과 STM Clock을 같은 값으로 가정해도 되는가?
  
    근거:아니다. STM Clock은 Clock Source와 분주 설정에 따라 결정되므로 CPU Clock과 항상 같다고 가정할 수 없다. U3 §10.5.1.3의 STM Clock 경로를 근거로, collector_init()에서는 고정된 100MHz 대신 전달받은 stm_hz를 사용했다.

- `hz × ms`를 32-bit로 먼저 계산했을 때의 실패 입력·실제 값:
    추가 Test에서 hz=5,000,000, ms=1,000을 사용했다.

    정상적인 중간 곱셈값은 5,000,000,000이며, 이를 1000으로 나눈 정답은 5,000,000 Tick이다.

    32-bit unsigned 곱셈을 먼저 수행하면 결과가 705,032,704로 Wraparound되어 정수 나눗셈 후 705,032가 나온다.

    이를 방지하기 위해 곱셈 전에 uint64_t로 형변환했다.

    직접 작성한 Test에서 5,000,000 Tick을 확인했고 전체 361개 검사가 통과했다.

- 정수가 아닌 Tick 또는 표현 범위를 벗어난 값의 처리:
  (uint64_t)hz * ms로 중간값을 계산하고, 1000으로 나눈 나머지가 0인지 확인한다.

    예를 들어 1500Hz, 1ms는 1.5 Tick이므로 실패한다.

    최종 결과가 1보다 작거나 INT32_MAX를 초과하면 실패한다.

    실패할 때는 기존 *out 값을 변경하지 않는다.

## 주기 판단

아래는 1 Tick = 1 ms인 별도 분석 입력입니다. 시작 기준 시각은 0, 주기는 10 Tick입니다.

| now | 실행 여부 | 호출 이후 last | 판단 근거 |
| --- | --- | --- | --- |
| 9 |X |0 |경과 시간이 9 Tick이므로 주기 10 Tick에 도달하지 않았다. |
| 10 |O |10 |정확히 10 Tick 경과했으므로 만료로 판단하고 last를 10으로 갱신한다. |
| 35 |O |35 |25 Tick 경과했지만 지연된 주기를 반복 처리하지 않고 한 번만 실행한다. |
| 36 |X |35 |마지막 기준 시각 35에서 1 Tick만 지났다. |
| 45 |O |45 |마지막 기준 시각 35에서 정확히 10 Tick 경과했다. |

- Counter Wrap 직전·직후에 비교할 값:
  last = UINT32_MAX - 4, period = 10으로 설정한다.

    now = UINT32_MAX - 2일 때는 2 Tick 경과하므로 false이다.

    now = 4일 때는 9 Tick 경과하므로 false이다.

    now = 5일 때는 정확히 10 Tick 경과하므로 true이며 last는 5로 갱신된다.

    now - last의 unsigned 차이를 사용하여 Wraparound를 처리한다.

- 관측을 Counter 한 바퀴보다 오래 중단하면 알 수 없는 정보:
  32-bit Counter가 몇 번 Wraparound되었는지 알 수 없으므로 실제 경과 Tick과 누락된 주기 횟수를 정확히 복원할 수 없다. 따라서 실제 경과가 2^32 Tick 미만이며 같은 Counter와 일정한 Clock을 사용한다는 전제가 필요하다.

- Disabled 중 기준 시각을 갱신하는 이유:
 센서 수집이 비활성화된 동안에도 주기가 만료되면 last_sample을 갱신한다.

    이를 통해 ENABLE이 다시 활성화되었을 때 과거에 누락된 수집을 즉시 보충하지 않고, 갱신된 기준 시각부터 다음 수집 주기를 판단하도록 한다.

    ENABLE이 비활성인 경우에는 센서를 읽지 않고 sample_count도 증가시키지 않는다.

- 250 ms Toggle 간격과 LED ON/OFF 전체 주기의 차이:
    250ms는 LED 상태를 한 번 반전시키는 간격이다.

    OFF → ON → OFF가 완료되는 이상적인 전체 주기는 500ms이다.

    따라서 Toggle 주기와 전체 점멸 주기를 구분해야 한다.

- Polling 호출이 늦어질 때 보장되는 것과 보장되지 않는 것:
  period_due()는 주기 이상 경과했는지 확인하고, 만료되었다면 현재 시각을 새 기준으로 저장한다.

    여러 주기가 지났더라도 한 번의 Step에서 해당 작업을 최대 한 번 처리한다.

    그러나 함수가 호출되지 않은 시간 동안 정확한 주기로 작업이 실행되는 것은 보장하지 않는다.

    실제 실행 지연, 실행 시간 및 하드웨어 시간 정확도는 별도의 측정이 필요하다.

## 실제 Demo 결과

`make demo` 실행 후 `make -s demo > timing.csv`로 CSV를 저장하고, 위 분석과 결과를 비교합니다.

- CSV: 실제 제출 파일은 logs/demo.csv이며, 수정한 Host 모델에서 직접 생성하였다.

- 불일치가 있었다면 근본 원인과 수정한 함수: 초기 Starter에서 시간 계산, GPIO 제어, Collector 상태 관리 및 주기 처리 오류가 있었으나, 이를 수정한 뒤 최종 테스트에서 361 checks, 0 failures를 확인하였다. Demo 결과에서도 요구된 센서 수집 및 LED의 독립 동작을 확인하였다.

- 실제 시간 측정인지, 입력 시각을 넣은 Host Simulation인지: 실제 TC387QP 하드웨어 측정이 아닌 C11 Host Simulation이다. host/demo.c가 주입한 가상 시각에 따른 결과를 관찰하였다.