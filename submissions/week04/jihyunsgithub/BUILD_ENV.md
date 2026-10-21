# 4주차 Build 환경 기록

빈 칸은 본인이 확인한 값으로 채웁니다. Source에서 읽은 사실과 실제 실행 결과를 구분합니다.

| 항목 | 확인한 내용 |
| --- | --- |
| GitHub 아이디 / 작업 Branch | jihyunsgithub / solve/week04-jihyunsgithub|
| 과제 기준 upstream Commit | 0df740936cba914b359a5706c6dc2ac6bffc5f0c (로컬 upstream/main 기준)|
| OS / CPU | macOS 14.5 (23F79) / Apple M1 (uname -m: x86_64)|
| C Compiler 이름 / version | GNU GCC 16.2.0 (gcc-16) |
| GNU Make version | GNU Make 3.81|
| 실제 CC / CFLAGS / ASFLAGS | CC: gcc-16; CFLAGS: -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g; ASFLAGS: -g|
| Host test 실행 위치 / 명령 / Exit code | submissions/week04/jihyunsgithub / make CC=gcc-16 test / Exit code: 0|
| 단계별 Build 실행 위치 / 명령 / Exit code | submissions/week04/jihyunsgithub / make CC=gcc-16 stages / Exit code: 0|
| 실행 File 형식 | Mach-O 64-bit executable arm64|
| 실행 Log | logs/test.log, logs/stages.log |

## 공개 STM32 예제의 기준

- MCU / Board: STM32F030R8 / NUCLEO-F030R8
- Datasheet 번호 / Revision: DS9773 / Rev 5
- Source URL / Version / Commit: https://github.com/STMicroelectronics/STM32CubeF0 / v1.11.5 / e220bfb12a162cfbf3bb65663e987e0d6550e9b4
- 분석한 Project 경로:Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle
- Target / Include / Define / Linker 설정: Target STM32F030R8Tx (Thumb, Soft Float); Include Inc, CMSIS/Device/ST/STM32F0xx/Include, STM32F0xx_HAL_Driver/Inc, BSP/STM32F0xx-Nucleo, CMSIS/Include; Define STM32F030x8, USE_HAL_DRIVER, USE_STM32F0XX_NUCLEO; Linker STM32F030R8Tx_FLASH.ld (-specs=nosys.specs -specs=nano.specs)
- HAL / CMSIS / BSP 의존성: STM32F0xx HAL Driver / CMSIS Cortex-M0 및 STM32F030x8 디바이스 지원 파일 / STM32F0xx Nucleo BSP
- 실제 ARM Compiler / IDE: 미설치
- 실제 ARM Build / Board 실행: 미실행

## 동료의 재현 절차

1. 제출 PR의 Branch 또는 Commit:solve/week04-jihyunsgithub (GitHub: jihyunsgithub, 최종 제출 Commit은 PR 생성 후 확인)
2. 진입할 제출 폴더:submissions/week04/jihyunsgithub
3. 필요한 도구: Git, GNU Make 3.81, GNU GCC 16.2.0 (gcc-16). 검증 환경은 macOS 14.5 / Apple M1.
4. Clean Build와 Test 명령: make clean / make CC=gcc-16 test / make CC=gcc-16 stages
5. 기대 결과와 확인할 Log:make test 실행 시 Public checks: 319, failures: 0 및 RESULT: PASS 확인. make stages 실행 시 .i, .s, .o와 build/mission-tests 생성 확인. 두 명령의 Exit code는 모두 0. 결과 로그는 logs/test.log, logs/stages.log에서 확인.

Host test 통과는 실제 MCU에서 Clock 주파수·LED 주기를 측정한 결과가 아닙니다. Binary는 제출하지 않고 다시 만드는 데 필요한 Source와 설정·명령을 기록합니다.
