# Nested Arduino Library Pattern (Umbrella → Devices)

This doc shows how to structure **one Arduino library** that supports multiple IMUs (or other parts) without “true” nested libraries. Works in Arduino IDE and Arduino CLI.

## Goals

* One installable library; many devices inside it
* Clean public API with auto-detect
* Shared bus + utils in `src/common` and `src/bus`
* No Arduino-unfriendly nested `library.properties`

---

## Folder Layout

```
MyIMU/
  library.properties
  src/
    MyIMU.h
    MyIMU.cpp
    common/
      Types.h
      Utils.h
    bus/
      Bus.h
      I2CBus.h
      I2CBus.cpp
      SPIBus.h
      SPIBus.cpp
    devices/
      DeviceBase.h
      ICM42688.h
      ICM42688.cpp
      MPU6000.h
      MPU6000.cpp
  examples/
    DetectAndInit/
      DetectAndInit.ino
```

**Key rule:** Arduino adds `src/` to include paths. Inside the lib, use `#include "devices/ICM42688.h"` (relative to `src/`). Sketches include only `#include <MyIMU.h>`.

---

## `library.properties` (minimal)

```ini
name=MyIMU
version=0.1.0
author=Your Name
maintainer=Your Name <you@example.com>
sentence=Unified IMU driver (ICM42688, MPU6000, ...)
paragraph=Single umbrella library with per-device drivers and auto-detect.
category=Sensors
architectures=*
includes=MyIMU.h
```

---

## Public Facade (`src/MyIMU.h`)

```cpp
#pragma once
#include <stdint.h>

enum class ImuType : uint8_t {
  Auto = 0,
  ICM42688,
  MPU6000,
  // add more here...
};

struct ImuSample {
  float ax, ay, az;
  float gx, gy, gz;
  uint32_t timestamp_us;
};

class MyIMU {
public:
  // Call before begin(): set bus type/params
  void attachI2C(TwoWire& wire, uint8_t addr);
  void attachSPI(SPIClass& spi, uint8_t csPin);

  // Auto tries WHO_AM_I on attached bus(es) unless type != Auto
  bool begin(ImuType type = ImuType::Auto);

  bool read(ImuSample& out);      // fused access (gyro+accel)
  bool readGyro(float& x, float& y, float& z);
  bool readAccel(float& x, float& y, float& z);

  ImuType type() const;

private:
  // pimpl to avoid leaking device headers to users
  class Impl;
  Impl* impl_ = nullptr;
};
```

### Facade (`src/MyIMU.cpp`) – essentials

```cpp
#include "MyIMU.h"
#include "bus/I2CBus.h"
#include "bus/SPIBus.h"
#include "devices/DeviceBase.h"
#include "devices/ICM42688.h"
#include "devices/MPU6000.h"
#include "common/Utils.h"

class MyIMU::Impl {
public:
  std::unique_ptr<Bus> bus;                // I2CBus or SPIBus
  std::unique_ptr<DeviceBase> dev;         // concrete device
  ImuType detected = ImuType::Auto;
};

void MyIMU::attachI2C(TwoWire& wire, uint8_t addr) {
  if (!impl_) impl_ = new Impl();
  impl_->bus = makeI2CBus(wire, addr);
}

void MyIMU::attachSPI(SPIClass& spi, uint8_t csPin) {
  if (!impl_) impl_ = new Impl();
  impl_->bus = makeSPIBus(spi, csPin);
}

static std::unique_ptr<DeviceBase> tryMakeDevice(Bus& bus, ImuType force) {
  if (force == ImuType::ICM42688 || force == ImuType::Auto) {
    if (ICM42688::probe(bus)) return std::make_unique<ICM42688>(bus);
  }
  if (force == ImuType::MPU6000 || force == ImuType::Auto) {
    if (MPU6000::probe(bus)) return std::make_unique<MPU6000>(bus);
  }
  return nullptr;
}

bool MyIMU::begin(ImuType type) {
  if (!impl_ || !impl_->bus) return false;
  impl_->dev = tryMakeDevice(*impl_->bus, type);
  if (!impl_->dev) return false;
  impl_->detected = impl_->dev->kind();
  return impl_->dev->begin();
}

bool MyIMU::read(ImuSample& s)           { return impl_ && impl_->dev && impl_->dev->read(s); }
bool MyIMU::readGyro(float& x,float& y,float& z){ return impl_ && impl_->dev && impl_->dev->readGyro(x,y,z); }
bool MyIMU::readAccel(float& x,float& y,float& z){ return impl_ && impl_->dev && impl_->dev->readAccel(x,y,z); }
ImuType MyIMU::type() const              { return impl_ ? impl_->detected : ImuType::Auto; }
```

---

## Bus Abstraction (lightweight)

`src/bus/Bus.h`

```cpp
#pragma once
#include <stdint.h>
#include <stddef.h>

class Bus {
public:
  virtual ~Bus() = default;
  virtual bool writeReg(uint8_t reg, const uint8_t* data, size_t len) = 0;
  virtual bool readReg(uint8_t reg, uint8_t* data, size_t len) = 0;
  virtual bool writeBits(uint8_t reg, uint8_t mask, uint8_t value) = 0;
};
```

`src/bus/I2CBus.h/.cpp` and `src/bus/SPIBus.h/.cpp` implement those methods (simple, blocking). Factory helpers:

