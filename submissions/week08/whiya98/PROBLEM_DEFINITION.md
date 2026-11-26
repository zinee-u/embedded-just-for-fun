# 08회차 문제 정의 기록

## P01. SDA 및 SCL Pin 번호 오류

| 단계 | 내 기록 |
| --- | --- |
| 현상 | 수정 전 Demo에서 SDA=P02.5, SCL=P02.4로 표시되고 Sensor 초기화 실패 |
| 원인 가설 | Pin 번호 오류 또는 출력 기능 및 출력 방식 오류 |
| 대조 Test | Pin 번호만 SDA=P02.4, SCL=P02.5로 수정 |
| 확정 원인 | board_init_sensor_bus()에서 SDA와 SCL의 Pin 번호가 뒤바뀌어 있었음 |
| 해결방안 | 요구 배선에 맞게 SDA Pin 4, SCL Pin 5 적용 |
| 검증 | 기본 Test 4/19 → 5/19 PASS. Pin 번호 Test 통과, 통신은 여전히 실패 |

관련 Log: before-demo.log, pin-only-demo.log, pin-only-test.log

## P02. GPIO 출력 기능 설정 오류

| 단계 | 내 기록 |
| --- | --- |
| 현상 | Pin 번호 수정 이후에도 SDA=O0으로 표시되며 통신 실패 |
| 원인 가설 | GPIO 출력 기능이 선택되어 I2C 주변장치 출력이 연결되지 않음 |
| 대조 Test | 고정 Test의 output_selection에서 SDA 또는 SCL의 출력 기능만 잘못 설정한 경우 검사 |
| 확정 원인 | SDA에 PORT_GPIO를 적용함. TC38x Datasheet Table 2-27의 I2C0 SDA/SCL 출력 기능은 O5 |
| 해결방안 | 두 Pin에 PORT_ALT5 적용 |
| 검증 | 최종 Board 관련 출력 기능 Test 통과. Output만 수정한 별도 Demo는 미수행 |

## P03. GPIO 출력 방식 설정 오류

| 단계 | 내 기록 |
| --- | --- |
| 현상 | SCL=O5/push-pull로 설정되어 I2C 통신 실패 |
| 원인 가설 | I2C 전기적 구동 조건 불충족 |
| 대조 Test | 고정 Test의 output_drive에서 각 Pin의 출력 방식만 Push-pull로 변경한 경우 검사 |
| 확정 원인 | SCL에 PUSH_PULL이 적용되어 있었음 |
| 해결방안 | SDA와 SCL 모두 OPEN_DRAIN 적용 |
| 검증 | Output Select 및 Output Drive 수정 후 Board 관련 Test 5개 통과, 전체 8/19 PASS |

관련 Log: board-fixed-test.log, board-fixed-demo.log

## P04. Sensor ID 검증 누락

| 단계 | 내 기록 |
| --- | --- |
| 현상 | WHO_AM_I 읽기에 성공했지만 기대 ID와 다른 경우에도 초기화가 성공할 수 있음 |
| 원인 가설 | 통신 성공 여부만 확인하고 ID 값을 비교하지 않음 |
| 대조 Test | 초기화 성공 후 ID를 0x32로 변경하여 재초기화 |
| 확정 원인 | sensor_init()에서 id == 0x33 검증 누락 |
| 해결방안 | ID가 0x33이 아니면 즉시 false 반환 |
| 검증 | init / changed ID after success PASS |

## P05. BDU 설정 누락

| 단계 | 내 기록 |
| --- | --- |
| 현상 | CTRL_REG4에 기대값 0x88이 아닌 0x08 기록 |
| 원인 가설 | BDU 활성화 Bit 누락 |
| 대조 Test | Demo의 두 번째 I2C Write 거래와 요구값 비교 |
| 확정 원인 | CTRL_REG4 Bit 7인 BDU가 0으로 설정되어 있음 |
| 해결방안 | CTRL_REG4에 0x88 적용. HR 설정은 유지 |
| 검증 | 초기화 순서 Test 통과, Demo 두 번째 거래에서 0x88 확인 |

BDU 적용 목적은 두 Byte를 읽는 동안 출력 데이터의 일관성을 유지하는 것이다. 실제 Sensor의 데이터 갱신 타이밍은 측정하지 않았다.

## P06. 초기화 Write 실패 처리 누락

