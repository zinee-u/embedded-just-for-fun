# 3주차 탐사 장치 레지스터 제어

GitHub 아이디: LSA31

## 요구 조건과 내가 정한 계약

이번 과제에서는 8비트 CTRL 및 STATUS 값을 실제 하드웨어 주소에 접근하지 않고 함수 인자로 전달받아 계산합니다.

CTRL 레지스터의 비트 구성은 다음과 같습니다.

- bit0: ENABLE
  - 0: 꺼짐
  - 1: 켜짐
- bits2:1: MODE
  - `00`: 대기
  - `01`: 저속
  - `10`: 탐사
  - `11`: 사용 금지
- bits7:3: 다른 설정이며 변경하지 않고 보존해야 합니다.

STATUS에서는 bit3이 READY입니다.

제가 구현한 함수의 계약은 다음과 같습니다.

```c
uint8_t enable(uint8_t ctrl);
uint8_t disable(uint8_t ctrl);
uint8_t set_mode(uint8_t ctrl, uint8_t mode);
uint8_t is_ready(uint8_t status);
```

- `enable`은 CTRL의 bit0만 항상 1로 만들고 다른 비트는 보존합니다.
- `disable`은 CTRL의 bit0만 항상 0으로 만들고 다른 비트는 보존합니다.
- `set_mode`는 CTRL의 bits2:1만 새 mode 값으로 교체하고 ENABLE과 bits7:3은 보존합니다.
- 새 `mode` 인자는 과제 조건에 따라 0~2가 들어온다고 가정합니다.
- 기존 CTRL의 MODE가 `11`이어도 정상적인 수정 대상입니다.
- `is_ready`는 STATUS의 bit3만 읽어 숫자 0 또는 1을 반환합니다.

## 발견한 모든 근본 원인

| 근본 원인 | 실패 입력 또는 상황 | 발생하는 영향 | 수정 방법과 이유 |
| --- | --- | --- | --- |
| `enable`이 `0x01u`만 반환 | `enable(0xA8)` | ENABLE은 켜지지만 MODE와 bits7:3이 모두 0으로 사라짐 | 기존 `ctrl`과 `0x01`을 OR하여 bit0만 1로 설정 |
| `disable`이 `0`만 반환 | `disable(0xAD)` | ENABLE뿐 아니라 MODE와 다른 설정도 모두 0으로 사라짐 | ENABLE 마스크를 반전한 값과 AND하여 bit0만 0으로 설정 |
| `set_mode`가 OR만 사용 | 기존 MODE에 1이 남아 있는 경우 | 기존 1비트를 0으로 바꾸지 못해 새 MODE로 교체되지 않을 수 있음 | 기존 MODE bits2:1을 먼저 0으로 지우고 새 mode를 이동시켜 OR |
| `is_ready`가 STATUS 전체를 `0x08`과 비교 | `is_ready(0x09)` | READY가 1이어도 다른 상태 비트가 켜져 있으면 0 반환 | READY bit3만 마스크로 추출하고 bit0 위치로 이동 |
| 하드웨어 STATUS 선언에 한정자가 없음 | 실제 읽기 전용 하드웨어 레지스터를 가리키는 경우 | 하드웨어에 의해 값이 바뀔 수 있다는 점과 쓰기 금지 의미가 표현되지 않음 | `volatile const uint8_t *status`로 선언 |

## 내가 이렇게 답변하겠습니다

> 이 문제의 핵심은 8비트 레지스터 전체를 새 값으로 덮어쓰는 것이 아니라, 변경해야 하는 비트만 수정하고 나머지 비트는 그대로 보존하는 것입니다.
>
> 원본 `enable`과 `disable`은 ENABLE bit만 변경해야 하지만 각각 `0x01`과 `0`을 그대로 반환하기 때문에 MODE와 다른 설정 비트가 손실됩니다. 그래서 `enable`에서는 OR를 사용해 bit0만 1로 만들고, `disable`에서는 ENABLE 마스크를 반전한 값과 AND하여 bit0만 0으로 만들었습니다.
>
> `set_mode`는 기존 코드처럼 OR만 사용하면 이미 1인 MODE 비트를 0으로 만들 수 없기 때문에 필드를 교체할 수 없습니다. 따라서 MODE에 해당하는 bits2:1을 먼저 0으로 지운 뒤 새 mode 값을 한 비트 왼쪽으로 이동시켜 해당 위치에 넣었습니다.
>
> `is_ready`는 STATUS 전체가 `0x08`인지 비교하면 다른 상태 비트가 함께 켜진 경우 READY를 놓칠 수 있습니다. 그래서 READY에 해당하는 bit3만 마스크로 추출하고 오른쪽으로 3비트 이동하여 0 또는 1을 반환하도록 수정했습니다.
>
> 필수 7개 입력을 실행한 결과 예상값과 실제값이 모두 일치했고 `Total failures: 0`을 확인했습니다.