```cpp
// I2CBus.h
#pragma once
#include <Wire.h>
#include "Bus.h"
std::unique_ptr<Bus> makeI2CBus(TwoWire& wire, uint8_t addr);

// SPIBus.h
#pragma once
#include <SPI.h>
#include "Bus.h"
std::unique_ptr<Bus> makeSPIBus(SPIClass& spi, uint8_t csPin);
```

---

## Per-Device Interface

`src/devices/DeviceBase.h`

```cpp
#pragma once
#include "../common/Types.h"
#include "../bus/Bus.h"
#include "../MyIMU.h"

class DeviceBase {
public:
  explicit DeviceBase(Bus& b) : bus_(b) {}
  virtual ~DeviceBase() = default;
  virtual bool begin() = 0;
  virtual bool read(ImuSample& s) = 0;
  virtual bool readGyro(float& x, float& y, float& z) = 0;
  virtual bool readAccel(float& x, float& y, float& z) = 0;
  virtual ImuType kind() const = 0;
protected:
  Bus& bus_;
};
```

### Example: ICM-42688

`src/devices/ICM42688.h`

```cpp
#pragma once
#include "DeviceBase.h"

class ICM42688 : public DeviceBase {
public:
  using DeviceBase::DeviceBase;
  static bool probe(Bus& bus);  // check WHO_AM_I
  bool begin() override;
  bool read(ImuSample& s) override;
  bool readGyro(float& x,float& y,float& z) override;
  bool readAccel(float& x,float& y,float& z) override;
  ImuType kind() const override { return ImuType::ICM42688; }
};
```

`src/devices/ICM42688.cpp` (sketch)

```cpp
#include "ICM42688.h"
#include "../common/Utils.h"

static constexpr uint8_t REG_WHOAMI = 0x75;
static constexpr uint8_t WHOAMI_OK  = 0x47; // example

bool ICM42688::probe(Bus& bus) {
  uint8_t v=0;
  if (!bus.readReg(REG_WHOAMI, &v, 1)) return false;
  return v == WHOAMI_OK;
}

bool ICM42688::begin() {
  // reset, config ODR, full-scale, filters...
  return true;
}

bool ICM42688::read(ImuSample& s) {
  // burst read accel+gyro registers, scale to SI
  return true;
}

bool ICM42688::readGyro(float& x,float& y,float& z)  { ImuSample s; if(!read(s)) return false; x=s.gx;y=s.gy;z=s.gz; return true; }
bool ICM42688::readAccel(float& x,float& y,float& z) { ImuSample s; if(!read(s)) return false; x=s.ax;y=s.ay;z=s.az; return true; }
```

Add other devices (e.g., `MPU6000`) with the same pattern and their own `probe()`.

---

## Example Sketch

`examples/DetectAndInit/DetectAndInit.ino`

```cpp
#include <MyIMU.h>

MyIMU imu;

void setup() {
  Serial.begin(115200);

  Wire.begin();             // if using I2C
  imu.attachI2C(Wire, 0x68);

  // SPI example:
  // SPI.begin();
  // imu.attachSPI(SPI, /*csPin=*/10);

  if (!imu.begin(ImuType::Auto)) {
    Serial.println("IMU init failed");
    while (1) {}
  }

  Serial.print("Detected: ");
  switch (imu.type()) {
    case ImuType::ICM42688: Serial.println("ICM42688"); break;
    case ImuType::MPU6000:  Serial.println("MPU6000");  break;
    default:                Serial.println("Unknown");   break;
  }
}

void loop() {
  ImuSample s;
  if (imu.read(s)) {
    Serial.print("G[deg/s] "); Serial.print(s.gx); Serial.print(", ");
    Serial.print(s.gy); Serial.print(", "); Serial.println(s.gz);
  }
  delay(5);
}
```

---

## Build & Include Notes

* **Sketch includes** only `#include <MyIMU.h>`.
* Inside the library, use **relative includes from `src/` root** (e.g., `"devices/ICM42688.h"`).
* **Arduino CLI install (local dev):**

  ```bash
  arduino-cli lib uninstall MyIMU 2>/dev/null || true
  arduino-cli lib install --zip-path /path/to/MyIMU.zip
  # or symlink/copy MyIMU/ into <Sketchbook>/libraries/
  ```

---

## Extending With New Devices

1. Add `devices/YourChip.h/.cpp` implementing `probe()`, `begin()`, `read()`.
2. Include its header in `MyIMU.cpp` and extend `tryMakeDevice()`.
3. Add your WHO_AM_I and config sequences.
4. Update examples if needed.

---

## Gotchas

* **No nested libraries:** Don’t put `library.properties` under `devices/`. Only the top-level one counts.
* **Name collisions:** Use namespaces inside device .cpps or keep symbols `static` where possible.
* **Code size:** If it grows, add `#define MYIMU_ENABLE_XYZ 0/1` toggles in a `MyIMU_Config.h` and guard `tryMakeDevice()` branches.

---

## Testing Tips

* Provide a `SelfTest` example that:

  * runs `probe()` on each device class,
  * prints WHO_AM_I values,
  * dumps a few samples at multiple ODRs.

---

**That’s it.** Drop this pattern in and you’ll have a single clean library that supports multiple IMUs with auto-detect, no “nested” library hacks, and a stable facade API.
