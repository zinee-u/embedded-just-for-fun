#include "decode_rain.h"

#include <stdio.h>

/* Host demonstration only. puts() is not a hardware driver. */
static void handle_frame(const uint8_t *bytes, size_t len)
{
    const enum wiper_command command = make_wiper_command(bytes, len);

    switch (command) {
    case WIPER_ON:
        puts("ON");
        break;
    case WIPER_OFF:
        puts("OFF");
        break;
    case WIPER_NO_COMMAND:
        /* Diagnostic only: do not send a device command here. */
        puts("DECODE FAILED: no command");
        break;
    }
}

int main(void)
{
    const uint8_t rain_500[] = {0xF4u, 0x01u};
    const uint8_t rain_1001[] = {0xE9u, 0x03u};
    const uint8_t rain_499[] = {0xF3u, 0x01u};

    handle_frame(rain_500, sizeof rain_500);
    handle_frame(rain_1001, sizeof rain_1001);
    handle_frame(rain_499, sizeof rain_499);
    return 0;
}