## 선택한 설계와 대안

### 사용한 마스크

```c
#define ENABLE_MASK ((uint8_t)0x01u)
#define MODE_MASK   ((uint8_t)0x06u)
#define READY_MASK  ((uint8_t)0x08u)
```

각 마스크를 2진수로 보면 다음과 같습니다.

```text
ENABLE_MASK = 0000 0001
MODE_MASK   = 0000 0110
READY_MASK  = 0000 1000
```

### ENABLE 설정

```c
return (uint8_t)(ctrl | ENABLE_MASK);
```

OR 연산에서 마스크가 1인 bit0만 1이 되고 나머지 비트는 기존 값이 유지됩니다.

예를 들어:

```text
0xA8        1010 1000
ENABLE_MASK 0000 0001
            ---------
결과        1010 1001 = 0xA9
```

### ENABLE 해제

```c
return (uint8_t)(ctrl & (uint8_t)~ENABLE_MASK);
```

`ENABLE_MASK`를 반전하면 bit0만 0이고 나머지는 1인 마스크가 됩니다.

```text
ENABLE_MASK     0000 0001
반전            1111 1110
```

AND 연산에서 bit0만 0으로 지워지고 다른 비트는 보존됩니다.

### MODE 교체

```c
ctrl = (uint8_t)(ctrl & (uint8_t)~MODE_MASK);
ctrl = (uint8_t)(ctrl | (uint8_t)(mode << 1));
```

첫 번째 연산에서 MODE bits2:1만 0으로 지웁니다.

두 번째 연산에서는 `mode`가 2비트 값이므로 한 비트 왼쪽으로 이동하여 bits2:1 위치에 넣습니다.

예를 들어 `set_mode(0xAB, 2)`는 다음과 같습니다.

```text
기존 CTRL       1010 1011
기존 MODE             01

MODE 제거       1010 1001
새 mode=2       0000 0100
                ---------
결과            1010 1101 = 0xAD
```

기존 MODE가 `11`인 `0xAF`도 먼저 MODE 필드를 지우기 때문에 새 MODE `10`으로 정상 교체할 수 있습니다.

OR만 사용하지 않은 이유는 OR는 0을 1로 만들 수 있지만 기존의 1을 0으로 만들 수 없기 때문입니다.

### READY 추출

```c
return (uint8_t)((status & READY_MASK) >> 3);
```

먼저 READY_MASK와 AND하여 bit3만 남긴 뒤, 이를 3비트 오른쪽으로 이동합니다.

READY가 켜진 경우:

```text
status       0000 1001
READY_MASK   0000 1000
             ---------
AND          0000 1000
>> 3         0000 0001
```

따라서 최종 반환값은 1입니다.

READY가 꺼져 있으면 최종 반환값은 0입니다.

## 하드웨어 STATUS 선언

실제 MCU의 읽기 전용 STATUS 레지스터를 가리키는 포인터는 다음과 같이 선언할 수 있습니다.

```c
volatile const uint8_t *status;
```

`volatile`은 포인터가 가리키는 값을 프로그램 외부의 하드웨어가 변경할 수 있다는 것을 표현합니다.

따라서 컴파일러가 해당 값을 한 번 읽은 뒤 계속 재사용한다고 가정하지 않고, 코드에서 요구하는 접근을 임의로 제거하거나 캐시된 값으로 대체하지 않도록 합니다.

`const`는 이 포인터를 통해 가리키는 STATUS 값을 수정하지 않겠다는 의미입니다. STATUS가 읽기 전용이라는 조건을 코드에 표현할 수 있습니다.

이 선언에서 `volatile`과 `const`는 포인터 변수 `status` 자체가 아니라 `status`가 가리키는 `uint8_t` 값에 적용됩니다.

```c
volatile const uint8_t *status;
```

따라서 가리키는 값은 `volatile const uint8_t`이고, 포인터 변수 자체는 다른 주소를 가리키도록 변경할 수 있습니다.

또한 `volatile`은 원자성이나 멀티스레드 동기화를 보장하지 않습니다. 하드웨어 레지스터 접근이 원자적인지, 여러 실행 주체 사이에서 동기화가 필요한지는 별도의 하드웨어 및 동시성 조건을 검토해야 합니다.

