# OneWire + DS18B20 (C and C++)

Lightweight, hardware-agnostic **OneWire bit-banging driver** and **DS18B20 temperature sensor library** for embedded systems.

## ✨ Features

* ✅ Hardware-independent
* ✅ Clean separation:

    * `OneWire` — low-level bus driver
    * `DS18b20` — device-level driver
* ✅ Supports:

    * Reset / presence detection
    * Read / write bytes
    * DS18B20 temperature conversion
    * CRC validation
    * Configurable resolution (9–12 bit)
* ✅ Works with:

    * Single device (SKIP ROM)
    * Multiple devices (MATCH ROM)

---

## ⚙️ Requirements

* Any microcontroller (tested with STM32)
* GPIO configured as **open-drain**
* External pull-up resistor (~4.7kΩ recommended)
* Accurate microsecond delay function

---

## 🚀 Quick Start

### 1. Provide hardware functions

You must implement 4 low-level functions:

```cpp
// Both C and C++

// Drive the GPIO line to a low level
void lowLine() {
    // Example implementation
    GPIOA->BSRR = ONEWIRE_PIN << 16;
}

// Release the GPIO line (set to high-impedance state).
// The line will be pulled high by an external pull-up resistor.
void releaseLine() {
    // Example implementation
    GPIOA->BSRR = ONEWIRE_PIN;
}

// Read the current GPIO line level.
// Returns 0 if the line is low.
// Returns a non-zero value if the line is high.
uint8_t readLine() {
    // Example implementation
    return GPIOA->IDR & ONEWIRE_PIN;
}

// Delay for the specified number of microseconds.
// Assumes the timer has at least 10-bit resolution.
void delayUs(uint16_t us) {
    // Example implementation
    __HAL_TIM_SET_COUNTER(&htim11, 0);
    HAL_TIM_Base_Start(&htim11);
    while (__HAL_TIM_GET_COUNTER(&htim11) < us) {}
}
```

---

### 2. Create OneWire instance

```cpp
// C++
OneWire ow(lowLine, releaseLine, readLine, delayUs);

// C
OneWire ow;
onewire_init(&ow, lowLine, releaseLine, readLine, delayUs);
```

---

### 3. Create DS18B20 instance

#### Single device on bus:

```cpp
// C++
DS18b20 sensor(ow);

// C
DS18b20 ds;
ds18b20_init(&ds, &ow);
```

#### Multiple devices (with ROM):

```cpp
// C++
uint8_t DS_ROM_1[8] = { /* device ROM */ };
DS18b20 sensor_1(ow, DS_ROM_1);

// C
uint8_t DS_ROM_1[8] = { /* device ROM */ };
DS18b20 ds1;
ds18b20_init_with_rom(&ds1, &ow, DS_ROM_1);
```

---

### 4. Read temperature

#### Blocking mode:

```cpp
// C++
float temp;
if (sensor.readTempBlocking(&temp)) {
    // temp contains value in °C
}

// C
float temp;
if (ds18b20_read_temp_blocking(&ds, &temp)) {
  // temp contains value in °C
}
```

#### Non-blocking mode:

```cpp
sensor.convertTemp();      // C++
ds18b20_convert_temp(&ds); // C

uint32_t timemark = sensor.getConvertingTime() + timeInMillisExample();

// do something ...

if (timeInMillisExample() > timemark) {
    // The data are ready
    float temp;
    
    if (!sensor.readTemp(&temp)) { doSomething() };        // C++
    if (!ds18b20_read_temp(&ds, &temp)) { doSomething() }; // C
}
```

You can look at the examples in the `examples/` directory.

---

## 🌡️ Resolution & Timing

| Resolution | Max Conversion Time |
| ---------- |---------------------|
| 9-bit      | < 94 ms             |
| 10-bit     | < 188 ms            |
| 11-bit     | < 375 ms            |
| 12-bit     | < 750 ms            |

Set resolution:

```cpp
sensor.setResolution(DS18b20Resolution::Bits12);
```

---


## ⚠️ Important Notes

### 🔌 Open-Drain Requirement

The bus **must NOT be driven HIGH actively**.

Correct behavior:

* LOW → actively pull line down
* HIGH → release line (external pull-up)

---

### ⏱ Timing Sensitivity

OneWire protocol is timing-critical:

* Interrupts may break communication
* Use accurate `delayUs()`
* Avoid long ISR execution during transactions

---

## 📌 Limitations

* No ROM search implementation
* No EEPROM save
* No parasite power handling

---

## 🔧 Possible Improvements

* [ ] Add ROM search algorithm
* [ ] Add EEPROM save
* [ ] Add parasite power handling
