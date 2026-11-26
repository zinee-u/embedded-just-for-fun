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
        .sda = {
            .port = 2,
            .pin = 4,
            .output = PORT_ALT5,
            .drive = OPEN_DRAIN
        },
        .scl = {
            .port = 2,
            .pin = 5,
            .output = PORT_ALT5,
            .drive = OPEN_DRAIN
        }
    };

    return bus_configure(pins);
}

bool sensor_init(Sensor *sensor)
{
    uint8_t id = 0;

    if (sensor == NULL) {
        return false;
    }

    sensor->ready = false;

    /* Read sensor ID */
    if (!bus_read(ADDRESS7, WHO_AM_I, &id, 1)) {
        return false;
    }

    /* Verify sensor ID */
    if (id != 0x33) {
        return false;
    }

    /* Enable BDU and high-resolution mode */
    if (!bus_write(ADDRESS7, CTRL_REG4, 0x88)) {
        return false;
    }

    /* Start measurement at 100 Hz */
    if (!bus_write(ADDRESS7, CTRL_REG1, 0x57)) {
        return false;
    }

    sensor->ready = true;

    return true;
}

int16_t sensor_decode_x(uint8_t low, uint8_t high)
{
    /* Combine Low and High bytes */
    uint32_t word = ((uint32_t)high << 8) | low;

    /* Extract upper 12 bits */
    uint32_t raw12 = word >> 4;

    /* Convert 12-bit two's complement to signed integer */
    if (raw12 >= 2048u) {
        return (int16_t)((int32_t)raw12 - 4096);
    }

    return (int16_t)raw12;
}

bool sensor_sample(const Sensor *sensor, uint16_t seq)
{
    uint8_t bytes[2] = {0, 0};

    if (sensor == NULL || !sensor->ready) {
        return false;
    }

    /* Read two consecutive registers */
    if (!bus_read(ADDRESS7, OUT_X_L | 0x80,
                  bytes, sizeof bytes)) {
        return false;
    }

    int16_t raw = sensor_decode_x(bytes[0], bytes[1]);

    return uart_record(seq, (uint16_t)(raw + 2048));
}
