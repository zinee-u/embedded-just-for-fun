# 08회차 실행 환경과 검증 기록

- **Host:** 이번 C Code를 실행한 PC. **Target:** 실제 적용 대상으로 가정한 TC387QP Board.
- 작성자 / GitHub ID:
- OS / Version:
- C Compiler / Version (`cc --version`):
- GNU Make / Version (`make --version`):
- 시작 자료: ZIP File 이름 또는 GitHub Commit (`git rev-parse --short HEAD`):
- 실행 위치: `Makefile`이 있는 작업 Folder:
- 실제 Board 사용 여부: `Host만 사용` / 직접 측정한 Board·장비·조건:

| 실행 대상 | 명령 | 관찰한 결과 / 종료 상태 | Log 경로 |
| --- | --- | --- | --- |
| 수정 전 Test | `make test` | | `logs/before-test.log` |
| 수정 전 Demo | `make demo` | | `logs/before-demo.log` |
| 수정 후 Test | `make test` | | `logs/after-test.log` |
| 수정 후 Demo | `make demo` | | `logs/after-demo.log` |

- 명령 바로 다음 줄의 `echo $?`: 해당 명령의 Exit code(종료 상태) 확인. Log 저장 방법은 `SUBMISSION.md` 참고.
- C Compiler 오류와 Test 실패를 구분: 앞의 경우 실행 File 생성 실패, 뒤의 경우 실행한 결과가 요구 조건과 다름.
- 제출 자료: 재현에 필요한 C File·Test·작성 기록·실행 Log. `build/` 안의 실행 File, PDF, ZIP 제외.

| 추가 Test 이름 | 확인할 가설 / 경계 | 입력과 상태 | 실행 전 예상 | 실제 결과 |
| --- | --- | --- | --- | --- |
| | | | | |
| | | | | |

- Pin 근거 — MCU Package, Datasheet의 SDA·SCL 출력 행과 Ctrl. 열, 두 iLLD 항목의 출력 선택:
- 출력 방식 근거 — iLLD의 Mode와 Pull-up이 연결된 회로에서 High를 만드는 방법:
- Demo에서 읽은 설정 두 줄 — 수정 전 / Pin 번호만 수정 / 출력 기능과 출력 방식까지 수정 / 전체 수정:
- 배선 출처 — Host 모의 배선 / 실제 PCB 회로도 이름·Revision·Connector / 실측 여부:
- Host에서 확인한 범위:
- 실제 Board에서 확인하지 않은 항목:
