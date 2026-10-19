# 05회차 과제를 Pull Request로 제출하기

명령은 Git Bash 또는 macOS/Linux Terminal 기준입니다. `YOUR_GITHUB_ID`를 본인 아이디로 바꿉니다. 원본 저장소는 https://github.com/zinee-u/embedded-just-for-fun 입니다.

05회차는 예외적으로 [한국어 해설 코드](reference/README.md)를 공개합니다. 해설을 참고하며 공부한 뒤, **Starter를 자기 제출 폴더로 복사해 개선하고 기존 PR 방식으로 제출**합니다. 공개 해설 자체를 수정하거나 다른 회차의 비공개 해설을 제출에 포함하지 않습니다.

## 1 Fork와 Clone

GitHub에서 원본 저장소의 Fork를 만든 뒤 Clone합니다. 이미 준비했다면 기존 Clone에서 다음 단계로 진행합니다.

```sh
git clone https://github.com/YOUR_GITHUB_ID/embedded-just-for-fun.git
cd embedded-just-for-fun
git remote add upstream https://github.com/zinee-u/embedded-just-for-fun.git
git remote -v
```

이미 `upstream`이 있으면 추가 명령을 반복하지 말고 URL을 확인합니다. `origin`은 자기 Fork입니다. 본인의 Commit 이름과 이메일을 설정하세요.

```sh
git config --local user.name "YOUR_GITHUB_ID"
git config --local user.email "YOUR_COMMIT_EMAIL"
```

## 2 새 Branch와 자기 제출 폴더

이전 작업은 먼저 Commit하거나 별도로 보관하고 현재 상태를 확인합니다. 아래는 새로 시작할 때의 절차입니다. 기존 풀이가 있다면 복사 명령으로 덮어쓰지 않습니다.

```sh
git status --short
git fetch upstream
git switch -c solve/week05-YOUR_GITHUB_ID upstream/main
mkdir -p submissions/week05/YOUR_GITHUB_ID
cp -R weeks/week05/starter/. submissions/week05/YOUR_GITHUB_ID/
cd submissions/week05/YOUR_GITHUB_ID
```

복사 대상은 개선 전 `starter/`입니다. `reference/`에는 읽고 실행해 볼 수 있는 해설 구현이 있으며, 제출 폴더를 대신하지 않습니다. 폴더 안에서 `git init`을 다시 실행하지 않습니다.

## 3 코드와 Test 작성

C11 Compiler와 GNU Make를 준비합니다. Windows는 GCC·Make가 설치된 MSYS2 UCRT64 같은 환경을 사용할 수 있습니다. Git Bash에 Compiler가 기본 포함되어 있다고 가정하지 않습니다.

```sh
cc --version
make --version
```

[공개 해설](reference/README.md)을 읽으며 다음 순서로 진행합니다.

1. 개선 전 코드의 실패 입력이나 호출 순서를 고르고, 예상 결과와 실제 결과를 비교합니다.
2. 해설 주석·구현과 [공식 자료](REFERENCES.md)를 대조해 실패의 근본 원인과 수정 이유를 정리합니다. Hardware가 정한 사실과 과제가 선택한 정책을 구분합니다.
3. 자기 제출 폴더의 세 `src/*.c`를 수정합니다. 작성 중에도 해설을 참고할 수 있습니다.
4. 직접 선택한 경계 또는 실패 조건을 `tests/test_student.c`에 최소 2개 추가합니다. 해설의 Test를 그대로 옮기는 데 그치지 말고, 선택한 입력과 예상 결과의 이유를 설명합니다.
5. `README.md`, `TIMING.md`, `BUILD_ENV.md`에 모든 근본 원인, 수정 이유, 근거와 검증 범위를 기록합니다.

제공 검증 기준은 바꾸지 않습니다. 실제 Compiler가 GCC이면 `make CC=gcc test`처럼 지정합니다. 해설과 코드 모양이 비슷한지는 완료 기준이 아니며, 자신의 입력·설명·실행 결과로 동작을 이해했는지 확인합니다.

## 4 실제 실행 Log 저장

자기 제출 폴더에서 실행합니다. `make test`가 실패하면 실패 원인을 수정하고 최종 코드로 다시 기록합니다.

```sh
mkdir -p logs
make clean
make test > logs/test.log 2>&1
test_status=$?
printf '\nmake test exit code: %s\n' "$test_status" >> logs/test.log
cat logs/test.log
```

Demo를 먼저 Build한 뒤 CSV만 별도로 저장합니다. 첫 번째 명령이 성공한 경우에만 두 번째 명령을 실행하세요.

```sh
make demo
make -s demo > logs/demo.csv
```

두 명령 모두 Exit code가 0이어야 합니다. `-s`는 Make의 명령 표시만 숨기며 Test 실패를 숨기는 옵션이 아닙니다. Demo의 시각은 주입된 가상 시각입니다. MCU 실측으로 표시하지 않습니다. 실제 사용한 Compiler 옵션과 명령을 `BUILD_ENV.md`에 남깁니다.

해설 폴더에서 실행한 Log를 제출 코드의 결과로 복사하지 않습니다. 자신의 최종 코드와 추가 Test로 생성한 Log를 제출합니다.

## 5 목적이 다른 Commit 남기기

```sh
cd ../../..
git add submissions/week05/YOUR_GITHUB_ID/src \
  submissions/week05/YOUR_GITHUB_ID/include \
  submissions/week05/YOUR_GITHUB_ID/host \
  submissions/week05/YOUR_GITHUB_ID/tests \
  submissions/week05/YOUR_GITHUB_ID/Makefile \
  submissions/week05/YOUR_GITHUB_ID/.gitignore
git diff --cached
git commit -m "Implement week05 TC387QP polling collector"
git add submissions/week05/YOUR_GITHUB_ID
git add -f submissions/week05/YOUR_GITHUB_ID/logs/test.log
git diff --cached
git commit -m "Verify week05 timing and document root causes"
git log --oneline -2
git push -u origin solve/week05-YOUR_GITHUB_ID
```

`build/`와 SDK 전체, 인증 파일은 Commit하지 않습니다. Log와 자기 코드·설명만 포함되어 있는지 확인합니다. 개수만 맞춘 빈 Commit 대신 실제 목적이 다른 변경을 남깁니다.

Starter의 `.gitignore`는 `*.log`를 제외하므로, 위 명령은 제출에 필요한 `logs/test.log` 한 파일만 명시적으로 추가합니다. `logs/demo.csv`는 일반 `git add`에 포함됩니다.

## 6 원본 저장소에 PR 생성

- Base repository: `zinee-u/embedded-just-for-fun`
- Base branch: `main`
- Head repository: 자신의 Fork
- Compare branch: `solve/week05-YOUR_GITHUB_ID`
- 제목: `[05회차] YOUR_GITHUB_ID TC387QP 주기 수집 개선`

[PR 본문](PR_BODY.md)을 실제 결과로 채우고 Files changed에서 자기 제출 폴더만 바뀌었는지 확인합니다. 자기 Fork 내부 PR만 만들면 제출이 완료되지 않습니다. Review 수정은 같은 Branch에 Commit하고 Push합니다.
