#include "register_control.h"

/* ENABLE(bit0)을 1로 설정하고 나머지 비트 보존 */
uint8_t enable(uint8_t ctrl)
{
    return (uint8_t)(ctrl | 0x01u);
}

/* ENABLE(bit0)을 0으로 설정하고 나머지 비트 보존 */
uint8_t disable(uint8_t ctrl)
{
    return (uint8_t)(ctrl & (uint8_t)~0x01u);
}

/* 기존 MODE(bits2:1)를 제거하고 새로운 MODE 설정 */
uint8_t set_mode(uint8_t ctrl, uint8_t mode)
{
    return (uint8_t)((ctrl & (uint8_t)~0x06u)
                     | ((uint8_t)mode << 1));
}

/* STATUS의 READY(bit3)만 추출 */
uint8_t is_ready(uint8_t status)
{
    return (uint8_t)((status >> 3) & 0x01u);
}
