# Betaflight × ICM‑42688‑P — Hardware Filtering (AAF + UI)

This document reflects how **Betaflight** actually configures and uses the ICM‑42688‑P’s **hardware filters** and related registers, based on reading the BF driver (`accgyro_spi_icm426xx.c/.h`). It is intended as a precise reference for parity and for informed deviations.

---

## 1) What Betaflight does (high‑level)

* **Gyro ODR:** Commonly 8 kHz on capable targets (4/2/1 kHz on others).
* **FSR:** Gyro ±2000 dps, Accel ±16 g.
* **Hardware anti‑alias (AAF):** Enabled and set near **~258 Hz** for gyro; BF also sets **accel AAF ≈ 258 Hz** (note: this differs from some FC guidance that prefers ≈170 Hz for accel).
* **UI filter (post‑AAF LPF):** **Explicitly set to low‑latency path** using **BW code = 15** for both gyro and accel; **filter order registers are left at reset (2nd‑order)**. This makes the UI stage effectively very wide / minimal‑delay relative to the control band; AAF does the real front‑end shaping.

Why: BF relies primarily on **software filters** (gyro PT1/biquad, D‑term filters, dynamic notch/RPM) and wants the sensor hardware to provide solid anti‑aliasing but **minimal extra delay**.

---

## 2) Registers programmed by BF

### Common control / ODR / FSR (Bank 0)

* `PWR_MGMT0 (0x4E)`: sequence to reset, power down/up, and place **GYRO_MODE/ACCEL_MODE = LN**.
* `GYRO_CONFIG0 (0x4F)`: sets **GYRO_ODR** via a LUT (8k→3, 4k→4, 2k→5, 1k→6) and **GYRO_FS_SEL** (±2000 dps typical).
* `ACCEL_CONFIG0 (0x50)`: sets **ACCEL_ODR** and **ACCEL_FS_SEL** (±16 g typical).

### AAF — Gyro (Bank 1)

* `REG_BANK_SEL` → **1**
* `GYRO_CONFIG_STATIC2 (0x0B)`: **GYRO_AAF_DIS bit = 0** (enable AAF).
* `GYRO_CONFIG_STATIC3 (0x0C)`: **GYRO_AAF_DELT**.
* `GYRO_CONFIG_STATIC4 (0x0D)`: **GYRO_AAF_DELTSQR[7:0]**.
* `GYRO_CONFIG_STATIC5 (0x0E)`: **GYRO_AAF_BITSHIFT[7:4]** + **GYRO_AAF_DELTSQR[11:8]**.

### AAF — Accel (Bank 2)

* `REG_BANK_SEL` → **2**
* `ACCEL_CONFIG_STATIC2 (0x03)`: **ACCEL_AAF_DIS bit = 0**; **ACCEL_AAF_DELT in bits 6:1** (i.e., `DELT << 1`).
* `ACCEL_CONFIG_STATIC3 (0x04)`: **ACCEL_AAF_DELTSQR[7:0]**.
* `ACCEL_CONFIG_STATIC4 (0x05)`: **ACCEL_AAF_BITSHIFT[7:4]** + **ACCEL_AAF_DELTSQR[11:8]**.

### UI filter — Gyro & Accel (Bank 0)

* `REG_BANK_SEL` → **0**
* **UI bandwidth codes:** `GYRO_ACCEL_CONFIG0 (0x52)`

  * **Gyro UI BW:** bits **[3:0] = 15**
  * **Accel UI BW:** bits **[7:4] = 15**
    Code **15** selects the **low‑latency (trivial decimation) path**, effectively very wide compared with AAF.
* **UI filter order:** left at reset defaults (no explicit write by BF):

  * `GYRO_CONFIG1 (0x51)`, bits **[3:2] = 01** → **2nd‑order**.
  * `ACCEL_CONFIG1 (0x53)`, bits **[4:3] = 01** → **2nd‑order**.

### Other IMU interface details BF touches

* Interrupt configuration and data‑ready timing.
* Disables AFSR in `INTF_CONFIG1` to avoid output stalls.

