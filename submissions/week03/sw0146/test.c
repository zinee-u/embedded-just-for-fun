#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/* Functions under test */
uint8_t enable(uint8_t ctrl);
uint8_t disable(uint8_t ctrl);
uint8_t set_mode(uint8_t ctrl, uint8_t mode);
uint8_t is_ready(uint8_t status);

/* =========================
 * enable() tests
 * ========================= */
static void test_enable(void)
{
    assert(enable(0x00u) == 0x01u);
    assert(enable(0x01u) == 0x01u);
    assert(enable(0x02u) == 0x03u);
    assert(enable(0xFEu) == 0xFFu);

    /* Other bits must be preserved */
    assert(enable(0xA0u) == 0xA1u);

    printf("test_enable: PASS\n");
}

/* =========================
 * disable() tests
 * ========================= */
static void test_disable(void)
{
    assert(disable(0x00u) == 0x00u);
    assert(disable(0x01u) == 0x00u);
    assert(disable(0x03u) == 0x02u);
    assert(disable(0xFFu) == 0xFEu);

    /* Other bits must be preserved */
    assert(disable(0xA1u) == 0xA0u);

    printf("test_disable: PASS\n");
}

/* =========================
 * set_mode() tests
 * ========================= */
static void test_set_mode(void)
{
    /*
     * mode is stored in bits [2:1].
     * Valid mode arguments are 0..2 (README guarantee).
     * Field value 11 is "forbidden" and is never requested as an argument.
     *
     * mode 0 -> bits [2:1] = 00
     * mode 1 -> bits [2:1] = 01
     * mode 2 -> bits [2:1] = 10
     */

    assert(set_mode(0x00u, 0u) == 0x00u);
    assert(set_mode(0x00u, 1u) == 0x02u);
    assert(set_mode(0x00u, 2u) == 0x04u);

    /* Existing mode bits must be replaced */
    assert(set_mode(0x06u, 0u) == 0x00u);

    /* Other bits must be preserved */
    assert(set_mode(0xF9u, 0u) == 0xF9u);
    assert(set_mode(0xF9u, 1u) == 0xFBu);
    assert(set_mode(0xF9u, 2u) == 0xFDu);

    printf("test_set_mode: PASS\n");
}

/* =========================
 * is_ready() tests
 * ========================= */
static void test_is_ready(void)
{
    /* Bit 3 clear */
    assert(is_ready(0x00u) == 0u);
    assert(is_ready(0x07u) == 0u);

    /* Bit 3 set */
    assert(is_ready(0x08u) == 1u);
    assert(is_ready(0x09u) == 1u);
    assert(is_ready(0xFFu) == 1u);

    /* Other bits must not affect the result */
    assert(is_ready(0xF7u) == 0u);
    assert(is_ready(0xF8u) == 1u);

    printf("test_is_ready: PASS\n");
}

/* =========================
 * main
 * ========================= */
int main(void)
{
    test_enable();
    test_disable();
    test_set_mode();
    test_is_ready();

    printf("\nAll tests passed!\n");

    return 0;
}
