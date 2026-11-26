# 08회차 실행 환경과 검증 기록

## 1. 실행 환경

- 작성자 / GitHub ID: 진동휘 / whiya98
- Host OS / Version: Microsoft Windows 10.0.26300.9974
- 개발 환경: Visual Studio Code, MSYS2 UCRT64
- C Compiler / Version: GCC 16.1.0 (Rev5, Built by MSYS2 project)
- GNU Make / Version: 4.4.1
- C 표준: C11
- 시작 자료: upstream/main (Commit: 28bf890)
- 작업 Branch: solve/week08-whiya98
- 실행 위치: submissions/week08/whiya98/
- Target MCU: TC387QP
- Sensor: LIS3DH
- 실제 Board 사용 여부: Host만 사용

## 2. 실행 및 검증 결과

| 실행 대상 | 명령 | 관찰한 결과 / 종료 상태 | Log 경로 |
| --- | --- | --- | --- |
| 수정 전 Test | `make test` | 4/19 PASS, Exit 2 | `logs/before-test.log` |
| 수정 전 Demo | `make demo` | Board 설정 실패, UART 0건, Exit 0 | `logs/before-demo.log` |
| 수정 후 Test | `make test` | 기본 19/19 PASS, 추가 3/3 PASS, Exit 0 | `logs/after-test.log` |
| 수정 후 Demo | `make demo` | S,42,2032,0 출력, Exit 0 | `logs/after-demo.log` |

수정 전에도 C Compiler 오류는 발생하지 않았으며, 실행 결과가 요구 조건을 만족하지 못하여 Test가 실패하였다.

## 3. 단계별 검증

| 수정 단계 | 기본 Test 결과 | Log |
| --- | --- | --- |
| Starter | 4/19 PASS | before-test.log |
| Pin 번호 수정 | 5/19 PASS | pin-only-test.log |
| Output 및 Drive 수정 | 8/19 PASS | board-fixed-test.log |
| Sensor 초기화 수정 | 12/19 PASS | init-fixed-test.log |
| Sample 읽기 수정 | 14/19 PASS | sample-fixed-test.log |
| Sensor Decode 수정 | 19/19 PASS | decode-fixed-test.log |
| 추가 Test 작성 | 기본 19개 및 추가 3개 PASS | after-test.log |

## 4. 추가 Test

| 추가 Test 이름 | 확인할 가설 / 경계 | 입력과 상태 | 실행 전 예상 | 실제 결과 |
| --- | --- | --- | --- | --- |
| ignore lower 4 bits | 하위 4-bit가 변환 결과에 영향을 주지 않는지 확인 | 0x0120, 0x0125, 0x012F, 0xFFFF | 각각 18, 18, 18, -1 | PASS |
| preserve previous UART frame | 읽기 실패 시 이전 UART 기록 유지 | 정상 Sample 후 5번째 I2C 거래 실패 | UART Count 1 유지, 이전 Frame 유지 | PASS |
| recovery after I2C read failure | 일시적인 통신 실패 이후 정상 복구 | 정상 → 실패 → 정상 | UART Count 1 → 1 → 2 | PASS |

## 5. Pin 및 출력 설정 근거

- MCU Package: LFBGA292 기준
- Datasheet: Infineon TC38x Datasheet, Table 2-27 Port 02 Functions
- SDA: P02.4, I2C0_SDA, O5
- SCL: P02.5, I2C0_SCL, O5
- iLLD PinMap: IfxI2c0_SDA_P02_4_INOUT, IfxI2c0_SCL_P02_5_INOUT
- 출력 선택: PORT_ALT5
- 출력 방식: OPEN_DRAIN
- Open-drain은 Low를 직접 구동하고 출력이 해제되면 Pull-up 저항을 통해 High를 형성한다.

## 6. Demo 설정 비교

| 단계 | Pin 설정 | 출력 기능 및 구동 방식 | 결과 |
| --- | --- | --- | --- |
| 수정 전 | SDA=P02.5, SCL=P02.4 | SDA=O0/open-drain, SCL=O5/push-pull | FAIL |
| Pin 번호 수정 | SDA=P02.4, SCL=P02.5 | SDA=O0/open-drain, SCL=O5/push-pull | FAIL |
| 출력 기능 및 방식 수정 | SDA=P02.4, SCL=P02.5 | SDA=O5/open-drain, SCL=O5/open-drain | OK |
| 전체 수정 | SDA=P02.4, SCL=P02.5 | SDA=O5/open-drain, SCL=O5/open-drain | OK, UART 정상 출력 |

Output Select와 Output Drive는 동시에 수정하였다. Output Select만 수정한 단계의 별도 Demo는 미수행하였으며, 각 설정의 독립적인 오류 검증은 제공된 고정 Test를 통해 확인하였다.

## 7. 검증 범위 및 제한 사항

- 배선 출처: 과제 README의 Host 모의 배선
- Host 검증 범위: Pin 설정, Sensor 초기화, Register 거래, 데이터 변환, UART 출력, 통신 실패 처리 및 복구
- I2C 모듈 번호만 잘못 선택한 독립 Test: 미수행
- iLLD의 실제 Open-drain Mode 설정 구현: 별도 확인하지 않음
- 실제 Board 회로도, Connector, 전압, Pull-up 저항값, I2C Clock 및 통신 파형: 미측정
- 실제 TC387QP Board 및 LIS3DH Sensor를 사용한 검증은 수행하지 않았다.