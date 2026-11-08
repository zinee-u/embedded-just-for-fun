# 08회차 · TC387QP에서 LIS3DH X축 값 읽기

- 목표 — TC387QP Datasheet를 읽고 SDA·SCL 각각의 GPIO 번호, 출력 기능과 구동 방식을 설계. 두 Byte의 Sensor 입력이 기존 UART 기록 한 줄이 되기까지 추적
- 대상: TC387QP가 I2C Controller, 외부 LIS3DH가 Sensor인 설계
- 실행: 실제 Board 없이 PC에서 C11 Compiler와 Make 사용
- 도구가 없으면: Linux/WSL에서는 C Compiler와 Make, macOS에서는 Command Line Tools 필요

## 1. 첫 실행

- 이 README와 Makefile이 있는 Folder에서 아래 명령을 한 줄씩 실행
- 첫 번째 명령: 수정 전 어느 요구 조건에 실패하는지 확인. Starter의 FAIL은 의도한 관찰값
- 두 번째 명령: I2C Pin 배정, Register 거래, UART 줄 수 차례로 확인. 첫 명령이 실패해도 실행 가능

```sh
make test
make demo
```

- Demo 첫 줄의 I2C module·SDA·SCL과 OK/FAIL 확인. 둘째 줄에서 각 선의 출력 선택 O번호와 구동 방식 확인. Pin 설정에 성공한 뒤 읽을 항목: `R` = 결합 읽기, `W` = 쓰기, `address7` = 장치 주소, `sub` = Sensor 내부 Register 선택값
- 예: 입력 `OUT_X_L=0x00, OUT_X_H=0xFF, seq=42` → 기대 UART `S,42,2032,0` 뒤에 LF 한 개
- 비교 방법: UART 값만 기록하지 말고, 처음으로 예상과 달라진 I2C 호출 순번도 기록

## 2. 작업 범위

| File | 작업 |
|---|---|
| `src/sensor.c` | Board Pin 설정과 Sensor 초기화·변환·기록 함수 수정 |
| `tests/test_student.c` | 고정 Test와 다른 상황 두 가지 이상 추가. 실패 개수 반환 |
| `tests/test_sensor.c` | 고정 Test. 입력·기대값 확인용 |
| `include/sensor.h`, `include/platform.h` | 함수의 입력·출력 확인용 |
| `host/host.c`, `host/host.h` | 실제 Board 대신 I2C·UART 호출을 기록하는 Host. 수정 대상에서 제외 |

## 3. 함수별 요구 조건

- 회로 요구 사항: `TC387QP I2C0 P02.4 SDA ↔ LIS3DH SDA`, `I2C0 P02.5 SCL ↔ LIS3DH SCL`. 두 신호선에 각각 Pull-up 저항. MCU와 Sensor의 GND 공통.
- `board_init_sensor_bus()`: `I2cPins`에 module 번호와 각 신호의 Port 번호, Pin 번호, 출력 선택(output), 구동 방식(drive)을 지정한 뒤 `bus_configure`로 적용. 반환값으로 성공 여부 전달. P02.4는 Port 2의 Pin 4. 적용에 실패하면 ID 읽기와 UART 기록 0건.
- 두 선에서 TC38x Datasheet 표 2-27에 맞는 출력 기능을 선택하고, I2C 전기 규격에 맞는 구동 방식을 적용. `OutputSelect`는 Host가 기록하는 O번호, `OutputDrive`는 Pin이 전기적으로 선을 구동하는 방식. 실제 Register 주소·Bit Encoding은 Hardware 문서에서 확인.
- 첫 실행에서 Demo 첫 줄을 요구 배선과 비교. Port·Pin만 고친 결과, 출력 선택과 구동 방식까지 고친 결과를 같은 입력에서 차례로 기록. 이어서 Sensor Register 동작 조사.

- `sensor_init`: 매 호출마다 `ready=false`에서 시작. ID 확인 → 출력 형식 설정 → 측정 시작. 모두 성공한 경우에만 `true`와 `ready=true`. 각 실패 지점에서 즉시 중단
- `sensor_decode_x(low, high)`: 먼저 받은 Byte가 Low Byte. 두 Byte의 상위 12 bit를 부호 있는 정수 `-2048..2047`로 반환. 가속도 단위로 변환하지 않음
- `sensor_sample(sensor, seq)`: 준비된 Sensor에서 안정된 X축 두 Byte를 한 거래로 읽고, `value=raw+2048`로 변환. 성공 시 UART에 정확히 한 줄 `S,<seq>,<value>,0\n`을 기록하고 `true` 반환
- 미초기화·읽기 실패: `false` 반환, 새 UART 기록 0개. 실패한 입력으로 직전 기록을 변경하지 않음
- `NULL` Sensor: 초기화·측정 모두 `false`. `seq`는 호출자 입력(0..65535). 함수 내부에서 새 번호를 만들지 않음

## 4. Board와 Host의 경계

- Board 전제: LIS3DH의 CS는 High, SDO/SA0은 Low. 이 배선에서 I2C 사용. Host 회로는 위 Port/Pin으로 고정. 실제 Board에서 사용할 때는 그 두 Pin이 나오는 Connector와 전압을 회로도에서 확인
- Host API 전제: 주소는 R/W bit를 제외한 7-bit 값. `bus_read` 한 번에 Register 선택, repeated START, 읽기, STOP 포함
- Host 입력: Test가 안정된 Sample을 미리 제공. 초기화 직후 Data-ready 대기 시간, TC387QP의 실제 Pin Register·Clock·Pull-up·전압·파형은 별도 실측 대상
- 제공 UART: `uart_record(seq, value)`는 유효 범위에서 한 줄 전체 수용. `\n`은 문자 두 개가 아니라 LF 한 Byte
- 고정 Test 통과는 Port·Pin 번호, 출력 선택, 구동 방식, Register 거래를 Host에서 확인한 결과. 실제 TC387QP에서 파형을 측정한 결과와 구분

- 고정 Test 구성: 앞의 다섯 Test에서 I2C0/Pin 번호, 두 선의 출력 선택, 구동 방식, 실제 설정으로 ID 읽기, 뒤바뀐 배선의 응답을 확인. 뒤의 14개 Test에서는 정상 시험 배선을 먼저 적용해 Sensor 구현을 독립적으로 검사.
- 제조사 PinMap에서 확인할 이름: `IfxI2c0_SDA_P02_4_INOUT`, `IfxI2c0_SCL_P02_5_INOUT`. [Infineon TC38x LFBGA292 PinMap](https://github.com/Infineon/illd_release_tc3x/blob/main/src/BaseSw/iLLD/TC3xx/Tricore/_PinMap/TC38x/IfxI2c_PinMap_TC38x_LFBGA292.c#L38-L44)

## 5. 추가 Test 시작점

- `host_reset()`: Test마다 초기 상태 생성. `host_set_id()`, `host_set_x(low, high)`: 입력 설정
- `host_fail_on(n)`: 전체 I2C 호출 중 n번째만 실패 처리(1부터 시작). `host_event(n)`: n번째 호출 기록 확인
- `host_uart_count()`, `host_last_frame()`: 수용된 UART 줄 수와 마지막 줄 비교
- 검증 순서: 수정 전 관찰 → 가설 한 개 선택 → 가설을 구분할 입력 추가 → 수정 → 같은 입력으로 재실행
- 정리: `make clean`으로 생성된 실행 File 제거. 제출 대상에서 `build/` 제외
