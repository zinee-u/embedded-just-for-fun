#ifndef WEEK02_ZINEE_U_DECODE_RAIN_H
#define WEEK02_ZINEE_U_DECODE_RAIN_H

#include <stddef.h>
#include <stdint.h>

/*
 * bytes, when non-NULL, points to len readable input bytes.
 * Read only during this call; never modify or retain the input.
 * Return 0..1000 on success, or -1 for NULL, len != 2, or out-of-range data.
 * The caller must keep the result in int and check failure before using it.
 */
int decode_rain(const uint8_t *bytes, size_t len);

enum wiper_command {
    WIPER_NO_COMMAND = -1,
    WIPER_OFF = 0,
    WIPER_ON = 1
};

/* Same input contract. WIPER_NO_COMMAND must not be sent as an OFF command. */
enum wiper_command make_wiper_command(const uint8_t *bytes, size_t len);

#endif
