#ifndef REGISTER_CONTROL_H
#define REGISTER_CONTROL_H

#include <stdint.h>

uint8_t enable(uint8_t ctrl);
uint8_t disable(uint8_t ctrl);
uint8_t set_mode(uint8_t ctrl, uint8_t mode);
uint8_t is_ready(uint8_t status);

#endif
