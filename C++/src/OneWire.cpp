/*
 * OneWire.cpp
 *
 *  Created on: Mar 22, 2026
 *      Author: SanctaTinea
 */


#include "OneWire.h"

OneWire::OneWire(void (*_lowLine)(),
                 void (*_releaseLine)(),
                 uint8_t (*_readLine)(),
                 void (*_delayUs)(uint16_t us))
    : lowLine(_lowLine),
      releaseLine(_releaseLine),
      readLine(_readLine),
      delayUs(_delayUs)
{
}

bool OneWire::reset() {
    delayUs(20);                          // Recovery time before reset

    lowLine(); delayUs(500);              // Reset pulse (480–960 us)

    releaseLine(); delayUs(70);           // Wait before sampling presence

    uint8_t presence = (readLine() == 0); // Device pulls line LOW

    delayUs(500);                         // Recovery after presence

    return presence;
}

void OneWire::write(uint8_t data) {
    for (uint8_t i = 1; i != 0; i <<= 1) {

        releaseLine();
        delayUs(2);

        lowLine(); // Start time slot

        if ((data & i) == 0) {
            // Write '0'
            delayUs(70);
            releaseLine();
            delayUs(2);
        } else {
            // Write '1'
            delayUs(10);
            releaseLine();
            delayUs(55);
        }
    }
}

void OneWire::write(const uint8_t* data, uint16_t size) {
    while (size--) {
        write(*data++);
    }
}

uint8_t OneWire::read() {
    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++) {

        releaseLine();
        delayUs(2);

        lowLine();
        delayUs(3);  // Start read slot

        releaseLine();
        delayUs(10); // Wait before sampling

        data |= (readLine() != 0) << i;

        delayUs(55); // End of timeslot
    }

    return data;
}

void OneWire::read(uint8_t* data, uint16_t size) {
    while (size--) {
        *data++ = read();
    }
}

void OneWire::delayMs(uint16_t ms) {
    while (ms--) {
        delayUs(1000);
    }
}

