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
// GPIO line to low level
void lowLine() {
    // as example
    GPIOA->BSRR = ONEWIRE_PIN << 16;
}

// GPIO line to Z-state (HIGH via pull-up)
void releaseLine() {
    // as example
    GPIOA->BSRR = ONEWIRE_PIN; // release (HIGH via pull-up)
}

// read the GPIO line level.
// It must be 0 if line has low level
// It must be any non-zero value if line has high level
uint8_t readLine() {
    // as example
    return GPIOA->IDR & ONEWIRE_PIN;
}

// just delay with us. 10 bit assumed at least.
void delayUs(uint16_t us) {
    // as example
    __HAL_TIM_SET_COUNTER(&htim11, 0);
    HAL_TIM_Base_Start(&htim11);
    while (__HAL_TIM_GET_COUNTER(&htim11) < us) {}
}
```

---

### 2. Create OneWire instance

```cpp
OneWire ow(lowLine, releaseLine, readLine, delayUs);
```

---

### 3. Create DS18B20 instance

#### Single device on bus:

```cpp
DS18b20 sensor(ow);
```

#### Multiple devices (with ROM):

```cpp
uint8_t rom_1[8] = { /* device ROM */ };
DS18b20 sensor_1(ow, rom_1);
```

---

### 4. Read temperature

#### Blocking mode:

```cpp
float temp;
if (sensor.readTempBlocking(&temp)) {
    // temp contains value in °C
}
```

#### Non-blocking mode:

```cpp
sensor.convertTemp();

// do something ...

float temp;
sensor.readTemp(&temp);
```

---

## 🌡️ Resolution & Timing

| Resolution | Max Conversion Time |
| ---------- |---------------------|
| 9-bit      | <94 ms              |
| 10-bit     | <188 ms             |
| 11-bit     | <375 ms             |
| 12-bit     | <750 ms             |

Set resolution:

```cpp
sensor.setResolution(DS18b20Resolution::Bits12);
```

---

## 🔐 CRC Protection

Library uses Dallas/Maxim CRC-8 to validate data:

* Scratchpad (9 bytes)
* Automatically checked in `readTemp()`

If CRC fails → function returns `false`.

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

### 🧵 Blocking Behavior

* `delayMs()` is busy-wait
* `readTempBlocking()` blocks CPU up to **750 ms**

For RTOS or complex systems:

* prefer non-blocking approach

---

## 📌 Limitations

* No ROM search implementation
* No EEPROM save
* No parasite power handling

---

## 🔧 Possible Improvements

* [ ] ROM search algorithm
* [ ] Add EEPROM save
