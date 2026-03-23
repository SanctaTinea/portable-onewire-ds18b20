#ifndef ONEWIRE_H
#define ONEWIRE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Low-level OneWire bus driver (bit-banging implementation).
 *
 * This module provides basic OneWire protocol operations:
 *  - Reset / presence detection
 *  - Write byte(s)
 *  - Read byte(s)
 *
 * Hardware access is abstracted via function pointers:
 *  - low_line()     -> actively pull bus LOW
 *  - release_line() -> release bus (open-drain HIGH via pull-up)
 *  - read_line()    -> read current bus level
 *  - delay_us()     -> microsecond delay
 *
 * IMPORTANT:
 *  - Bus must be configured as open-drain with pull-up resistor.
 *  - Timing must be precise (interrupts may break protocol).
 */

typedef struct {
    void (*low_line)(void);
    void (*release_line)(void);
    uint8_t (*read_line)(void);
    void (*delay_us)(uint16_t us);
} OneWire;

/**
 * @brief Initialize OneWire driver object
 *
 * @param ow            Pointer to OneWire object
 * @param low_line      Function to pull line LOW
 * @param release_line  Function to release line
 * @param read_line     Function to read line level
 * @param delay_us      Microsecond delay function
 */
void onewire_init(OneWire* ow,
                  void (*low_line)(void),
                  void (*release_line)(void),
                  uint8_t (*read_line)(void),
                  void (*delay_us)(uint16_t us));

/**
 * @brief Send reset pulse and detect device presence
 *
 * @param ow Pointer to OneWire object
 * @return true if there is device, false otherwise
 */
bool onewire_reset(OneWire* ow);

/**
 * @brief Write one byte to bus
 *
 * @param ow Pointer to OneWire object
 * @param data Byte to send
 */
void onewire_write_byte(OneWire* ow, uint8_t data);

/**
 * @brief Write multiple bytes
 *
 * @param ow Pointer to OneWire object
 * @param data Pointer to data buffer
 * @param size Number of bytes
 */
void onewire_write(OneWire* ow, const uint8_t* data, uint16_t size);

/**
 * @brief Read one byte from bus
 *
 * @param ow Pointer to OneWire object
 * @return Read byte
 */
uint8_t onewire_read_byte(OneWire* ow);

/**
 * @brief Read multiple bytes
 *
 * @param ow Pointer to OneWire object
 * @param data Output buffer
 * @param size Number of bytes
 */
void onewire_read(OneWire* ow, uint8_t* data, uint16_t size);

/**
 * @brief Millisecond delay (based on delay_us)
 *
 * WARNING: Busy-wait, blocks CPU.
 *
 * @param ow Pointer to OneWire object
 * @param ms Delay in milliseconds
 */
void onewire_delay_ms(OneWire* ow, uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif /* ONEWIRE_H */