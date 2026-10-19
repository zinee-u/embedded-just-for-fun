
#include "mission.h"
#include "platform.h"

/*
 * G1: Output 모드 전 HIGH Latch 준비
 * G2: Active LOW LED는 HIGH일 때 OFF
 * G3: ENABLE 입력 Pull-up 활성화
 */
void board_io_init(void)
{
    // LED를 먼저 OFF(HIGH) 상태로 설정
    platform_port_write(PORT_LED, LED_MASK, LED_MASK);

    // 출력값을 준비한 다음 Push-pull Output으로 전환
    platform_port_output_pushpull(PORT_LED, LED_MASK);

    // ENABLE 입력에 Pull-up 적용
    platform_port_input(PORT_ENABLE, ENABLE_MASK, true);
}

/*
 * G4: ENABLE은 Active LOW
 * LOW일 때 수집 허용(true)
 */
bool board_collection_enabled(void)
{
    return !platform_port_read(PORT_ENABLE, ENABLE_MASK);
}

/*
 * G5: LED Bit만 Toggle
 * 같은 Port의 다른 Bit는 변경하지 않음
 */
void board_led_toggle(void)
{
    platform_port_toggle(PORT_LED, LED_MASK);
}
