# [08회차] YOUR_GITHUB_ID TC387QP Sensor Register 읽기 개선

## 개선 결과

- 재현한 대표 현상:
- 같은 입력의 수정 전 / 수정 후 결과:
- `P01` 등 문제 ID별 상세 근거: 내 제출 Folder의 `PROBLEM_DEFINITION.md`.

| 문제 ID | 현상: 기대 / 실제 | 원인: 근거·Code 위치 | 해결방안: 변경 규칙·이유 | 검증: Test·수정 후 실제값 |
| --- | --- | --- | --- | --- |
| P01 | | | | |

## Bus 선택과 읽기 근거

- 지정 배선 — I2C0 SDA=P02.4, SCL=P02.5. 이를 적용한 `src/sensor.c`의 변경 위치:
- 근거 — 두 Pin 각각의 Table 2-27 신호명·Ctrl. 열 → iLLD 출력 선택 → `src/sensor.c`의 설정값:
- 출력 방식 — Open-drain 선택 근거와 Pull-up의 역할:
- 원본 → Pin 번호 수정 → 출력 기능 수정 → 출력 방식 수정 → 전체 수정. 각 단계의 I2C 설정 결과·UART 줄 수 또는 값:
- 대조 검증 — Pin 역할 역전 / 모듈 불일치 / 한 Pin의 출력 기능 불일치 / 한 Pin의 Push-pull. 각 조건의 Host 결과와 Log 위치:
- I2C와 SPI의 차이, 이 Board 배선에서 선택한 방식과 이유:
- 판단에 사용한 공식 자료의 이름·절·Register:
- LIS3DH 초기화부터 X축 값 수신·UART 기록까지의 순서: 내 제출 Folder의 `BUS_ANALYSIS.md`.
- 실제 Board에서 확인한 사항 / Host에서만 확인한 사항:

## 실행 결과

| 항목 | 입력·예상 | 실제 결과·Exit code | 근거 |
| --- | --- | --- | --- |
| 수정 전 고정 Test / Demo | | | `logs/before-test.log`, `logs/before-demo.log` |
| 수정 후 고정 Test / Demo | | | `logs/after-test.log`, `logs/after-demo.log` |
| 추가 Test 1 | | | 함수명: |
| 추가 Test 2 | | | 함수명: |

## 제출 확인

- [ ] 모든 독립된 근본 원인과 수정 근거 작성. 같은 원인의 여러 실패는 하나로 묶고 연결한 Test 표시.
- [ ] `board_init_sensor_bus()`에서 설계한 값을 Host 실행 경로에 적용. 각 Pin의 출력 기능과 출력 방식을 따로 확인한 실행 결과 첨부.
- [ ] I2C/SPI 비교표·읽기 순서도·UART 출력 추적 작성.
- [ ] 실행 환경과 미측정 항목 구분. 실제 측정하지 않은 값은 Host 모의 결과로 표시.
- [ ] `submissions/week08/YOUR_GITHUB_ID/`만 변경. Starter 원본·고정 Test 보존. PDF·ZIP·Build 산출물 제외.
