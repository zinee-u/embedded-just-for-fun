# 08회차 I2C/SPI 선택과 Sensor 읽기 기록

## 1. I2C와 SPI 비교

| 확인 항목 | I2C | SPI |
| --- | --- | --- |
| 사용하는 신호선 | SDA: Data, SCL: Clock | MOSI: 송신, MISO: 수신, SCLK: Clock, CS: 장치 선택 (4-wire 기준) |
| 통신할 장치를 고르는 방법 | 장치 고유 주소와 R/W bit 사용 | CS 신호 사용 |
| Register 위치 선택 | 장치 주소 전송 후 Sub-address 전송 | Register 주소 및 읽기/쓰기 명령 전송 |
| 여러 Byte 연속 읽기 | LIS3DH Sub-address의 Bit 7 활성화 | LIS3DH SPI 명령의 MS bit 활성화 |
| 출력 방식 | Open-drain 및 Pull-up | 일반적으로 Push-pull |
| 이번 과제 배선에서 사용 가능 여부 | 가능. CS=High, SDO/SA0=Low | 불가. SPI 통신에 필요한 배선 및 CS 제어 조건이 구성되지 않음 |

### 선택 근거

이번 과제에서는 I2C를 사용하였다.

LIS3DH의 CS가 High이고 SDO/SA0가 Low인 배선 조건이므로 I2C 장치 주소는 0x18이다. SDA와 SCL 두 신호선을 이용하여 Sensor Register에 접근할 수 있다.

- 근거: STMicroelectronics LIS3DH Datasheet, Digital interfaces / I2C serial interface
- 실제 Board 적용 전 확인 사항: CS, SA0, 전원 전압, 공통 GND, Pull-up 저항 및 Connector 배선

## 2. 지정 배선과 C 설정 대조

| 신호 | LIS3DH Pin | TC387QP Port / Pin | Datasheet 출력 이름 / Ctrl. | iLLD 출력 선택 | C 출력 방식 |
| --- | --- | --- | --- | --- | --- |
| Data | SDA | P02.4 | I2C0_SDA / O5 | IfxPort_OutputIdx_alt5 | OPEN_DRAIN |
| Clock | SCL | P02.5 | I2C0_SCL / O5 | IfxPort_OutputIdx_alt5 | OPEN_DRAIN |

- MCU Package: LFBGA292 기준
- I2C Module: I2C0
- 입력 경로: I2C0_SDAA, I2C0_SCLA
- iLLD 입력 선택: Ifx_RxSel_a
- C 설정 위치: src/sensor.c의 board_init_sensor_bus()

### 출력 기능 선택 근거

TC38x Datasheet Table 2-27에서 P02.4와 P02.5의 I2C0 출력 기능이 O5임을 근거로 PORT_ALT5를 적용하였다.

Pin 번호가 올바르더라도 Output Select가 잘못되면 해당 Pin에 I2C 주변장치의 출력 기능이 연결되지 않는다.

### 출력 방식 선택 근거

I2C는 여러 장치가 동일한 신호선을 공유한다.

Open-drain 방식은 Low만 직접 구동하고, 장치가 출력을 해제하면 Pull-up 저항을 통해 High 상태를 형성한다. 따라서 서로 다른 장치가 High와 Low를 동시에 직접 구동하는 충돌을 방지할 수 있다.

### 단계별 Demo 결과

| 단계 | Pin 설정 | Output / Drive | Sensor 초기화 | UART |
| --- | --- | --- | --- | --- |
| Starter | SDA=P02.5, SCL=P02.4 | SDA=O0/open-drain, SCL=O5/push-pull | 실패 | 0건 |
| Pin 번호 수정 | SDA=P02.4, SCL=P02.5 | SDA=O0/open-drain, SCL=O5/push-pull | 실패 | 0건 |
| Output만 수정 | 별도 Demo 미수행 | - | 미확인 | 미확인 |
| Output 및 Drive 수정 | SDA=P02.4, SCL=P02.5 | SDA=O5/open-drain, SCL=O5/open-drain | 성공 | S,42,2048,0 |
| 전체 수정 | SDA=P02.4, SCL=P02.5 | SDA=O5/open-drain, SCL=O5/open-drain | 성공 | S,42,2032,0 |

### 독립적인 오류 조건 검증

| 바꾼 조건 | 예상 / 실제 설정 결과 | Sensor 거래 및 UART | 검증 근거 |
| --- | --- | --- | --- |
| 정상 설정 | OK / OK | 최종 Demo I2C 4건, UART 1건 | after-demo.log |
| SDA와 SCL 역할 교체 | FAIL / FAIL | ID 읽기 실패, UART 미호출 | swapped pins disconnect |
| I2C 모듈만 변경 | FAIL 예상 / 미검증 | 미확인 | 독립 Test 미수행 |
| 한 Pin의 출력 기능 변경 | FAIL / FAIL | ID 읽기 실패, UART 미호출 | output_selection |
| 한 Pin의 출력 방식 변경 | FAIL / FAIL | ID 읽기 실패, UART 미호출 | output_drive |

