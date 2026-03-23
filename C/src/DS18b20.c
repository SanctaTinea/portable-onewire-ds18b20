#include "DS18b20.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DS18B20_CMD_CONVERT_T        0x44U
#define DS18B20_CMD_SKIP_ROM         0xCCU
#define DS18B20_CMD_READ_ROM         0x33U
#define DS18B20_CMD_MATCH_ROM        0x55U
#define DS18B20_CMD_READ_SCRATCHPAD  0xBEU
#define DS18B20_CMD_WRITE_SCRATCHPAD 0x4EU

#define DS18B20_TEMPERATURE_LSB      0.0625f

static void ds18b20_select_device(DS18b20* dev);
static uint8_t ds18b20_crc8_dallas(const uint8_t* data, uint8_t size);

void ds18b20_init_with_rom(DS18b20* dev, OneWire* ow, const uint8_t* rom)
{
    uint8_t i;

    if (dev == NULL) {
        return;
    }

    dev->one_wire = ow;
    dev->resolution = DS18B20_RESOLUTION_12BIT;
    dev->has_rom_code = false;

    for (i = 0; i < 8U; ++i) {
        dev->rom_code[i] = 0U;
    }

    if (ow == NULL) {
        return;
    }

    if (rom != NULL) {
        for (i = 0; i < 8U; ++i) {
            dev->rom_code[i] = rom[i];
        }
        dev->has_rom_code = true;
    }
}

void ds18b20_init(DS18b20* dev, OneWire* ow)
{
    if (dev == NULL) {
        return;
    }

    dev->one_wire = ow;
    dev->resolution = DS18B20_RESOLUTION_12BIT;
    dev->has_rom_code = false;

    for (uint8_t i = 0; i < 8U; ++i) {
        dev->rom_code[i] = 0U;
    }
}

bool ds18b20_is_present(DS18b20* dev)
{
    if (dev == NULL || dev->one_wire == NULL) {
        return false;
    }

    return onewire_reset(dev->one_wire);
}

bool ds18b20_convert_temp(DS18b20* dev)
{
    if (dev == NULL || dev->one_wire == NULL) {
        return false;
    }

    if (!onewire_reset(dev->one_wire)) {
        return false;
    }

    ds18b20_select_device(dev);
    onewire_write_byte(dev->one_wire, DS18B20_CMD_CONVERT_T);

    return true;
}

bool ds18b20_read_temp(DS18b20* dev, float* temp)
{
    uint8_t data[9];
    int16_t raw_temp;

    if (dev == NULL || dev->one_wire == NULL || temp == NULL) {
        return false;
    }

    if (!onewire_reset(dev->one_wire)) {
        return false;
    }

    ds18b20_select_device(dev);
    onewire_write_byte(dev->one_wire, DS18B20_CMD_READ_SCRATCHPAD);
    onewire_read(dev->one_wire, data, 9U);

    if (ds18b20_crc8_dallas(data, 8U) != data[8]) {
        return false;
    }

    raw_temp = (int16_t)(((uint16_t)data[1] << 8U) | data[0]);
    *temp = (float)raw_temp * DS18B20_TEMPERATURE_LSB;

    return true;
}

uint16_t ds18b20_get_converting_time(DS18b20* dev)
{
    if (dev == NULL) {
        return 750U;
    }

    switch (dev->resolution) {
        case DS18B20_RESOLUTION_9BIT:
            return 94U;
        case DS18B20_RESOLUTION_10BIT:
            return 188U;
        case DS18B20_RESOLUTION_11BIT:
            return 375U;
        case DS18B20_RESOLUTION_12BIT:
        default:
            return 750U;
    }
}

void ds18b20_wait_for_converting(DS18b20* dev)
{
    if (dev == NULL || dev->one_wire == NULL) {
        return;
    }

    onewire_delay_ms(dev->one_wire, ds18b20_get_converting_time(dev));
}

bool ds18b20_read_temp_blocking(DS18b20* dev, float* temp)
{
    if (!ds18b20_convert_temp(dev)) {
        return false;
    }

    ds18b20_wait_for_converting(dev);
    return ds18b20_read_temp(dev, temp);
}

bool ds18b20_read_rom(DS18b20* dev, uint8_t* rom)
{
    if (dev == NULL || dev->one_wire == NULL || rom == NULL) {
        return false;
    }

    if (!onewire_reset(dev->one_wire)) {
        return false;
    }

    onewire_write_byte(dev->one_wire, DS18B20_CMD_READ_ROM);
    onewire_read(dev->one_wire, rom, 8U);

    return true;
}

bool ds18b20_set_resolution(DS18b20* dev, DS18b20Resolution new_resolution)
{
    uint8_t config;

    if (dev == NULL || dev->one_wire == NULL) {
        return false;
    }

    if (!onewire_reset(dev->one_wire)) {
        return false;
    }

    ds18b20_select_device(dev);

    onewire_write_byte(dev->one_wire, DS18B20_CMD_WRITE_SCRATCHPAD);

    /* TH register */
    onewire_write_byte(dev->one_wire, 0x00U);

    /* TL register */
    onewire_write_byte(dev->one_wire, 0x00U);

    /*
     * Configuration register:
     * bits 6:5 define resolution,
     * lower fixed bits must remain 1s
     */
    config = (uint8_t)(0x1FU | (uint8_t)new_resolution);
    onewire_write_byte(dev->one_wire, config);

    dev->resolution = new_resolution;
    return true;
}

static void ds18b20_select_device(DS18b20* dev)
{
    if (dev->has_rom_code) {
        onewire_write_byte(dev->one_wire, DS18B20_CMD_MATCH_ROM);
        onewire_write(dev->one_wire, dev->rom_code, 8U);
    } else {
        onewire_write_byte(dev->one_wire, DS18B20_CMD_SKIP_ROM);
    }
}

static uint8_t ds18b20_crc8_dallas(const uint8_t* data, uint8_t size)
{
    uint8_t crc = 0U;

    while (size--) {
        uint8_t in_byte = *data++;

        for (uint8_t i = 0U; i < 8U; ++i) {
            uint8_t mix = (uint8_t)((crc ^ in_byte) & 0x01U);
            crc >>= 1U;

            if (mix != 0U) {
                crc ^= 0x8CU;
            }

            in_byte >>= 1U;
        }
    }

    return crc;
}

#ifdef __cplusplus
}
#endif