# 08회차 Sensor Register 읽기 개선 요청서

TC387QP · LIS3DH  /  I2C · SPI · UART

## 기울기 시험 기록을 믿을 수 있게 만들기

> 시험팀 요청  |  장치를 왼쪽으로 기울였을 때 방향이 보존된 기록이 필요함. Sensor 읽기에 실패한 시점에는 기록을 남기지 않아야 함.

### 어떤 제품인가

- LIS3DH는 가속도를 세 축으로 측정하는 Sensor. 이번 변경 범위는 좌우 방향에 배정한 X축 한 개.
- TC387QP는 Sensor에서 값을 읽는 MCU(측정과 통신을 제어하는 작은 연산 장치). PC는 MCU가 보낸 UART(직렬 통신) 기록을 표시하는 역할.
- Host Mock은 PC에서 장치의 응답을 흉내 내는 도구. C Compiler는 C로 작성한 내용을 실행 가능한 형태로 바꾸고, Make는 준비된 실행 절차를 수행. 실제 Board 없이 전 과정 실행 가능.

```text
LIS3DH  -- I2C -->  TC387QP  -- UART -->  PC
                                           S,42,2032,0
```

### 학습 목표

1. TC38x Datasheet(부품 설명서) 표 2-27에서 P02.4·P02.5의 출력 행 O5 찾기. 각 GPIO(외부와 연결되는 MCU 단자)에 ALT5와 Open-drain 지정. 잘못된 기능·구동 방식을 한 항목씩 바꿔 비교.
2. 배선을 보고 I2C와 SPI 중 사용할 방식 결정. Sensor Datasheet에서 Register(기능이나 측정값을 담는 칸)를 찾아 설정·읽기 순서 작성.
3. Sensor의 두 Byte가 UART 한 줄로 바뀌는 과정 재현. 관찰한 현상, 근거로 확인한 원인, 해결방안을 각각 기록.

> 한 줄 읽기  |  S는 Sample(측정 한 건), 42는 호출자가 붙인 순번, 2032는 전송용 X 값, 마지막 0은 이 과제에서 고정한 상태값. 기록 끝에 줄바꿈 한 번.

## 첫 실행에서 원본의 현상 남기기

> 목표  |  원본에서 Pin 설정 결과, Sensor 거래 횟수, UART 출력 줄 수를 각각 기록.

1. ZIP은 압축을 푼 week08, GitHub 저장소는 weeks/week08에서 Terminal(명령 입력 창) 열기. 아래는 macOS·Linux 또는 Windows WSL 기준.
2. 아래 명령을 한 줄씩 실행. work는 PC 연습용 사본. PR 제출 위치와 절차는 SUBMISSION.md 확인.

```text
cp -R starter work
cd work
make demo
make test
```

- make demo로 처음 거부된 단계 확인. S,로 시작하는 줄이 없으면 UART 0줄로 기록.
- make test는 여러 조건의 실제값과 기대값을 비교하는 명령. 첫 실행의 FAIL은 조사 대상. Compiler 오류가 먼저 나오면 C Compiler(cc)와 Make 설치 상태 확인.
- src/sensor.c에서 먼저 두 Pin의 번호·출력 기능·구동 방식만 고치고 make demo 재실행. 이후 Sensor 읽기·값 변환을 고치고 같은 입력으로 다시 비교.

### File을 여는 순서

| File | 첫 번째로 확인할 내용 |
| --- | --- |
| README.md (work 안) | 실행 명령과 각 File의 역할 |
| src/sensor.c | Pin 설정 → 장치 식별 → X 읽기 → 값 변환 → 기록 |
| tests/ 안의 검사 Code | 실패한 검사 이름, 입력, 통과 조건 |
| PROBLEM_DEFINITION.md | 수정 전 관찰과 검증할 가설을 적는 양식 |

## TC387QP의 Pin 번호로 I2C0 배선 설계하기

> 시험 배선  |  MCU의 P02.4는 LIS3DH의 SDA에, P02.5는 SCL에 연결. P02.4는 Port 02의 4번 Pin. 두 장치의 GND(전압 기준점) 공유.

| 연결 | 역할과 필수 조건 |
| --- | --- |
| I2C0 SDA · P02.4 | 측정값과 명령을 주고받는 Data 선. TC387QP의 I2C0 기능으로 연결. |
| I2C0 SCL · P02.5 | 전송 박자를 맞추는 Clock 선. Data 선과 구분해 설정. |
| 두 Pin의 전기 설정 | 표 2-27에서 SDA·SCL의 출력 행 O5 확인. 각 Pin에 ALT5(출력 기능 5번)와 Open-drain 지정. 두 선에 각각 외부 Pull-up 저항 사용. |
| LIS3DH의 CS / SA0 | CS는 High로 고정해 I2C 선택. SA0은 Low로 고정해 장치 주소의 마지막 Bit를 0으로 설정. |

### 제출할 설계

- SDA·SCL 각각의 Port·Pin·출력 행·신호명·구동 방식을 BUS_ANALYSIS.md에 기록. O5는 어느 신호를 보낼지 선택. Open-drain은 Low를 만들거나 선을 놓는 방식. 선을 놓으면 Pull-up이 High 유지.
- I2C와 4선 SPI의 신호선·장치 선택·읽기 방식 비교. SPI의 SCLK는 Clock, MOSI·MISO는 방향별 Data, CS는 장치 선택 선. SPI를 쓰려면 바꿔야 하는 배선 한 줄 기록.

