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