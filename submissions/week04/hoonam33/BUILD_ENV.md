# 4주차 Build 환경 기록

빈 칸은 본인이 확인한 값으로 채웁니다. Source에서 읽은 사실과 실제 실행 결과를 구분합니다.

| 항목 | 확인한 내용 |
| --- | --- |
| GitHub 아이디 / 작업 Branch |hoonam33, solve/week04-hoonam33 |
| 과제 기준 upstream Commit |a40695f53d7d78a3f239c6e5baced853c151e3a6 |
| OS / CPU |Windows 11 Home 64-bit (빌드 26200) / Intel Core Ultra 5 228V (8코어) |
| C Compiler 이름 / version |GCC (MinGW-W64, x86_64-win32-seh-rev0)/8.1.0 |
| GNU Make version |GNU Make 4.2.1 (x86_64-w64-mingw32) |
| 실제 CC / CFLAGS / ASFLAGS |CC=`gcc`; CFLAGS=`-std=c11 -Wall -Wextra -Werror -pedantic -O0 -g`; ASFLAGS=`-g` |
| Host test 실행 위치 / 명령 / Exit code |`submissions/week04/hoonam33/` / `make SHELL=C:/Progra~1/Git/usr/bin/sh.exe CC=gcc EXEEXT=.exe test` / `0` |
| 단계별 Build 실행 위치 / 명령 / Exit code |`submissions/week04/hoonam33/` / `make -B SHELL=C:/Progra~1/Git/usr/bin/sh.exe CC=gcc EXEEXT=.exe stages` / `0` |
| 실행 File 형식 |Windows PE32+ (x86-64) |
| 실행 Log | logs/test.log, logs/stages.log |

## 공개 STM32 예제의 기준

- MCU / Board:STM32F030R8 / NUCLEO-F030R8
- Datasheet 번호 / Revision:DS9773 Rev 5, November 2021
- Source URL / Version / Commit:`https://github.com/STMicroelectronics/STM32CubeF0/tree/e220bfb12a162cfbf3bb65663e987e0d6550e9b4`/v1.11.5 / e220bfb12a162cfbf3bb65663e987e0d6550e9b4
- 분석한 Project 경로:Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/
- Target / Include / Define / Linker 설정:`STM32F030R8Tx, Board NUCLEO-F030R8`/`../../../Inc
../../../../../../../../Drivers/CMSIS/Device/ST/STM32F0xx/Include
../../../../../../../../Drivers/STM32F0xx_HAL_Driver/Inc
../../../../../../../../Drivers/BSP/STM32F0xx-Nucleo
../../../../../../../../Drivers/CMSIS/Include`/`STM32F030x8, USE_HAL_DRIVER, USE_STM32F0XX_NUCLEO`/`../STM32F030R8Tx_FLASH.ld`
- HAL / CMSIS / BSP 의존성:`RCC·GPIO 등의 하드웨어 설정·제어 함수를 제공하는 STM32F0xx_HAL_Driver`/`Cortex-M 코어와 STM32F0 장치 정의·시스템 초기화에 필요한 헤더와 코드.`/`Nucleo 보드의 LED·버튼 등 보드별 연결과 제어를 제공하는 STM32F0xx-Nucleo`
- 실제 ARM Compiler / IDE: '미설치'
- 실제 ARM Build / Board 실행: '미실행'

## 동료의 재현 절차

1. 제출 PR의 Branch 또는 Commit:solve/week04-hoonam33
2. 진입할 제출 폴더:submissions/week04/hoonam33/
3. 필요한 도구:GCC 8.1.0, GNU Make 4.2.1
4. Clean Build와 Test 명령: 
```
$env:Path = 'C:\Program Files\Git\usr\bin;' + $env:Path
$env:CPPFLAGS = '-D__USE_MINGW_ANSI_STDIO=1'

make SHELL=C:/Progra~1/Git/usr/bin/sh.exe CC=gcc EXEEXT=.exe clean
make SHELL=C:/Progra~1/Git/usr/bin/sh.exe CC=gcc EXEEXT=.exe test
make -B SHELL=C:/Progra~1/Git/usr/bin/sh.exe CC=gcc EXEEXT=.exe stages

```
5. 기대 결과와 확인할 Log:`공개 검사 319, 실패 0, 학생 assert 통과, 두 빌드 명령 종료 코드 0. logs/test.log, logs/stages.log를 확인하며, 빌드 후 .i·.s·.o와 build/mission-tests.exe가 생성되어야 합니다.`

Host test 통과는 실제 MCU에서 Clock 주파수·LED 주기를 측정한 결과가 아닙니다. Binary는 제출하지 않고 다시 만드는 데 필요한 Source와 설정·명령을 기록합니다.