> 실제 Board에서는 회로도로 Connector 위치 확인. 출력 전압과 SDA·SCL 파형은 계측기로 확인.

[Infineon · TC38x Datasheet — BGA292, 표 2-27 Port 02 Functions](https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-datasheet-en.pdf#page=179)

[ST · LIS3DH Datasheet — §6 Digital interfaces](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=24)

## 측정 한 건의 성공·실패 조건 구현하기

> 변경 대상  |  work/src/sensor.c의 board_init_sensor_bus()에서 I2cPins의 sda·scl 각각에 port·pin·output·drive 지정. 이후 Sensor 거래에서 bus_read·bus_write의 첫 인자는 7-bit 장치 주소.

| 단계 | 요구 조건 |
| --- | --- |
| 1 · 장치 준비 | 먼저 I2C0 Pin 설정. 성공 후 준비 상태 해제 → 제품 식별 → 측정 설정 → 측정 시작. Pin 설정 거부 시 Sensor 거래와 UART 출력 0건. |
| 2 · 측정 설정 | High-resolution(12-bit), ±2 g 범위, Low Byte 먼저. 한 측정값의 두 Byte를 읽는 중에 값이 섞이지 않게 설정. 이후 100 Hz, X·Y·Z 활성화. |
| 3 · X 읽기 | 준비 완료 후 한 번의 연속 읽기로 X의 Low·High Byte 확보. 준비되지 않았거나 읽기 실패 시 UART 출력 0줄. |
| 4 · 값과 기록 | 두 Byte를 부호 있는 X 값(-2048~2047)으로 복원. 07회차 기록 형식에 맞게 2048을 더해 0~4095로 옮긴 뒤 한 줄 출력. |

### 구현 전에 찾을 근거

- LIS3DH Datasheet에서 WHO_AM_I, CTRL_REG4, CTRL_REG1, OUT_X_L, OUT_X_H 검색. 각 Register의 주소와 판단에 쓴 Bit를 BUS_ANALYSIS.md에 기록.
- 준비(ready)는 설정 완료를 뜻함. 첫 측정값이 이미 도착한 상황을 Host에서 재현. 성공·실패 갈림길을 넣어 수집부터 UART까지 순서도 작성.

[ST · LIS3DH Datasheet — Register mapping과 Register description](https://www.st.com/resource/en/datasheet/lis3dh.pdf)

> g는 중력가속도 단위. UART의 value는 전송을 위해 범위를 옮긴 정수. 정지 상태의 측정값을 보정하거나 g로 환산할 필요 없음.

## 같은 입력으로 수정 전후 비교하기

> 인수 조건  |  아래 입력에서 출력 비교. Byte는 왼쪽부터 Low, High 순서. 0x는 16진수 표기이며 0x10은 십진수 16.

| Host 입력 | 기대 결과 |
| --- | --- |
| Pin 교환 / 한 선의 출력 기능 오류 / 한 선의 구동 방식 오류 | 각각 독립적으로 거부. 식별 Register 접근 전 중단. Sensor 거래 0건, UART 0줄. |
| 지정 Pin, seq=42, X Byte = 00 FF | X=-16, UART 한 줄 S,42,2032,0 뒤에 줄바꿈 |
| 00 80 / 00 00 / F0 7F | 각각 전송값 0 / 2048 / 4095. 최솟값·중앙·최댓값 확인 |
| 장치가 응답하지만 식별값 불일치 | 초기화 실패, 측정 설정 진행 중단, 기록 0줄 |
| 설정 쓰기 실패 / X 읽기 실패 | 실패한 시도에서 측정 기록 0줄. 다음 시도에서 정상 복구 가능 |

### Pull Request에 남길 것

1. src/sensor.c 변경. tests/test_student.c에 서로 다른 추가 Test를 2개 이상 작성. PROBLEM_DEFINITION.md에 원인별 현상 → 원인 → 해결방안과 검증 결과 기록.
2. BUS_ANALYSIS.md에 Pin별 O5 근거·구동 방식·배선표, I2C/SPI 비교표, Register 읽기 순서도 작성. BUILD_ENV.md에 실행 환경과 수정 전후 결과 기입. 각 Log(실행 기록)의 출처는 Host Mock 또는 실측으로 표시.
3. 제출 시 SUBMISSION.md의 절차와 PR_BODY.md 양식 사용. 자신의 submissions/week08/YOUR_GITHUB_ID/에서 변경 내용을 검토받는 Pull Request(PR) 작성.

> 한 가지 현상에 여러 원인이 있을 수 있음. 예: UART 숫자가 틀렸을 때, 읽어 온 두 Byte부터 틀렸는지 같은 Byte를 숫자로 바꾸는 중에 틀렸는지 따로 검사.

[수정할 C 구현](starter/src/sensor.c) · [실행 방법](starter/README.md) · [Datasheet 탐색](REFERENCES.md) · [PR 제출 안내](SUBMISSION.md)