| 단계 | 내 기록 |
| --- | --- |
| 현상 | CTRL_REG4 또는 CTRL_REG1 쓰기 실패 조건에서도 ready=true가 될 수 있음 |
| 원인 가설 | bus_write() 반환값을 무시함 |
| 대조 Test | 두 번째 또는 세 번째 I2C 거래에 실패 주입 |
| 확정 원인 | 기존 코드가 (void)bus_write()로 반환값을 무시하고 다음 단계 수행 |
| 해결방안 | 각 Write 실패 시 즉시 false 반환, 모든 초기화 성공 후에만 ready=true 설정 |
| 검증 | 초기화 관련 Test 5/5 PASS, 전체 12/19 PASS |

관련 Log: init-fixed-test.log, init-fixed-demo.log

## P07. Auto-increment 설정 누락

| 단계 | 내 기록 |
| --- | --- |
| 현상 | Demo 네 번째 거래에서 sub=0x28, len=2로 기록됨 |
| 원인 가설 | High Byte Register로 주소가 자동 증가하지 않음 |
| 대조 Test | Sub-address를 0x28에서 0xA8로 변경하여 거래 기록 비교 |
| 확정 원인 | sensor_sample()에서 Sub-address의 Auto-increment Bit 7을 설정하지 않음 |
| 해결방안 | OUT_X_L에 0x80을 Bitwise OR하여 Combined Read 수행 |
| 검증 | sample / one combined read PASS, 네 번째 거래 sub=0xA8 확인 |

## P08. Sample 읽기 실패 처리 누락

| 단계 | 내 기록 |
| --- | --- |
| 현상 | 읽기 실패 조건에서 UART가 출력될 수 있음 |
| 원인 가설 | bus_read() 반환값을 확인하지 않음 |
| 대조 Test | Sample 읽기 거래에 실패 주입 |
| 확정 원인 | 기존 코드가 bus_read() 반환값을 무시하고 데이터 변환 및 UART 출력 수행 |
| 해결방안 | 읽기 실패 시 즉시 false 반환 |
| 검증 | 읽기 실패 Test PASS, 전체 14/19 PASS. 추가 Test에서 이전 UART 기록 유지 및 실패 후 복구 확인 |

관련 Log: sample-fixed-test.log, after-test.log

## P09. Sensor Byte 결합 순서 오류

| 단계 | 내 기록 |
| --- | --- |
| 현상 | Low=0x20, High=0x01 입력에서 기대 Raw 18과 다른 값 반환 |
| 원인 가설 | High/Low Byte 결합 순서 오류 또는 부호 처리 오류 |
| 대조 Test | 서로 다른 Byte를 가진 양수 입력으로 결합 순서 확인 |
| 확정 원인 | sensor_decode_x()에서 Low Byte를 상위 8-bit에 배치함 |
| 해결방안 | High Byte를 8-bit Left Shift하고 Low Byte와 Bitwise OR 수행 |
| 검증 | 양수 Decode Test PASS |

## P10. 12-bit Signed Data 변환 오류

| 단계 | 내 기록 |
| --- | --- |
| 현상 | Low=0xF0, High=0xF0 입력의 기대값은 -241이지만 기존 코드는 양수 반환 |
| 원인 가설 | Unsigned 상태에서 Right Shift 후 부호 복원 누락 |
| 대조 Test | 두 Byte가 동일한 음수 입력으로 Byte 순서 오류와 부호 처리 오류 구분 |
| 확정 원인 | 상위 12-bit 추출 이후 2의 보수에 대한 부호 변환을 수행하지 않음 |
| 해결방안 | Unsigned 상태에서 하위 4-bit 제거 후 값이 2048 이상이면 4096을 차감 |
| 검증 | 음수, 최솟값, 최댓값 Test PASS. 기본 19/19 PASS 및 Demo 기대값 일치 |

Signed 음수에 대한 Right Shift의 구현 의존성을 피하기 위해 명시적인 부호 변환을 적용하였다.

관련 Log: decode-fixed-test.log, decode-fixed-demo.log

## 최종 검증 및 누락 확인

- 수정 전 기본 Test: 4/19 PASS
- 수정 후 기본 Test: 19/19 PASS
- 추가 Test: 3/3 PASS
- 전체 Test: 22/22 PASS
- make test 종료 코드: 0
- make demo 종료 코드: 0
- 최종 Demo: S,42,2032,0
- 추가 Test를 통해 하위 4-bit 무시, 이전 UART 기록 유지, 일시적인 통신 실패 후 복구 확인
- 수정 전 15개 Test 실패를 15개의 독립적인 근본 원인으로 해석하지 않음
- Output Select만 수정한 별도 Demo와 I2C 모듈 번호만 변경한 독립 Test는 미수행
- 실제 Board의 Register 설정, 전압, Pull-up, Clock, 통신 파형 및 Sensor 측정값은 미검증