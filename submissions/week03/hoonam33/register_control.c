#include <stdint.h>

#define ENABLE_MASK  0x01u   // 0000 0001
#define MODE_MASK    0x06u   // 0000 0110
#define READY_MASK   0x08u   // 0000 1000

volatile const uint8_t *status;

uint8_t enable(uint8_t ctrl) { 
    return ctrl | ENABLE_MASK; 
}

uint8_t set_mode(uint8_t ctrl, uint8_t mode) {
    
    ctrl = ctrl & ~MODE_MASK;
    ctrl = ctrl | ((mode << 1) & MODE_MASK);

    return ctrl;
}

uint8_t disable(uint8_t ctrl) { 
    return ctrl & ~ENABLE_MASK; 
}

uint8_t is_ready(uint8_t status) {
    return (status & READY_MASK) >> 3;
}