---

## 3) AAF presets BF uses

Betaflight selects from a small set of **AAF presets** via a LUT (chip‑specific). Typical entries used on ICM‑42688‑P include:

| 3 dB BW (Hz) | DELT |     DELTSQR | BITSHIFT |
| -----------: | ---: | ----------: | -------: |
|      **258** |    6 |   36 (0x24) |       10 |
|      **536** |   12 |  144 (0x90) |        8 |
|      **997** |   21 | 440 (0x1B8) |        6 |
|     **1962** |   28 | 784 (0x310) |        5 |

Notes:

* The *same* triplet format applies to **gyro** and **accel** AAF; only the register locations differ.
* BF writes **gyro AAF ≈ 258 Hz** and **accel AAF ≈ 258 Hz** by default in the 42688P driver (i.e., accel is not set lower by default in BF).

---

## 4) How this interacts with BF’s software filters

* **AAF** defines the **front‑end corner** to prevent aliasing before the data hits UI/software.
* **UI filter** is put in **low‑latency (code 15)** to keep its effect minimal at typical ODRs; it contributes little additional roll‑off or delay within the gyro control band.
* Main shaping is done in **software** (gyro LPF(s), D‑term filters, dynamic notch, RPM filtering). This division of labor is why BF picks a **modest AAF** and then keeps UI effectively open.

---

## 5) Parity checklist (do this to match BF)

1. **Reset → LN mode** for gyro & accel.
2. **Set ODR** using BF’s LUT (8k→3, 4k→4, 2k→5, 1k→6).
3. **FSR:** gyro ±2000 dps, accel ±16 g.
4. **AAF:** program presets (e.g., **258 Hz**) for **both** gyro and accel.
5. **UI:** set `GYRO_ACCEL_CONFIG0` **BW code = 15** for both sensors; **leave order = 2nd‑order** (power‑on default).
6. Apply BF’s other interface bits (DRDY polarity/timing, disable AFSR).

---

## 6) Differences vs common FC guidance & rationale

* Many FC guides recommend **accel AAF ≈ 170 Hz**. **BF uses ≈ 258 Hz** for accel on 42688P—leaning on software to manage residual noise. If you prefer a quieter accel, you can drop to 170 Hz without breaking parity for gyro.
* Some guides call “UI wide” = **code 0 (ODR/2)**. **BF uses code 15 (low‑latency)**, which is even wider / lower‑delay. If you’re cloning BF behavior, prefer **code 15**.
* BF **does not change** UI filter **order**; it remains **2nd‑order** by reset. That’s fine because the **code‑15 path** dominates behavior at high ODR.

---

## 7) Ready‑to‑paste programming summary

* **Gyro AAF ≈ 258 Hz**
  Bank1: `0x0B`(enable AAF), `0x0C`=0x06, `0x0D`=0x24, `0x0E`=0xA0.
* **Accel AAF ≈ 258 Hz**
  Bank2: `0x03` bits6:1 = `0b000110` (DELT=6), bit0=0; `0x04`=0x24; `0x05`=0xA0.
* **UI low‑latency (both)**
  Bank0: `0x52` → set **bits[3:0]=15** (gyro), **bits[7:4]=15** (accel).
  Leave `0x51` and `0x53` (order fields) at defaults (2nd‑order).

---

## 8) When to deviate from BF defaults

* **Very noisy airframe / weak MCU:** Drop gyro AAF step(s) (→ 213/170 Hz), or tighten UI from code 15 → a real LPF code (accept extra delay).
* **High‑performance stiff build:** Keep BF defaults; consider higher AAF (→ 303/348 Hz) only if your software filtering and noise budget permit.

---

### Bottom line

To mirror Betaflight on ICM‑42688‑P: **set both AAFs ≈ 258 Hz, set UI BW code to 15 (low‑latency) for gyro & accel, leave UI order at reset (2nd‑order), and drive ODR/FSR via the standard LUT.** AAF performs the meaningful anti‑aliasing; the UI stage stays out of the way so BF’s software filters can do the fine work.
