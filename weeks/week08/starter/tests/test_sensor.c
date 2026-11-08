#include "sensor.h"
#include "host.h"
#include <stdio.h>
#include <string.h>

int run_student_tests(void);
static unsigned total, failed;

static void check(const char *name, bool ok)
{
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
    ++total;
    if (!ok) { ++failed; }
}
static const I2cPins test_wiring = {
    .module = 0,
    .sda = { .port = 2, .pin = 4, .output = PORT_ALT5, .drive = OPEN_DRAIN },
    .scl = { .port = 2, .pin = 5, .output = PORT_ALT5, .drive = OPEN_DRAIN }
};

static void reset_on_wired_bus(void)
{
    host_reset();
    (void)bus_configure(test_wiring);
}

static bool configured_sensor_responds(void)
{
    uint8_t id = 0;
    host_reset();
    return board_init_sensor_bus() && bus_read(0x18, 0x0F, &id, 1) && id == 0x33;
}

static bool swapped_pins_disconnect(void)
{
    uint8_t id = 0;
    I2cPins swapped = test_wiring;
    swapped.sda = test_wiring.scl;
    swapped.scl = test_wiring.sda;
    reset_on_wired_bus();
    return !bus_configure(swapped) && !bus_read(0x18, 0x0F, &id, 1);
}

/* 설정 시도 자체를 관찰해 다른 속성의 실패와 독립적으로 검사. */
static I2cPins student_pins(void)
{
    host_reset();
    (void)board_init_sensor_bus();
    return host_last_pins();
}
static bool pin_numbers(void)
{
    I2cPins pins = student_pins();
    return pins.module == 0 && pins.sda.port == 2 && pins.sda.pin == 4 &&
           pins.scl.port == 2 && pins.scl.pin == 5;
}
static bool rejected(I2cPins pins)
{
    uint8_t id = 0;
    reset_on_wired_bus();
    return !bus_configure(pins) && !bus_read(0x18, 0x0F, &id, 1);
}
static bool output_selection(void)
{
    I2cPins actual = student_pins();
    I2cPins wrong_sda = test_wiring, wrong_scl = test_wiring;
    wrong_sda.sda.output = PORT_GPIO;
    wrong_scl.scl.output = PORT_GPIO;
    return actual.sda.output == PORT_ALT5 && actual.scl.output == PORT_ALT5 &&
           rejected(wrong_sda) && rejected(wrong_scl);
}
static bool output_drive(void)
{
    I2cPins actual = student_pins();
    I2cPins wrong_sda = test_wiring, wrong_scl = test_wiring;
    wrong_sda.sda.drive = PUSH_PULL;
    wrong_scl.scl.drive = PUSH_PULL;
    return actual.sda.drive == OPEN_DRAIN && actual.scl.drive == OPEN_DRAIN &&
           rejected(wrong_sda) && rejected(wrong_scl);
}

static bool event_is(size_t n, char op, uint8_t sub, size_t len, uint8_t value)
{
    BusEvent e = host_event(n);
    return e.ok && e.op == op && e.address7 == 0x18 && e.sub == sub &&
           e.len == len && (op != 'W' || e.value == value);
}
static bool init_sequence(void)
{
    Sensor sensor = {false};
    reset_on_wired_bus();
    return sensor_init(&sensor) && sensor.ready && host_bus_count() == 3 &&
           event_is(1, 'R', 0x0F, 1, 0) && event_is(2, 'W', 0x23, 1, 0x88) &&
           event_is(3, 'W', 0x20, 1, 0x57);
}
static bool reject_changed_id(void)
{
    Sensor sensor = {false};
    reset_on_wired_bus();
    if (!sensor_init(&sensor)) { return false; }
    host_set_id(0x32);
    return !sensor_init(&sensor) && !sensor.ready && host_bus_count() == 4 &&
           event_is(4, 'R', 0x0F, 1, 0) && !sensor_sample(&sensor, 3) &&
           host_bus_count() == 4 && host_uart_count() == 0;
}
static bool failed_init(size_t fail_on)
{
    Sensor sensor = {false};
    reset_on_wired_bus();
    host_fail_on(fail_on);
    bool initialized = sensor_init(&sensor);
    return !initialized && !sensor.ready && host_bus_count() == fail_on &&
           !host_event(fail_on).ok && !sensor_sample(&sensor, 1) &&
           host_bus_count() == fail_on && host_uart_count() == 0;
}
static bool idle_sensor(void)
{
    Sensor sensor = {false};
    reset_on_wired_bus();
    return !sensor_init(NULL) && !sensor_sample(NULL, 0) &&
           !sensor_sample(&sensor, 1) && host_bus_count() == 0 && host_uart_count() == 0;
}
static bool combined_read(void)
{
    /* 단위 Test 전제: 초기화에 성공한 상태. 이 Test는 읽기 거래만 확인. */
    Sensor sensor = {true};
    reset_on_wired_bus();
    (void)sensor_sample(&sensor, 9);
    return host_bus_count() == 1 && event_is(1, 'R', 0xA8, 2, 0);
}
static bool failed_read(void)
{
    Sensor sensor = {true};
    reset_on_wired_bus();
    host_fail_on(1);
    return !sensor_sample(&sensor, 42) && host_bus_count() == 1 && host_uart_count() == 0;
}
static bool frame_for(uint16_t seq, uint8_t low, uint8_t high, const char *expected)
{
    Sensor sensor = {false};
    reset_on_wired_bus();
    host_set_x(low, high);
    if (!sensor_init(&sensor) || !sensor_sample(&sensor, seq)) { return false; }
    return host_uart_count() == 1 && strcmp(host_last_frame(), expected) == 0;
}
int main(void)
{
    check("board / I2C0 and SDA-SCL pin numbers", pin_numbers());
    check("board / SDA and SCL output O5", output_selection());
    check("board / SDA and SCL open-drain", output_drive());
    check("board / I2C0 SDA P02.4 SCL P02.5 reads ID", configured_sensor_responds());
    check("board / swapped pins disconnect the Sensor", swapped_pins_disconnect());
    check("init / address7, ID read, CTRL_REG4, CTRL_REG1", init_sequence());
    check("init / changed ID after success", reject_changed_id());
    check("init / ID read failure", failed_init(1));
    check("init / CTRL_REG4 write failure", failed_init(2));
    check("init / CTRL_REG1 write failure", failed_init(3));
    check("sample / not ready or NULL", idle_sensor());
    check("sample / one combined read", combined_read());
    check("decode / positive, distinct Bytes (20 01 -> 18)", sensor_decode_x(0x20, 0x01) == 18);
    check("decode / negative, equal Bytes (F0 F0 -> -241)", sensor_decode_x(0xF0, 0xF0) == -241);
    check("sample / read failure leaves UART empty", failed_read());
    check("UART / raw zero", frame_for(0, 0, 0, "S,0,2048,0\n"));
    check("UART / raw -16", frame_for(42, 0, 0xFF, "S,42,2032,0\n"));
    check("UART / raw minimum", frame_for(8, 0, 0x80, "S,8,0,0\n"));
    check("UART / raw maximum and seq maximum", frame_for(65535, 0xF0, 0x7F, "S,65535,4095,0\n"));
    int extra = run_student_tests();
    printf("fixed: %u/%u PASS; added Test failures: %d\n", total - failed, total, extra);
    return failed != 0 || extra != 0;
}
