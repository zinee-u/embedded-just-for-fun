#include <stdint.h>

#define CTRL_ENABLE_MASK   0x01u   /* bit0 */
#define CTRL_MODE_MASK     0x06u   /* bits2:1 */
#define CTRL_MODE_SHIFT    1u
#define STATUS_READY_MASK  0x08u   /* bit3 */
#define STATUS_READY_SHIFT 3u

uint8_t enable(uint8_t ctrl)
{
    return (uint8_t)(ctrl | CTRL_ENABLE_MASK);
}

uint8_t disable(uint8_t ctrl)
{
    return (uint8_t)(ctrl & ~CTRL_ENABLE_MASK);
}

uint8_t set_mode(uint8_t ctrl, uint8_t mode)
{
    return (uint8_t)((ctrl & ~CTRL_MODE_MASK) |
                     ((mode << CTRL_MODE_SHIFT) & CTRL_MODE_MASK));
}

uint8_t is_ready(uint8_t status)
{
    return (uint8_t)((status & STATUS_READY_MASK) >>
                     STATUS_READY_SHIFT);
}