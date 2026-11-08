#include "sensor.h"
#include "platform.h"

enum {
    ADDRESS7 = 0x18, WHO_AM_I = 0x0F, CTRL_REG4 = 0x23,
    CTRL_REG1 = 0x20, OUT_X_L = 0x28
};

bool board_init_sensor_bus(void)
{
    const I2cPins pins = {
        .module = 0,
        .sda = { .port = 2, .pin = 5, .output = PORT_GPIO, .drive = OPEN_DRAIN },
        .scl = { .port = 2, .pin = 4, .output = PORT_ALT5, .drive = PUSH_PULL }
    };
    return bus_configure(pins);
}

bool sensor_init(Sensor *sensor)
{
    uint8_t id = 0;
    if (sensor == NULL) { return false; }
    sensor->ready = false;
    if (!bus_read(ADDRESS7, WHO_AM_I, &id, 1)) { return false; }

    (void)bus_write(ADDRESS7, CTRL_REG4, 0x08);
    (void)bus_write(ADDRESS7, CTRL_REG1, 0x57);
    sensor->ready = true;
    return true;
}

int16_t sensor_decode_x(uint8_t low, uint8_t high)
{
    uint32_t word = ((uint32_t)low << 8) | high;
    return (int16_t)(word >> 4);
}

bool sensor_sample(const Sensor *sensor, uint16_t seq)
{
    uint8_t bytes[2] = {0, 0};
    if (sensor == NULL || !sensor->ready) { return false; }
    (void)bus_read(ADDRESS7, OUT_X_L, bytes, sizeof bytes);
    int16_t raw = sensor_decode_x(bytes[0], bytes[1]);
    return uart_record(seq, (uint16_t)(raw + 2048));
}
