#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

/* Educational Host interface; these are NOT TC387QP SFR/Port definitions. */
typedef enum { PORT_LED = 0, PORT_ENABLE = 1, PORT_COUNT = 2 } PlatformPort;
#define LED_MASK UINT32_C(1)
#define ENABLE_MASK UINT32_C(1)

void platform_port_write(PlatformPort port, uint32_t set, uint32_t clear);
void platform_port_output_pushpull(PlatformPort port, uint32_t mask);
void platform_port_input(PlatformPort port, uint32_t mask, bool pullup);
bool platform_port_read(PlatformPort port, uint32_t mask);
void platform_port_toggle(PlatformPort port, uint32_t mask);
uint16_t platform_sensor_read(void); /* Always returns 0..4095 in this task. */

/* Legacy API: records requests on Host; MUST NOT be called by collector_step. */
void platform_delay_ms(uint32_t ms);

#endif
