#ifndef DS18B20_H
#define DS18B20_H

#include <cstdint>
#include "OneWire.h"

/**
 * @brief Supported DS18B20 conversion resolutions.
 *
 * Higher resolution increases conversion time:
 * - 9-bit  -> 93.75 ms max
 * - 10-bit -> 187.5 ms max
 * - 11-bit -> 375 ms max
 * - 12-bit -> 750 ms max
 */
enum class DS18b20Resolution : uint8_t {
    Bits9  = 0x00,
    Bits10 = 0x20,
    Bits11 = 0x40,
    Bits12 = 0x60
};

/**
 * @brief Driver for DS18B20 temperature sensor.
 *
 * Supports:
 * - presence detection
 * - temperature conversion start
 * - scratchpad read
 * - CRC check
 * - resolution setup
 * - ROM read (single-device bus only)
 *
 * The driver can work:
 * - with a specific ROM code (when multiple devices share the bus)
 * - without ROM code using SKIP ROM (single device on the bus)
 */
class DS18b20 {
public:
    /**
     * @brief Create DS18B20 object for a known ROM address.
     *
     * @param ow   Reference to OneWire bus object
     * @param rom  Pointer to 8-byte ROM code
     */
    DS18b20(OneWire& ow, const uint8_t* rom);

    /**
     * @brief Create DS18B20 object for a single-device bus.
     *
     * In this mode SKIP ROM is used.
     *
     * @param ow Reference to OneWire bus object
     */
    explicit DS18b20(OneWire& ow);

    /**
     * @brief Check if a device responds on the bus.
     *
     * @return true if presence pulse detected
     * @return false if no device detected
     */
    bool isPresent();

    /**
     * @brief Start temperature conversion.
     *
     * This function only starts conversion.
     * Use waitForConverting() or readTempBlocking() if needed.
     *
     * @return true if command sent successfully
     * @return false if device not present
     */
    bool convertTemp();

    /**
     * @brief Read temperature from sensor scratchpad.
     *
     * Performs:
     * - reset
     * - ROM selection
     * - scratchpad read
     * - CRC validation
     *
     * @param temp Pointer to output temperature value
     * @return true if temperature was read successfully
     * @return false on bus error / CRC error / invalid pointer
     */
    bool readTemp(float* temp);

    /**
     * @brief Start conversion and wait until it finishes, then read temperature.
     *
     * @param temp Pointer to output temperature value
     * @return true if success
     * @return false if conversion/read failed
     */
    bool readTempBlocking(float* temp);

    /**
     * @brief Wait maximum conversion time depending on selected resolution.
     *
     * WARNING:
     * This function uses blocking delay and occupies CPU.
     */
    void waitForConverting();

    /**
     * @brief Read ROM code from device.
     *
     * Works only if exactly one OneWire device is connected to the bus.
     *
     * @param rom Pointer to 8-byte output buffer
     * @return true if success
     * @return false if failed
     */
    bool readROM(uint8_t* rom);

    /**
     * @brief Set sensor resolution.
     *
     * Writes configuration into scratchpad.
     * This does not permanently save configuration to EEPROM.
     *
     * @param resolution New resolution
     * @return true if success
     * @return false if device not present
     */
    bool setResolution(DS18b20Resolution resolution);

private:
    /**
     * @brief Select target device on the OneWire bus.
     *
     * Uses MATCH ROM if ROM code is known,
     * otherwise uses SKIP ROM.
     */
    void selectDevice();

    /**
     * @brief Calculate Dallas/Maxim CRC-8.
     *
     * Used for ROM and scratchpad validation.
     *
     * @param data Pointer to input bytes
     * @param size Number of bytes
     * @return Calculated CRC8
     */
    static uint8_t crc8Dallas(const uint8_t* data, uint8_t size);

private:
    static constexpr uint8_t CMD_CONVERT_T        = 0x44;
    static constexpr uint8_t CMD_SKIP_ROM         = 0xCC;
    static constexpr uint8_t CMD_READ_ROM         = 0x33;
    static constexpr uint8_t CMD_MATCH_ROM        = 0x55;
    static constexpr uint8_t CMD_READ_SCRATCHPAD  = 0xBE;
    static constexpr uint8_t CMD_WRITE_SCRATCHPAD = 0x4E;

    static constexpr float TEMPERATURE_LSB = 0.0625f;

    OneWire& oneWire;
    uint8_t romCode[8] = {0};
    DS18b20Resolution resolution = DS18b20Resolution::Bits12;
    bool hasRomCode = false;
};

#endif /* DS18B20_H */
