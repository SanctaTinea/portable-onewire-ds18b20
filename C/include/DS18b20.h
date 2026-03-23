#ifndef DS18B20_H
#define DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
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
typedef enum {
    DS18B20_RESOLUTION_9BIT  = 0x00,
    DS18B20_RESOLUTION_10BIT = 0x20,
    DS18B20_RESOLUTION_11BIT = 0x40,
    DS18B20_RESOLUTION_12BIT = 0x60
} DS18b20Resolution;

/**
 * @brief DS18B20 device descriptor.
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
typedef struct {
    OneWire* one_wire;
    uint8_t rom_code[8];
    DS18b20Resolution resolution;
    bool has_rom_code;
} DS18b20;

/**
 * @brief Initialize DS18B20 object for known ROM address.
 *
 * @param dev Pointer to device object
 * @param ow Pointer to OneWire bus object
 * @param rom Pointer to 8-byte ROM code
 */
void ds18b20_init_with_rom(DS18b20* dev, OneWire* ow, const uint8_t* rom);

/**
 * @brief Initialize DS18B20 object for a single-device bus.
 *
 * In this mode SKIP ROM is used.
 *
 * @param dev Pointer to device object
 * @param ow Pointer to OneWire bus object
 */
void ds18b20_init(DS18b20* dev, OneWire* ow);

/**
 * @brief Check if a device responds on the bus.
 *
 * @param dev Pointer to device object
 * @return true if presence pulse detected
 * @return false if no device detected
 */
bool ds18b20_is_present(DS18b20* dev);

/**
 * @brief Start temperature conversion.
 *
 * This function only starts conversion.
 * Use ds18b20_wait_for_converting() or ds18b20_read_temp_blocking() if needed.
 *
 * @param dev Pointer to device object
 * @return true if command sent successfully
 * @return false if device not present
 */
bool ds18b20_convert_temp(DS18b20* dev);

/**
 * @brief Read temperature from sensor scratchpad.
 *
 * Performs:
 * - reset
 * - ROM selection
 * - scratchpad read
 * - CRC validation
 *
 * @param dev Pointer to device object
 * @param temp Pointer to output temperature value
 * @return true if temperature was read successfully
 * @return false on bus error / CRC error / invalid pointer
 */
bool ds18b20_read_temp(DS18b20* dev, float* temp);

/**
 * @brief Start conversion and wait until it finishes, then read temperature.
 *
 * @param dev Pointer to device object
 * @param temp Pointer to output temperature value
 * @return true if success
 * @return false if conversion/read failed
 */
bool ds18b20_read_temp_blocking(DS18b20* dev, float* temp);

/**
 * @brief Returns maximum conversion time in ms depending on selected resolution.
 *
 * @param dev Pointer to device object
 * @return Maximum conversion time in milliseconds
 */
uint16_t ds18b20_get_converting_time(DS18b20* dev);

/**
 * @brief Wait maximum conversion time depending on selected resolution.
 *
 * WARNING:
 * This function uses blocking delay and occupies CPU.
 *
 * @param dev Pointer to device object
 */
void ds18b20_wait_for_converting(DS18b20* dev);

/**
 * @brief Read ROM code from device.
 *
 * Works only if exactly one OneWire device is connected to the bus.
 *
 * @param dev Pointer to device object
 * @param rom Pointer to 8-byte output buffer
 * @return true if success
 * @return false if failed
 */
bool ds18b20_read_rom(DS18b20* dev, uint8_t* rom);

/**
 * @brief Set sensor resolution.
 *
 * Writes configuration into scratchpad.
 * This does not permanently save configuration to EEPROM.
 *
 * @param dev Pointer to device object
 * @param resolution New resolution
 * @return true if success
 * @return false if device not present
 */
bool ds18b20_set_resolution(DS18b20* dev, DS18b20Resolution resolution);

#ifdef __cplusplus
}
#endif

#endif /* DS18B20_H */