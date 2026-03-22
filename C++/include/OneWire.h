#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <cstdint>

/**
 * @brief Low-level OneWire bus driver (bit-banging implementation).
 *
 * This class provides basic OneWire protocol operations:
 *  - Reset / presence detection
 *  - Write byte(s)
 *  - Read byte(s)
 *
 * Hardware access is abstracted via function pointers:
 *  - lowLine()      -> actively pull bus LOW
 *  - releaseLine()  -> release bus (open-drain HIGH via pull-up)
 *  - readLine()     -> read current bus level
 *  - delayUs()      -> microsecond delay
 *
 * IMPORTANT:
 *  - Bus must be configured as open-drain with pull-up resistor.
 *  - Timing must be precise (interrupts may break protocol).
 */
class OneWire {
public:

    /**
     * @brief Constructor
     *
     * @param _lowLine      Function to pull line LOW
     * @param _releaseLine  Function to release line (HIGH via pull-up)
     * @param _readLine     Function to read line (returns 0 or 1)
     * @param _delayUs      Microsecond delay function
     */
    OneWire(void (*_lowLine)(),
            void (*_releaseLine)(),
            uint8_t (*_readLine)(),
            void (*_delayUs)(uint16_t us));

    /**
     * @brief Send reset pulse and detect device presence
     *
     * @return 1 if there is device, or 0 if not
     */
    bool reset();

    /**
     * @brief Write one byte to bus
     */
    void write(uint8_t data);

    /**
     * @brief Write multiple bytes
     */
    void write(const uint8_t* data, uint16_t size);

    /**
     * @brief Read one byte from bus
     */
    uint8_t read();

    /**
     * @brief Read multiple bytes
     */
    void read(uint8_t* data, uint16_t size);

    /**
     * @brief Millisecond delay (based on delayUs)
     *
     * WARNING: Busy-wait, blocks CPU.
     */
    void delayMs(uint16_t ms);

private:
    void (*lowLine)();
    void (*releaseLine)();
    uint8_t (*readLine)();
    void (*delayUs)(uint16_t us);
};

#endif /* ONEWIRE_H */
