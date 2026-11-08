# 08회차 과제 실행·제출 안내

- **ZIP으로 연습** — ZIP을 풀고 A 경로 사용. GitHub 계정과 실제 Board 없이 시작 가능.
- **GitHub로 제출** — B 경로 사용. 개선 전 Code는 `starter/`, 자신의 풀이는 `submissions/week08/YOUR_GITHUB_ID/`에 보관.
- **준비:** C11 Compiler(Code를 실행 File로 바꾸는 도구), GNU Make(`Makefile`의 명령을 실행하는 도구), Terminal(명령을 입력하는 창). 아래 명령은 macOS·Linux·Windows WSL/MSYS2의 Bash 기준.

## A. ZIP으로 시작

1. ZIP 압축 해제. `README.md`, `SUBMISSION.md`, `starter/`가 함께 보이는 `week08/` Folder에서 Terminal 열기.
2. `pwd`로 현재 위치, `ls starter/Makefile`로 시작 File 확인.
3. 최초 1회만 아래 명령으로 작업용 사본 생성. 이미 `work/`가 있으면 복사를 건너뛰고 `cd work`만 실행.

```sh
cp -R starter work
cd work
```

4. **아래 1~3단계**를 `work/`에서 실행. PR로 제출할 때 B 경로의 개인 제출 Folder로 자신의 변경·기록을 옮기고 재실행.

## B. GitHub에서 개인 제출 Folder 준비

