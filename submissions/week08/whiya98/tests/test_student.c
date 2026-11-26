
#include "sensor.h"
#include "host.h"
#include <stdio.h>
#include <string.h>

/* TC01: Ignore unused lower 4 bits */
static bool test_ignore_lower_bits(void)
{
    host_reset();

    return sensor_decode_x(0x20, 0x01) == 18 &&
           sensor_decode_x(0x25, 0x01) == 18 &&
           sensor_decode_x(0x2F, 0x01) == 18 &&
           sensor_decode_x(0xFF, 0xFF) == -1;
}

/* TC02: Preserve previous UART frame on read failure */
static bool test_preserve_previous_frame(void)
{
    Sensor sensor = {false};

    host_reset();

    if (!board_init_sensor_bus() ||
        !sensor_init(&sensor)) {
        return false;
    }

    host_set_x(0x00, 0xFF);

    if (!sensor_sample(&sensor, 42)) {
        return false;
    }

    if (host_uart_count() != 1 ||
        strcmp(host_last_frame(), "S,42,2032,0\n") != 0) {
        return false;
    }

    host_set_x(0x20, 0x01);
    host_fail_on(host_bus_count() + 1);

    return !sensor_sample(&sensor, 43) &&
           host_bus_count() == 5 &&
           !host_event(5).ok &&
           host_uart_count() == 1 &&
           strcmp(host_last_frame(), "S,42,2032,0\n") == 0;
}

/* TC03: Recover after a temporary I2C read failure */
static bool test_recovery_after_read_failure(void)
{
    Sensor sensor = {false};

    host_reset();

    if (!board_init_sensor_bus() ||
        !sensor_init(&sensor)) {
        return false;
    }

    host_set_x(0x00, 0x00);

    if (!sensor_sample(&sensor, 100) ||
        host_uart_count() != 1) {
        return false;
    }

    host_fail_on(host_bus_count() + 1);

    if (sensor_sample(&sensor, 101) ||
        host_uart_count() != 1 ||
        !sensor.ready) {
        return false;
    }

    host_fail_on(0);
    host_set_x(0x20, 0x01);

    return sensor_sample(&sensor, 102) &&
           sensor.ready &&
           host_bus_count() == 6 &&
           host_uart_count() == 2 &&
           strcmp(host_last_frame(), "S,102,2066,0\n") == 0;
}

/* Print test result and return failure count */
static int check_student(const char *name, bool ok)
{
    printf("[%s] student / %s\n",
           ok ? "PASS" : "FAIL", name);

    return ok ? 0 : 1;
}

int run_student_tests(void)
{
    int failed = 0;

    failed += check_student(
        "ignore lower 4 bits",
        test_ignore_lower_bits());

    failed += check_student(
        "preserve previous UART frame",
        test_preserve_previous_frame());

    failed += check_student(
        "recovery after I2C read failure",
        test_recovery_after_read_failure());

    return failed;
}
