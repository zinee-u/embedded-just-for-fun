#include <stddef.h>
#include <stdint.h>

#define RAIN_ERROR (-1)

int decode_rain(const uint8_t *bytes, size_t len)
{
    if (bytes == NULL) {
        return RAIN_ERROR;
    }

    if (len != 2U) {
        return RAIN_ERROR;
    }

    uint32_t rain =
        (uint32_t)bytes[0] |
        ((uint32_t)bytes[1] << 8);

    if (rain > 1000U) {
        return RAIN_ERROR;
    }

    return (int)rain;
}