1. [원본 저장소](https://github.com/zinee-u/embedded-just-for-fun) 열기. 저장소에서의 과제 경로는 `weeks/week08/`.
2. GitHub의 **Fork**로 원본 저장소의 개인 사본 생성. 아래 모든 `YOUR_GITHUB_ID`를 본인 GitHub ID로 교체.
3. **Clone**으로 개인 사본을 PC에 복사. 기존 Clone이 있으면 해당 Folder에서 `git remote -v`로 주소만 확인.

```sh
git clone https://github.com/YOUR_GITHUB_ID/embedded-just-for-fun.git
cd embedded-just-for-fun
git remote add upstream https://github.com/zinee-u/embedded-just-for-fun.git
git remote -v
```

- `origin`: 내 Fork 주소. `upstream`: 원본 저장소 주소. 기존 `upstream`에 `remote add` 반복 금지.
- `git status --short`로 기존 작업 확인. 다른 작업이 있으면 먼저 별도 보관.
- **Branch**: 개인 풀이를 기록할 작업 줄기. 아래 명령은 새 제출을 처음 시작할 때만 실행.

```sh
git fetch upstream
git switch -c solve/week08-YOUR_GITHUB_ID upstream/main
mkdir -p submissions/week08/YOUR_GITHUB_ID
cp -R weeks/week08/starter/. submissions/week08/YOUR_GITHUB_ID/
cd submissions/week08/YOUR_GITHUB_ID
```

- ZIP에서 먼저 풀었다면: 저장소의 Starter와 ZIP의 시작 Version 비교 → 아래 1단계의 원본 결과 확보 → 수정한 C File·추가 Test·작성한 기록을 개인 제출 Folder에 옮기기. `weeks/week08/starter/`에 개인 풀이를 덮어쓰지 않도록 주의.

## 1. 수정 전 실행

- 작업 위치: A는 `work/`, B는 `submissions/week08/YOUR_GITHUB_ID/`. **`ls Makefile`이 성공하는 위치**에서 실행.
- `make test`: 요구 조건과 Code 결과를 비교하는 Test 실행. 개선 전에는 의도된 실패 발생.
- `make demo`: 대표 입력의 출력 확인. `Exit code`는 명령 종료 상태. `0`은 명령이 성공적으로 끝났다는 뜻이며 값의 정확성은 별도 비교.

```sh
cc --version
make --version
mkdir -p logs
make clean
make test > logs/before-test.log 2>&1
printf '\nmake test exit: %s\n' "$?" >> logs/before-test.log
make demo > logs/before-demo.log 2>&1
printf '\nmake demo exit: %s\n' "$?" >> logs/before-demo.log
cat logs/before-test.log
cat logs/before-demo.log
```

- `> File 이름`: 출력을 File에 저장. 기존 내용을 덮어쓰므로 `before-*.log`는 원본 실행 때만 사용.
- `2>&1`: 오류 문구도 같은 File에 저장. `$?`: 직전 명령의 종료 상태.

## 2. 현상 정의 → 근거 확인 → Code 수정

- `PROBLEM_DEFINITION.md`: 기대값과 실제값 → 원인 가설 → 가설을 가를 Test → 확정 원인 → 해결방안 작성. 서로 다른 근본 원인마다 반복.
- `BUS_ANALYSIS.md`: I2C/SPI 비교표·통신 선택 이유·Sensor 읽기 순서도·UART까지의 값 추적 작성. 근거는 `REFERENCES.md`에 연결된 공식 자료의 절·표·Register 이름으로 표시.
- `src/sensor.c` — `board_init_sensor_bus()`에서 I2C0의 SDA=P02.4, SCL=P02.5 적용. Datasheet Table 2-27에서 두 신호의 출력 기능을 각각 찾고, C에 각 Pin의 출력 선택과 Open-drain 지정. Pin 설정 성공 후 Sensor 식별·초기화·읽기 수행. 제공 Host 대역·Header·고정 Test(`tests/test_sensor.c`)·기대값 유지.
- `tests/test_student.c`: 서로 다른 경계 Test 최소 2개 추가. Test마다 수정 전 예상값·확인할 가설을 먼저 기록.
- Pin 번호를 수정한 시점, 출력 기능을 수정한 시점, 출력 방식까지 수정한 시점마다 `make demo` 실행. 설정 두 줄·Sensor 접근 여부·UART 출력을 `BUS_ANALYSIS.md`에 복사. 같은 입력으로 전체 수정 뒤 다시 비교.
- `make test`에서 Pin 역할·I2C 모듈·출력 기능·출력 방식을 각각 잘못 설정한 결과 확인. 한 항목만 바꿔 비교한 입력과 출력을 `BUS_ANALYSIS.md`에 기록. 실제 Board에서 확인했다면 회로도와 Connector 번호도 기재.
- `BUILD_ENV.md`: 실제 실행 환경 기록. `README.md`: 자신의 변경과 결과 요약.

## 3. 같은 입력으로 재실행

```sh
make clean
make test > logs/after-test.log 2>&1
printf '\nmake test exit: %s\n' "$?" >> logs/after-test.log
make demo > logs/after-demo.log 2>&1
printf '\nmake demo exit: %s\n' "$?" >> logs/after-demo.log
cat logs/after-test.log
cat logs/after-demo.log
```

- `after-test.log`: 고정 Test·추가 Test 결과와 종료 상태 확인.
- `after-demo.log`: 같은 입력의 동작·UART 출력 비교. 실제 Board에서 측정하지 않았으면 **Host 모의 결과**로 표시.

## 4. GitHub에 PR 제출

- B의 **개인 제출 Folder**에서 시작. 아래 `cd ../../..`는 해당 위치에서만 사용.
- **Commit**: 내 변경을 기록. **Push**: Commit을 내 Fork에 전송. **Pull Request(PR)**: 해당 변경을 원본 저장소에 제출.

```sh
cd ../../..
git status --short
git add -- submissions/week08/YOUR_GITHUB_ID
git diff --cached --stat
git diff --cached
git commit -m "Improve week08 sensor register read and verify UART output"
git push -u origin solve/week08-YOUR_GITHUB_ID
```

- GitHub에서 **Compare & pull request** 선택. Base: `zinee-u/embedded-just-for-fun:main`. Head: `YOUR_GITHUB_ID:solve/week08-YOUR_GITHUB_ID`.
- [PR 본문 양식](PR_BODY.md)에 **자신이 실행한 결과** 작성. `Files changed`에는 자기 제출 Folder만 포함. PDF·ZIP·`build/` 제외.