오류 조건별 I2C 거래 횟수는 별도로 기록하지 않았다. Pin 교체, 출력 기능 및 출력 방식 오류는 제공된 고정 Test의 설정 실패 및 ID 읽기 실패 조건을 통해 확인하였다.

실제 Board 측정: 미수행.

## 3. 초기화부터 UART까지의 순서

1. 전원 및 배선 조건 확인: CS=High, SA0=Low, 공통 GND, Pull-up 적용
2. board_init_sensor_bus(): I2C0, SDA=P02.4, SCL=P02.5, O5, Open-drain 설정
3. sensor_init(): ready=false 설정
4. WHO_AM_I(0x0F) 읽기 및 ID 0x33 확인
5. CTRL_REG4(0x23)에 0x88 쓰기
6. CTRL_REG1(0x20)에 0x57 쓰기
7. 모든 초기화 성공 시 ready=true 설정
8. sensor_sample(): Sub-address 0xA8로 Combined Read 수행
9. sensor_decode_x(): High/Low Byte 결합, 하위 4-bit 제거 및 12-bit 부호 변환
10. raw + 2048 계산
11. uart_record(): S,42,2032,0 및 LF 출력

Board 설정에 실패하면 이후 Sensor 접근을 수행하지 않는다.

초기화 중 실패하면 ready=false 상태를 유지하고 즉시 false를 반환한다.

Sample 읽기 실패 시 데이터 변환 및 UART 출력을 수행하지 않고 false를 반환한다.

### Register 설정 근거

| 확인 항목 | Datasheet Register / 절 | 확인한 사실 | Code 적용 |
| --- | --- | --- | --- |
| 장치 선택 | I2C serial interface | SA0=Low일 때 주소 0x18 | ADDRESS7 |
| 장치 식별 | WHO_AM_I | ID=0x33 | sensor_init |
| 출력 형식 | CTRL_REG4 | 0x88: BDU 활성화, HR 모드, ±2g, Little-endian | sensor_init |
| 측정 시작 | CTRL_REG1 | 0x57: 100Hz, X/Y/Z 활성화 | sensor_init |
| X축 데이터 | OUT_X_L / OUT_X_H | 상위 12-bit가 유효한 Signed Data | sensor_decode_x |
| 연속 읽기 | I2C serial interface | Sub-address Bit 7을 1로 설정 | sensor_sample |

BDU는 Low Byte와 High Byte를 모두 읽기 전에 데이터가 갱신되는 것을 방지한다.

Auto-increment는 OUT_X_L에서 OUT_X_H로 Register 주소를 자동으로 증가시킨다.

Repeated START는 STOP 없이 Register 선택 거래에서 읽기 거래로 전환할 때 사용된다.

## 4. 입력부터 UART 출력까지 추적

| Test / seq | Low / High Byte | 결합값 | Signed Raw | UART용 값 | 예상 출력 | 실제 출력 |
| --- | --- | --- | --- | --- | --- | --- |
| Demo / 42 | 0x00 / 0xFF | 0xFF00 | -16 | 2032 | S,42,2032,0 | 일치 |
| 양수 / 102 | 0x20 / 0x01 | 0x0120 | 18 | 2066 | S,102,2066,0 | 일치 |
| 부호 경계 / 8 | 0x00 / 0x80 | 0x8000 | -2048 | 0 | S,8,0,0 | 일치 |
| 최댓값 / 65535 | 0xF0 / 0x7F | 0x7FF0 | 2047 | 4095 | S,65535,4095,0 | 일치 |
| 읽기 실패 / 43 | 읽기 실패로 사용하지 않음 | - | - | - | 신규 출력 없음 | 이전 Frame 유지 |

각 정상 UART 출력은 마지막에 LF 한 Byte를 포함한다.

### Demo 데이터 변환

- 수신 Byte: Low=0x00, High=0xFF
- Byte 결합: 0xFF00
- Right Shift 4-bit: 0xFF0 (4080)
- Signed 변환: 4080 - 4096 = -16
- Offset 적용: -16 + 2048 = 2032
- UART 기록: S,42,2032,0

UART용 값은 Signed Raw Data에 2048을 더하여 0~4095 범위로 변환한 값이다. 물리적인 가속도 단위로 변환한 값은 아니다.

모든 입력은 Host에서 설정한 모의 Byte이며 실제 Sensor에서 측정한 데이터가 아니다.

## 5. 참고 자료

- Infineon TC38x Datasheet: https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-datasheet-en.pdf
- Infineon TC38x LFBGA292 iLLD I2C PinMap: https://github.com/Infineon/illd_release_tc3x/blob/main/src/BaseSw/iLLD/TC3xx/Tricore/_PinMap/TC38x/IfxI2c_PinMap_TC38x_LFBGA292.c
- STMicroelectronics LIS3DH Datasheet: https://www.st.com/resource/en/datasheet/lis3dh.pdf