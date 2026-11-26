#include "host.h"
#include "sensor.h"
#include <stdio.h>

int main(void)
{
    Sensor sensor = {false};
    host_reset();
    host_set_x(0x00, 0xFF);
    bool connected = board_init_sensor_bus();
    I2cPins pins = host_last_pins();
    printf("I2C%u SDA=P%02u.%u SCL=P%02u.%u %s\n",
           (unsigned)pins.module, (unsigned)pins.sda.port, (unsigned)pins.sda.pin,
           (unsigned)pins.scl.port, (unsigned)pins.scl.pin, connected ? "OK" : "FAIL");
    printf("SDA=O%u/%s SCL=O%u/%s\n", (unsigned)pins.sda.output,
           pins.sda.drive == OPEN_DRAIN ? "open-drain" : "push-pull",
           (unsigned)pins.scl.output, pins.scl.drive == OPEN_DRAIN ? "open-drain" : "push-pull");
    bool initialized = connected && sensor_init(&sensor);
    bool sampled = initialized && sensor_sample(&sensor, 42);
    printf("Fixture: WHO_AM_I=0x33  OUT_X_L=0x00  OUT_X_H=0xFF  seq=42\n");
    for (size_t i = 1; i <= host_bus_count(); ++i) {
        BusEvent ev = host_event(i);
        printf("%zu %c address7=0x%02X sub=0x%02X len=%zu", i, ev.op,
               (unsigned)ev.address7, (unsigned)ev.sub, ev.len);
        if (ev.op == 'W') { printf(" value=0x%02X", (unsigned)ev.value); }
        printf(" %s\n", ev.ok ? "OK" : "FAIL");
    }
    printf("init=%d sample=%d UART=%zu\n", initialized, sampled, host_uart_count());
    if (host_uart_count()) { printf("%s", host_last_frame()); }
    return 0;
}
