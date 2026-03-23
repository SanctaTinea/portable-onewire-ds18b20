#include "OneWire.h"

#ifdef __cplusplus
extern "C" {
#endif

void onewire_init(OneWire* ow,
                  void (*low_line)(void),
                  void (*release_line)(void),
                  uint8_t (*read_line)(void),
                  void (*delay_us)(uint16_t us))
{
    if (ow == NULL) {
        return;
    }

    ow->low_line = low_line;
    ow->release_line = release_line;
    ow->read_line = read_line;
    ow->delay_us = delay_us;
}

bool onewire_reset(OneWire* ow)
{
    uint8_t presence;

    if (ow == NULL ||
        ow->low_line == NULL ||
        ow->release_line == NULL ||
        ow->read_line == NULL ||
        ow->delay_us == NULL) {
        return false;
    }

    ow->delay_us(20);              /* Recovery time before reset */

    ow->low_line();
    ow->delay_us(500);             /* Reset pulse (480–960 us) */

    ow->release_line();
    ow->delay_us(70);              /* Wait before sampling presence */

    presence = (ow->read_line() == 0U); /* Device pulls line LOW */

    ow->delay_us(500);             /* Recovery after presence */

    return (presence != 0U);
}

void onewire_write_byte(OneWire* ow, uint8_t data)
{
    uint8_t mask;

    if (ow == NULL ||
        ow->low_line == NULL ||
        ow->release_line == NULL ||
        ow->delay_us == NULL) {
        return;
    }

    for (mask = 1U; mask != 0U; mask <<= 1U) {

        ow->release_line();
        ow->delay_us(2);

        ow->low_line(); /* Start time slot */

        if ((data & mask) == 0U) {
            /* Write '0' */
            ow->delay_us(70);
            ow->release_line();
            ow->delay_us(2);
        } else {
            /* Write '1' */
            ow->delay_us(10);
            ow->release_line();
            ow->delay_us(55);
        }
    }
}

void onewire_write(OneWire* ow, const uint8_t* data, uint16_t size)
{
    if (ow == NULL || data == NULL) {
        return;
    }

    while (size--) {
        onewire_write_byte(ow, *data++);
    }
}

uint8_t onewire_read_byte(OneWire* ow)
{
    uint8_t data = 0;
    uint8_t i;

    if (ow == NULL ||
        ow->low_line == NULL ||
        ow->release_line == NULL ||
        ow->read_line == NULL ||
        ow->delay_us == NULL) {
        return 0;
    }

    for (i = 0; i < 8; i++) {

        ow->release_line();
        ow->delay_us(2);

        ow->low_line();
        ow->delay_us(3);           /* Start read slot */

        ow->release_line();
        ow->delay_us(10);          /* Wait before sampling */

        data |= (uint8_t)((ow->read_line() != 0U) << i);

        ow->delay_us(55);          /* End of timeslot */
    }

    return data;
}

void onewire_read(OneWire* ow, uint8_t* data, uint16_t size)
{
    if (ow == NULL || data == NULL) {
        return;
    }

    while (size--) {
        *data++ = onewire_read_byte(ow);
    }
}

void onewire_delay_ms(OneWire* ow, uint16_t ms)
{
    if (ow == NULL || ow->delay_us == NULL) {
        return;
    }

    while (ms--) {
        ow->delay_us(1000);
    }
}

#ifdef __cplusplus
}
#endif