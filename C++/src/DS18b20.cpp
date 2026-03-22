/*
 * DS18b20.cpp
 *
 *  Created on: Mar 22, 2026
 *      Author: SanctaTinea
 */

#include "DS18b20.h"

DS18b20::DS18b20(OneWire& ow, const uint8_t* rom)
    : oneWire(ow), hasRomCode(true)
{
    if (rom != nullptr) {
        for (uint8_t i = 0; i < 8; ++i) {
            romCode[i] = rom[i];
        }
    } else {
        hasRomCode = false;
    }
}

DS18b20::DS18b20(OneWire& ow)
    : oneWire(ow)
{
}

bool DS18b20::isPresent() {
    return oneWire.reset();
}

bool DS18b20::convertTemp() {
    if (!oneWire.reset()) {
        return false;
    }

    selectDevice();
    oneWire.write(CMD_CONVERT_T);

    return true;
}

bool DS18b20::readTemp(float* temp) {
    if (temp == nullptr) {
        return false;
    }

    uint8_t data[9];

    if (!oneWire.reset()) {
        return false;
    }

    selectDevice();
    oneWire.write(CMD_READ_SCRATCHPAD);
    oneWire.read(data, 9);

    if (crc8Dallas(data, 8) != data[8]) {
        return false;
    }

    const int16_t rawTemp = static_cast<int16_t>((data[1] << 8) | data[0]);
    *temp = static_cast<float>(rawTemp) * TEMPERATURE_LSB;

    return true;
}

uint16_t DS18b20::getConvertingTime() {
	switch (resolution) {
		case DS18b20Resolution::Bits9: return 94;
		case DS18b20Resolution::Bits10: return 188;
		case DS18b20Resolution::Bits11: return 375;
		case DS18b20Resolution::Bits12: return 750;
	}
}

void DS18b20::waitForConverting() {
	oneWire.delayMs(getConvertingTime());
}

bool DS18b20::readTempBlocking(float* temp) {
    if (!convertTemp()) {
        return false;
    }

    waitForConverting();
    return readTemp(temp);
}

bool DS18b20::readROM(uint8_t* rom) {
    if (rom == nullptr) {
        return false;
    }

    if (!oneWire.reset()) {
        return false;
    }

    oneWire.write(CMD_READ_ROM);
    oneWire.read(rom, 8);

    return true;
}

bool DS18b20::setResolution(DS18b20Resolution newResolution) {
    if (!oneWire.reset()) {
        return false;
    }

    selectDevice();

    oneWire.write(CMD_WRITE_SCRATCHPAD);

    // TH register
    oneWire.write(0x00);

    // TL register
    oneWire.write(0x00);

    // Configuration register:
    // bits 6:5 define resolution, lower fixed bits must remain 1s
    oneWire.write(static_cast<uint8_t>(0x1F | static_cast<uint8_t>(newResolution)));

    resolution = newResolution;
    return true;
}

void DS18b20::selectDevice() {
    if (hasRomCode) {
        oneWire.write(CMD_MATCH_ROM);
        oneWire.write(romCode, 8);
    } else {
        oneWire.write(CMD_SKIP_ROM);
    }
}

uint8_t DS18b20::crc8Dallas(const uint8_t* data, uint8_t size) {
    uint8_t crc = 0;

    while (size--) {
        uint8_t inByte = *data++;

        for (uint8_t i = 0; i < 8; ++i) {
            const uint8_t mix = (crc ^ inByte) & 0x01;
            crc >>= 1;

            if (mix) {
                crc ^= 0x8C;
            }

            inByte >>= 1;
        }
    }

    return crc;
}



