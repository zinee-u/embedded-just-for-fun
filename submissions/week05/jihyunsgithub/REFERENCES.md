# 05회차 공식 자료와 TC387QP 연결

조회 기준일은 2026-10-11입니다. **문서의 근거 → 설계 판단 → C 코드와 Test**를 연결해 기록하세요. Hardware 사실, 과제가 정한 요구사항, Host 검증 결과는 구별합니다. 아래 `p`는 문서 하단의 **인쇄 페이지**이며 PDF Viewer의 페이지 번호와 다를 수 있습니다.

## 문서 ID와 읽는 순서

| ID | 공식 자료·Version | 이 자료에서 확인할 내용 |
| --- | --- | --- |
| D0 | [TC38x Data Sheet Addendum](https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-addendum-datasheet-en.pdf), v1.9, 2025-12 | 정확한 Part Number, Package, 제품별 사양 |
| D1 | [TC38x Datasheet](https://www.infineon.com/dgdl/Infineon-TC38x-DataSheet-v01_02-EN.pdf?fileId=5546d4626f229553016fb316e6cf748b), V1.2, 2021-03 | Package별 Pin 기능, 전원 domain, Electrical Specification |
| U1 | [TC38x User Manual Appendix](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-tc38x-usermanual-en.pdf), V2.0.0, 2021-02 | TC38x의 Module 주소와 장치별 연결 |
| U2 | [System Timer 공식 설명](https://documentation.infineon.com/aurixtc3xx/docs/wvt1713281569556), HTML, 조회일 기준 | Feature list·Functional description; STM 구조를 먼저 파악 |
| U3 | [TC3xx User Manual Part 1](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-tc3xx-part1-usermanual-en.pdf), V2.0.0, 2021-02 | Clock 분주와 Port Register 동작 |
| U4 | [TC3xx User Manual Part 2](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-tc3xx-part2-usermanual-en.pdf), V2.0.0, 2021-02 | STM Counter와 TIM0 Register |
| B1 | [Application Kit TC3X7 Manual](https://www.infineon.com/assets/row/public/documents/10/44/infineon-applicationkitmanual-tc3x7-usermanual-en.pdf), V2.0, 2018-06, Hardware V2.0 | 실제 Board의 LED·Button·Connector 배선 |

Datasheet로 사용 가능한 Pin과 전기적 조건을 확인하고, User Manual로 Register의 동작을 확인합니다. 특정 Board의 LED 연결과 극성은 Board Manual에서 찾습니다. Family User Manual은 여러 장치를 포함하므로 U1의 TC38x 정보와 함께 읽어야 합니다.

## MCU와 Pin 근거

- **D0 Table 1, p3:** 이번 CPU Clock 비교의 기준 Part는 `SAK-TC387QP-160F300S` AE입니다. Package는 `PG-LFBGA-292`, 제품 최대 주파수는 300 MHz입니다. D0 Table 2 p6에는 F400S 제품도 있으므로 `TC387QP`라는 이름만으로 최대 주파수를 단정하지 않습니다.
- **D1 §2.2, Table 2-25, p160:** BGA292의 P00.0은 Ball G1입니다. `General-purpose input`, `FAST / PU1 / VEXT / ES` 항목을 읽어 기능과 전원 domain을 확인합니다.
- **D1 Figure 2-2, p13:** BGA292의 P13.0 위치는 B12입니다. BGA516의 Pin 표와 혼용하지 않습니다.
- **D1 §3.4, p291부터 / §3.5, p294부터:** 실제 공급 전압·Pad type·입력 mode에 해당하는 표에서 VIL/VIH와 VOL/VOH의 조건을 찾습니다. Min/Max뿐 아니라 전류·온도·Note/Test Condition도 함께 기록합니다. §3.2 p287의 Absolute Maximum Ratings는 정상 동작 조건과 구별합니다.

## Board 연결 근거

참조 Board는 `KIT_A2G_TC387_5V_TFT`입니다.

- **B1 §3.15, p3-6:** D107은 P13.0이며 LED는 Active LOW입니다. 같은 페이지 §3.16에서 S101은 RESET, S102는 PMIC TLF35584 WAKE임을 확인합니다.
- **B1 Figure 6-1, p6-1:** X103 Pin 22=P00.0, Pin 3/4=GND입니다. Table 5-6으로 Board 내 주변장치의 Pin 사용도 확인합니다.
- ENABLE용 P00.0 Pull-up과 외부 GND 접점은 과제가 선택한 구성입니다. Board의 RESET/WAKE Button이 이 입력에 연결되었다고 가정하지 않습니다.

TC387QP가 같아도 Board의 LED Pin·극성·Connector는 달라질 수 있습니다. 실제 배선 전에는 Board Revision과 회로도, 추가 연결 장치를 확인하세요.

## Register와 Clock에서 찾을 위치

| 확인할 대상 | 문서 위치 | 읽어야 할 항목 |
| --- | --- | --- |
| GPIO의 MMIO 주소 | U1 Table 3, p2-11 | P00 `0xF003A000..0xF003A0FF`, P13 `0xF003AD00..0xF003ADFF` |
| 입력·출력 mode | U3 §14.4.2, p14-8 | IOCR0 offset `0x10`, PCx의 Input/Output·Pull-up·Push-pull 선택 |
| 다른 Pin을 보존하는 출력 변경 | U3 §14.4.8, p14-21 | OMR offset `0x04`, Pin별 Set/Clear/Toggle 의미 |
| 입력 상태를 읽는 Register | U3 §14.4.14, p14-31 | Port Input Register의 역할 |
| STM Clock 경로 | U3 §10.5.1.3, p10-29 | CCUCON0.STMDIV, 허용 설정의 `fSTM = fsource0 / STMDIV`; 0은 Clock 정지 |
| 분주 설정의 제한 | U3 §10.5.1.2, Table 280, p10-28 | Clock 사이 비율과 Datasheet의 주파수 제한을 함께 만족하는지 확인 |
| STM0 주소 | U1 §25.2, Table 273, p25-1 | STM0 base `0xF0001000` |
| 읽는 Counter의 폭 | U4 §27.4.2, p27-9 | TIM0 offset `0x10`, read-only, 64-bit STM의 하위 32-bit |

STM 전체는 64-bit Free-running Counter입니다. 이번 과제처럼 TIM0를 읽으면 32-bit 값으로 관찰하므로, Counter 전체의 폭과 C에서 읽는 값의 폭을 구별해야 합니다. 주소는 Module base와 Register offset을 결합해 확인하며 다른 MCU의 주소를 재사용하지 않습니다.

과제의 **100 MHz는 제공된 Test 조건**입니다. CPU 최대 Clock, Reset 기본값 또는 Board 실측값을 뜻하지 않습니다. **200 MHz Case는 Host의 가상 산술 입력**이며 TC387QP의 허용 STM 설정을 뜻하지 않습니다. 실제 STM 주파수와 허용 범위는 해당 Part의 Datasheet, Clock 초기화 코드, 조회 API를 함께 확인합니다.

## 공개 예제와 iLLD 기준

공식 `Infineon/AURIX_code_examples`의 Commit **6759d2845feaac0107b785fe6bc57ed785c3f4fc**를 기준으로 합니다. 예제 경로는 `code_examples/Blinky_LED_1_KIT_TC387_TFT`입니다.

| ID | Source | 추적할 항목 |
| --- | --- | --- |
| S1 | [Blinky_LED.c](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/Blinky_LED_1_KIT_TC387_TFT/Blinky_LED.c) | Port·Pin·Push-pull·초기 출력, 대기 방식 |
| S2 | [IfxStm.h](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/Blinky_LED_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Stm/Std/IfxStm.h#L595) | `IfxStm_getLower()`의 TIM0 접근과 주파수 조회 |
| S3 | [IfxPort.h](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/Blinky_LED_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Port/Std/IfxPort.h#L640) | Port mode·입력·출력 API와 Register 연결 |
| S4 | [IfxScuCcu.h](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/Blinky_LED_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Scu/Std/IfxScuCcu.h#L1748) | STM Clock의 source와 STMDIV 경로 |

[Cpu0_Main.c](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/Blinky_LED_1_KIT_TC387_TFT/Cpu0_Main.c)의 metadata에는 Board와 `TC38xQP_A-Step`이 표시됩니다. 같은 예제 README에는 `TC38xTP_A-Step`이 있어 불일치합니다. MCU 식별은 D0·D1과 Source metadata로 교차 확인하세요. S1의 대기 방식이 이번 Non-blocking 요구에도 적합한지는 별도로 판단해야 합니다.

## 근거를 설계와 Test로 연결하기

| 찾아야 할 근거 | 내려야 할 설계 판단 | 확인할 Test·기록 |
| --- | --- | --- |
| D0·D1의 Part/Package/Pin 기능 | 사용할 MCU와 입력·출력 Pin이 맞는가? | 선택한 표·행과 C의 Pin 정의를 대응시킨 기록 |
| D1의 전원 domain·입출력 조건, B1 배선 | HIGH/LOW의 전기적 조건과 논리적 의미가 맞는가? | ENABLE LOW/HIGH와 LED 상태 Case; 전압은 별도 Board 검증 |
| U3·S4의 STM Clock 경로 | CPU Clock과 Timer Clock을 구별했는가? | 서로 다른 `stm_hz` 입력으로 Tick 계산 확인 |
| U4·S2의 TIM0 폭 | 읽는 값의 범위와 경계 통과를 고려했는가? | 경계 Tick·Wraparound Case |
| U3·S3의 Pin별 Register 기능 | 대상 Pin 외의 상태도 바뀌는가? | 다른 GPIO Bit를 미리 설정한 뒤 보존 여부 확인 |
| 과제의 수집·대기·늦은 호출 요구 | 호출이 늦거나 입력이 바뀌면 어떤 동작을 해야 하는가? | 시각·입력·수집 횟수·LED 상태를 적은 시간표 |

마지막 행의 동작 정책이나 C 산술 범위는 Datasheet에서 정답을 찾는 항목이 아닙니다. 요구사항과 C 언어 규칙을 근거로 정하고, Hardware 조건과 연결해 검증하세요. PR에는 **문서 ID·절/표·읽은 사실·설계 판단·Test 결과**를 함께 적습니다.

## Host 계약과 실제 API 연결

| 목적 | 실제 iLLD에서 확인할 호출 |
| --- | --- |
| LED OFF 준비 | `IfxPort_setPinHigh(&MODULE_P13, 0)` |
| LED 출력 설정 | `IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general)` |
| ENABLE 입력 설정 | `IfxPort_setPinModeInput(&MODULE_P00, 0, IfxPort_InputMode_pullUp)` |
| ENABLE 상태 | `IfxPort_getPinState(&MODULE_P00, 0)`의 LOW 판정 |
| 현재 Tick | `IfxStm_getLower(&MODULE_STM0)` |
| 실제 STM Clock 확인 | `IfxStm_getFrequency(&MODULE_STM0)` |

`platform.h`의 논리 Port 번호는 실제 Port 번호나 MMIO 주소가 아닙니다. Host 대역은 호출 순서와 Bit 보존을 검사하며, 전압·실제 시간·Board 배선을 검증하지 않습니다. Hardware Adapter를 작성할 때는 iLLD 주파수 반환형과 과제의 정수 Hz API 사이 단위·표현 가능 범위도 확인해야 합니다.

실제 Board에서는 GPIO·Clock·Startup·Watchdog·Core 설정이 포함된 정상 Project에 Adapter를 연결해야 합니다. 이 자료는 완성된 Flash Image나 Hardware 동작 확인서가 아닙니다. STM의 `volatile` Register 접근은 iLLD와 장치 Header에서 확인합니다. Host 구조체에 `volatile`을 붙이는 것만으로 Polling 주기나 Hardware 접근이 해결되지는 않으며, Single-core 순차 실행인 이번 과제에 ISR 경쟁 상태를 필수 오류로 추가하지 않습니다.