이번 과제에서는 실제 하드웨어 주소를 역참조하지 않고 선언의 의미만 설명했습니다.

## 실제 구현

```c
#include <stdint.h>

#define ENABLE_MASK ((uint8_t)0x01u)
#define MODE_MASK   ((uint8_t)0x06u)
#define READY_MASK  ((uint8_t)0x08u)

uint8_t enable(uint8_t ctrl)
{
    return (uint8_t)(ctrl | ENABLE_MASK);
}

uint8_t disable(uint8_t ctrl)
{
    return (uint8_t)(ctrl & (uint8_t)~ENABLE_MASK);
}

uint8_t set_mode(uint8_t ctrl, uint8_t mode)
{
    ctrl = (uint8_t)(ctrl & (uint8_t)~MODE_MASK);
    ctrl = (uint8_t)(ctrl | (uint8_t)(mode << 1));

    return ctrl;
}

uint8_t is_ready(uint8_t status)
{
    return (uint8_t)((status & READY_MASK) >> 3);
}
```

## 실제 실행 환경

- OS 및 CPU 또는 MCU:
  - Windows 호스트 PC
  - 실제 MCU에서는 실행하지 않음
- 터미널 환경:
  - MSYS2 UCRT64
- 컴파일러:
  - GCC 16.2.0
- 언어 버전:
  - C11
- 사용한 옵션:
  - `-Wall`
  - `-Wextra`
  - `-Wpedantic`

이번 테스트는 실제 하드웨어 레지스터에 접근한 것이 아니라 `uint8_t` 값을 이용하여 순수한 비트 연산 결과를 확인했습니다.

## 빌드와 실행 방법

저장소 루트에서 다음 명령으로 빌드했습니다.

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic \
  submissions/week03/LSA31/register_control.c \
  submissions/week03/LSA31/tests.c \
  -o week03_test.exe
```

실행 명령은 다음과 같습니다.

```sh
./week03_test.exe
```

## 검증 결과

필수 일곱 입력은 각각 독립된 입력으로 실행했습니다.

| 입력 또는 시나리오 | 예상 결과 | 실제 결과 | 통과 여부 |
| --- | --- | --- | --- |
| `enable(0xA8)` | `0xA9` | `0xA9` | PASS |
| `enable(0xA9)` | `0xA9` | `0xA9` | PASS |
| `set_mode(0xAB, 2)` | `0xAD` | `0xAD` | PASS |
| `set_mode(0xAF, 2)` | `0xAD` | `0xAD` | PASS |
| `disable(0xAD)` | `0xAC` | `0xAC` | PASS |
| `is_ready(0x09)` | `1` | `1` | PASS |
| `is_ready(0x01)` | `0` | `0` | PASS |

실제 실행 결과:

```text
PASS: enable 0xA8          expected=0xA9, actual=0xA9
PASS: enable 0xA9          expected=0xA9, actual=0xA9
PASS: set_mode 0xAB,2      expected=0xAD, actual=0xAD
PASS: set_mode 0xAF,2      expected=0xAD, actual=0xAD
PASS: disable 0xAD         expected=0xAC, actual=0xAC
PASS: ready 0x09           expected=1, actual=1
PASS: ready 0x01           expected=0, actual=0

Total failures: 0
```

`enable(0xA9)`를 통해 이미 ENABLE이 1인 경우에도 다른 비트를 변경하지 않는 것을 확인했습니다.

`set_mode(0xAF, 2)`를 통해 기존 MODE가 사용 금지 값인 `11`이어도 새 MODE `10`으로 교체할 수 있음을 확인했습니다.

`is_ready(0x09)`를 통해 READY 외의 다른 상태 비트가 함께 켜져 있더라도 bit3만 올바르게 읽는 것을 확인했습니다.

## 아직 확인하지 못한 사항

- 실제 MCU에서의 레지스터 접근
- 실제 MCU의 STATUS 주소 및 접근 폭
- 실제 하드웨어에서 `volatile` 접근 동작 확인
- 인터럽트 또는 다른 실행 주체와 동시에 레지스터를 변경하는 상황
- 실제 하드웨어의 원자적 접근 여부
- MCU별 레지스터 읽기/쓰기 부작용
- 실제 장치의 동작 및 성능

이번 과제에서는 문제에서 보장한 조건에 따라 읽기 부작용과 동시 변경이 없다고 가정하고, 전달받은 8비트 값을 이용한 레지스터 비트 연산과 한정자의 의미를 검증했습니다.