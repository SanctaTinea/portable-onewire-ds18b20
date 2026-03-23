#include <stdint.h>
#include <stdio.h>
#include "OneWire.h"

/* --- GPIO configuration --- */

#define ONEWIRE_PORT   GPIOA
#define ONEWIRE_PIN   OneWire_Pin

static void GPIO_OpenDrainSetup() {
	GPIOA->BSRR = ONEWIRE_PIN << 16;
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = ONEWIRE_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* Drive the GPIO line to a low level */
static void lowLine() {
    GPIOA->BSRR = ONEWIRE_PIN << 16;
}

/* Release the GPIO line (set to high-impedance state).
   The line will be pulled high by an external pull-up resistor. */
static void releaseLine() {
    GPIOA->BSRR = ONEWIRE_PIN;
}

/* Read the current GPIO line level.
   Returns 0 if the line is low.
   Returns a non-zero value if the line is high. */
static uint8_t readLine() {
    return GPIOA->IDR & ONEWIRE_PIN;
}

/* Delay for the specified number of microseconds.
   Assumes the timer has at least 10-bit resolution.
   Timer must already be initialized.                */
static void delayUs(uint16_t us) {
    // Timer reset
    __HAL_TIM_SET_COUNTER(&htim11, 0);
    // Delay
    while (__HAL_TIM_GET_COUNTER(&htim11) < us) {}
}

/* --- Main example --- */
int main(void)
{
    /* HAL initialization and system clock configuration are omitted for brevity */

    // Config GPIO as openDrain
    GPIO_OpenDrainSetup();
    // Start the timer for delayUs
    HAL_TIM_Base_Start(&htim11);

    // Create OneWire instance
    OneWire ow;
    // Feed ow with prepared functions
    onewire_init(&ow, lowLine, releaseLine, readLine, delayUs);

    // Create DS18B20 instances
    uint8_t DS_ROM_1[8] = {0x28, 0xAA, 0x61, 0x51, 0x21, 0x20, 0x03, 0x1A};
    uint8_t DS_ROM_2[8] = {0x28, 0x24, 0xF2, 0x96, 0xF0, 0x01, 0x3C, 0xED};
    uint8_t DS_ROM_3[8] = {0x28, 0x30, 0x06, 0x96, 0xF0, 0x01, 0x3C, 0xD3};
    DS18b20 ds1, ds2, ds3;
    ds18b20_init_with_rom(&ds1, &ow, DS_ROM_1);
    ds18b20_init_with_rom(&ds2, &ow, DS_ROM_2);
    ds18b20_init_with_rom(&ds3, &ow, DS_ROM_3);
    // Temperature variables
    float dsTemp1, dsTemp2, dsTemp3;

    // Start conversion on the first time
    ds18b20_convert_temp(&ds1);
    ds18b20_convert_temp(&ds2);
    ds18b20_convert_temp(&ds3);

    // Time we need to wait before read
    uint32_t dsTimeMark = ds18b20_get_converting_time(&ds1) + HAL_GetTick();

    while (1)
    {
        // Check if new valid data is ready
        if (dsTimeMark < HAL_GetTick()) {
            // Read the temperature
            if (ds18b20_read_temp(&ds1, &dsTemp1)) { printf("Temperature (ds1):  %.6f C\r\n", dsTemp1); };
            else { printf("DS18B20 #1 read failed\r\n"); }
            if (ds18b20_read_temp(&ds2, &dsTemp2)) { printf("Temperature (ds2):  %.6f C\r\n", dsTemp2); };
            else { printf("DS18B20 #2 read failed\r\n"); }
            if (ds18b20_read_temp(&ds3, &dsTemp3)) { printf("Temperature (ds3):  %.6f C\r\n", dsTemp3); };
            else { printf("DS18B20 #3 read failed\r\n"); }

            // Start conversion again
            ds18b20_convert_temp(&ds1);
            ds18b20_convert_temp(&ds2);
            ds18b20_convert_temp(&ds3);

            // Time we need to wait before read
            dsTimeMark = ds18b20_get_converting_time(&ds1) + HAL_GetTick();
        }
    }
}