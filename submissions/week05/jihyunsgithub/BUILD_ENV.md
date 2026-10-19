# Build 환경 기록

- 작성자 / GitHub ID:jihyunsgithub
- 실행 OS 및 Version: macOS 14.5 (Build 23F79)
- C Compiler와 Version (`cc --version` 또는 `gcc --version`): Apple Clang 15.0.0 (clang-1500.3.9.4), Target: arm64-apple-darwin23.5.0
- Make Version (`make --version`):GNU Make 3.81
- 실제 Build 명령과 Flag:
    실행 명령: make clean, make, make test, make demo

    Compiler: cc (Apple Clang)

    C 언어 표준: -std=c11

    Warning 및 Build 옵션: -Wall -Wextra -Wpedantic -Werror -O2

    Include 경로: -Iinclude -Ihost -Itests

    소스 파일: src/timebase.c, src/board_io.c, src/collector.c, host/fake_platform.c

    Test Build 추가 소스: tests/test_mission.c, tests/test_student.c

    Demo Build 추가 소스: host/demo.c

    Test 실행 파일: build/mission-tests

    Demo 실행 파일: build/mission-demo

- 테스트한 Commit: 5af4ce2 (Implement week05 TC387QP polling collector, 코드 및 Test를 저장한 첫 번째 Commit)

## 실행 결과

| 실행 | Exit code | Check 수 / 실패 수 | 증빙 |
| --- | --- | --- | --- |
| `make` |0 | 해당 없음 |제출 폴더에서 실행, Nothing to be done for 'all' 출력 |
| `make test` |0 |361 checks / 0 failures |logs/test.log |
| `make demo` |0 | 해당 없음 |logs/demo.csv, Terminal Demo 출력 |

Test 실행 파일과 Make의 Exit code를 혼동하지 마세요. 출력은 실제로 실행한 결과를 기록합니다. Binary를 올리지 말고 실패 원인·수정 내용·결과를 PR에 적으세요.

## 직접 추가한 Test

| Case | 요구사항 | 입력·전제 | 예상 결과 | 실제 결과 | 추가한 이유 |
| --- | --- | --- | --- | --- | --- |
| 1 |ticks_from_ms()는 중간 곱셈의 32-bit Overflow를 방지하고 정확한 Tick을 반환해야 한다. |hz=5,000,000, ms=1,000, out=123 |true 반환, out=5,000,000 |2개 CHECK 통과 |중간 곱셈값 5,000,000,000은 32-bit 범위를 넘지만 최종 결과는 유효하다. 곱셈 전에 uint64_t로 형변환했는지 검증하기 위해 선정했다. |
| 2 |period_due()는 정확한 만료 경계를 포함하고, 만료 전에는 last 값을 보존해야 한다. |last=500, period=25, now=524, 525 |524에서는 false, last=500. 525에서는 true, last=525 |4개 CHECK 통과 |주기 만료 직전과 정확한 만료 시점을 비교하여 >= 조건 및 기준 시각 갱신을 검증하기 위해 선정했다. |

## Host 검증과 실물 검증의 구분

- Host에서 확인한 내용:
    C11 Host 환경에서 STM 주파수 기반 Tick 계산, 64-bit 중간 곱셈, 유효 Tick 범위 및 32-bit Counter Wraparound를 검증했다.

    GPIO의 Active LOW 동작, 초기 HIGH Latch 설정, ENABLE Pull-up 및 다른 Port Bit 보존 여부를 검증했다.

    Collector의 초기화, 상태 초기화, ENABLE 판정, 주기적 센서 수집과 독립적인 LED 제어를 검증했다.

    make test 실행 결과 총 361 checks, 0 failures를 확인했다.

    Host Demo에서 250ms 시점에 ENABLE이 비활성인데도 LED가 Toggle되는 것을 확인했다.

    Demo 출력에서 모든 delay_calls 값이 0임을 확인했다.

- 실제 TC387QP에서 실행했는지 여부: 해당 없음. C11 Host Simulation에서만 실행했다.
- 실제 Board / Silicon / iLLD Version (실행한 경우): 해당 없음. 실제 하드웨어 실행을 수행하지 않았다.
- 아직 확인하지 못한 내용:
    실제 TC387QP의 STM Clock 설정과 Clock 정확도

    실제 Board의 GPIO 전압, LED 출력 파형, ENABLE Pull-up 동작

    실제 MCU에서의 실행 시간 및 Polling 지연
 
    Startup, Watchdog, Interrupt, Multi-core 환경에서의 동작