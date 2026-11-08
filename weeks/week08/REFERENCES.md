# 08회차 | TC387QP · LIS3DH 근거 찾기

## 1. 회로도에서 통신 경로 찾기

- 지정 배선은 `TC387QP P02.4 ↔ LIS3DH SDA`, `TC387QP P02.5 ↔ LIS3DH SCL`. P02.4는 Port 02의 Pin 4. Datasheet의 LFBGA292 표에서 두 Pin의 I2C0 입력·출력 기능 찾기. [TC38x Datasheet, Table 2-27, pp. 179–180](https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-datasheet-en.pdf#page=179)
- PinMap은 각 Port와 Pin을 담당 통신 기능에 연결하는 제조사의 대응표. 두 행에서 모듈 이름, Port 번호, Pin 번호, 입력 경로, 출력 기능을 찾고, `src/sensor.c`의 역할별 지정값과 비교. [Infineon iLLD · TC38x LFBGA292 I2C PinMap, L38·L42](https://github.com/Infineon/illd_release_tc3x/blob/ac8fb805633894b89819b953516b4e94387056fd/src/BaseSw/iLLD/TC3xx/Tricore/_PinMap/TC38x/IfxI2c_PinMap_TC38x_LFBGA292.c#L38-L42)
- 우선 확인할 질문 — TC38x Table 2-27의 같은 Pin 아래에는 출력 기능이 여러 개 있음. P02.4에서 I2C0 Data를, P02.5에서 I2C0 Clock을 내보내는 행을 각각 찾고, 두 행의 Ctrl. 열을 그대로 기록. iLLD는 그 선택을 어떤 이름으로 표현하는가?
- 전기적 동작은 별도로 확인. Open-drain은 Low를 구동하거나 선을 놓아서 Pull-up 저항이 High로 올리도록 하는 방식. 두 기능을 어떻게 고르는지 iLLD 함수의 `mode`와 `outSelect` 인자에서 각각 찾기. [NXP UM10204 §3.1.1, p. 8](https://www.nxp.com/docs/en/user-guide/UM10204.pdf#page=8) · [Infineon iLLD · IfxI2c_initSclSdaPin, L203–211](https://github.com/Infineon/illd_release_tc3x/blob/ac8fb805633894b89819b953516b4e94387056fd/src/BaseSw/iLLD/TC3xx/Tricore/I2c/Std/IfxI2c.c#L203-L211)
- PCB(인쇄회로기판)의 Connector 번호는 그 Board의 회로도에서 확인. 과제에서 정한 MCU Pin·기능, Host(PC)에서 관찰한 설정, 실제로 측정한 배선을 각각 표시.
- ST의 Pin 설명에서 같은 Pin이 I2C와 4-wire SPI에서 어떤 역할을 하는지 확인. 각 방식에서 Clock을 보내는 선, Data가 이동하는 선, 특정 Sensor를 선택하는 방법은 무엇인가? [LIS3DH Datasheet §6, Table 14](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=24) · [§6.2.1, Figure 8](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=28)
- 요청서의 배선 조건과 TC387QP의 통신 기능 대조. Sensor에서 값을 읽는 구간과 PC에 기록을 보내는 구간에 각각 어떤 모듈이 필요한가? SPI를 선택하면 어떤 선이 추가로 필요한가? [Infineon TC387QP 제품 정보](https://www.infineon.com/part/SAK-TC387QP-160F300S-AE) · [I2C](https://documentation.infineon.com/aurixtc3xx/docs/ijt1710345337103) · [QSPI](https://documentation.infineon.com/aurixtc3xx/docs/isf1702560910053) · [ASCLIN](https://documentation.infineon.com/aurixtc3xx/docs/dma1710341265971)

## 2. Sensor 선택과 Register 선택 구분하기

- 요청서의 `SA0` 연결에 해당하는 행 찾기. 7-bit Device Address와 Bus에 나타날 Read·Write Byte를 각각 기록. 세 값에서 같은 Bit와 달라지는 Bit는 어디인가? [LIS3DH §6.1.1, Table 16](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=25)
- `WHO_AM_I` Register의 **주소**와 **읽으면 기대하는 값**을 따로 찾기. Bus가 응답했지만 다른 값이 돌아왔다면 무엇까지 확인된 것인가? [LIS3DH §8.5, Table 24](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=34)
- Host API의 주소 인자 설명과 Infineon iLLD의 `deviceAddress` 예시 대조. 실제 Board에서 어느 표현을 쓸지는 어떤 문서로 확인할 수 있는가? [Infineon iLLD Device 설정 예시](https://github.com/Infineon/illd_release_tc3x/blob/main/src/BaseSw/iLLD/TC3xx/Tricore/I2c/I2c/IfxI2c_I2c.h)

## 3. 요청한 동작을 Register 설정으로 옮기기

- 요청서에서 측정 주기, 사용할 축, 측정 범위, 해상도, Byte 순서 조건에 밑줄 긋기. 각 조건을 담당하는 Bit의 이름과 위치는 무엇인가? Low Byte와 High Byte를 읽는 사이에 값이 바뀌는 문제는 어느 Bit로 막을 수 있는가? [LIS3DH §8.8, Tables 29–31](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=35) · [§8.11, Tables 37–38](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=37)
- Mode별 표에서 요청한 해상도의 행 찾기. 첫 값이 안정될 때까지 몇 번의 측정 주기가 필요한가? 실제 Board에서 새 값의 도착을 확인할 Register는 무엇인가? [LIS3DH §3.2.1, Tables 10–11](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=16) · [§8.15, STATUS_REG](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=39)

## 4. X축 한 번 읽기를 그림과 숫자로 확인하기

- 여러 Byte를 읽는 ST 표에서 Sub-address와 repeated START 찾기. **어떤 Register부터** 읽는지, **다음 Register로 자동 이동**해야 하는지를 각각 어떻게 표시하는가? [LIS3DH §6.1.1, Table 20](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=26) · [NXP UM10204 §3.1.10, Figure 13](https://www.nxp.com/docs/en/user-guide/UM10204.pdf#page=14)
- 수신 확인을 뜻하는 ACK와 전송을 마칠 때 쓰는 NACK의 발신자 표시. 주소 뒤의 NACK와 마지막 Data Byte 뒤의 NACK는 각각 어떤 상황을 뜻하는가? [NXP UM10204 §3.1.6](https://www.nxp.com/docs/en/user-guide/UM10204.pdf#page=10)
- X축의 두 Byte를 종이에 그려 유효한 Bit와 부호 Bit에 표시. 직접 고른 음수 한 개를 Sensor 출력으로 표현한 뒤, 요청서의 UART 기록 규칙으로 바꾸면 어떤 값이 남는가? [LIS3DH §8.16, OUT_X_L/H](https://www.st.com/resource/en/datasheet/lis3dh.pdf#page=40) · [ST 공식 Driver의 High-resolution 변환 함수](https://github.com/STMicroelectronics/lis3dh-pid/blob/master/lis3dh_reg.c)
