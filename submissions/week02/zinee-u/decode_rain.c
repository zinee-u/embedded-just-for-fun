#include "decode_rain.h"

int decode_rain(const uint8_t *bytes, size_t len)
{
    if (bytes == NULL || len != 2u) {
        return -1;
    }

    const uint16_t rain = (uint16_t)(
        (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8u));

    if (rain > 1000u) {
        return -1;
    }

    return (int)rain;
}

enum wiper_command make_wiper_command(const uint8_t *bytes, size_t len)
{
    const int rain = decode_rain(bytes, len);

    if (rain < 0) {
        return WIPER_NO_COMMAND;
    }

    return rain >= 500 ? WIPER_ON : WIPER_OFF;
}
