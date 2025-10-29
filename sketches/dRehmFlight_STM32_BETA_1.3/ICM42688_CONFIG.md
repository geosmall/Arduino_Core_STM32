# ICM‑42688‑P Filter & ODR Guidance

This document explains the **two filter blocks** in the ICM‑42688‑P, how they relate to **ODR** (Output Data Rate), and provides a **clear power‑up init recipe** for drone flight controllers targeting a **2 kHz PID loop** with **Gyro ODR = 4 kHz** and **Accel ODR = 1 kHz**. It also includes a small tuning playbook.

> **TL;DR (recommended for FCs):**
>
> * **Gyro:** ODR **4 kHz**, **AAF ≈ 258 Hz**, **UI filter = 1st‑order, BW = ODR/2 (wide)**.
> * **Accel:** ODR **1 kHz**, **AAF ≈ 170 Hz**, **UI filter = 1st‑order, BW = ODR/2 (wide)**.
> * Keep full‑scale ranges per your needs (±2000 dps, ±16 g typical).

---

## 1) The two filter blocks & data path

**AAF (Anti‑Alias Filter)**

* Purpose: Front‑end low‑pass that prevents aliasing **before/with decimation**.
* Programmed via three parameters per preset: **DELT**, **DELTSQR**, **BITSHIFT** (picked from a datasheet table by target 3 dB BW).
* Same preset table applies to **gyro** and **accel** (different registers/bit placements).

**UI (User Interface) Filter Block**

* Purpose: Post‑processing low‑pass that shapes the **delivered UI sample**.
* Select **order** (1st/2nd/3rd) and a **bandwidth code** (which maps to a 3 dB BW based on ODR and order).
* Useful to add or remove extra smoothing without changing ODR.

**Why use both?**

* AAF sets the **real corner** that protects against aliasing at the sensor front end.
* UI filter should be set **wide** (or minimally intrusive) when you want the AAF to define behavior (MPU‑6000 “DLPF 260/256‑ish” feel), or tightened later if the airframe is noisy.

---

## 2) Power‑on defaults vs recommended “wide” UI

**Power‑on defaults (LN mode):**

* **UI order:** 2nd order (gyro & accel).
* **UI BW code:** 1 (≈ `max(400 Hz, ODR)/4`).

  * At **Gyro ODR 4 kHz** → UI ≈ **1000 Hz**.
  * At **Accel ODR 1 kHz** → UI ≈ **250 Hz**.

**Recommended for FC use (“wide” UI):**

* **UI order:** **1st order**.
* **UI BW code:** **0** (= **ODR/2**).

  * At **Gyro ODR 4 kHz** → UI ≈ **2000 Hz** (well above the AAF corner).
  * At **Accel ODR 1 kHz** → UI ≈ **500 Hz** (above the AAF corner).

**Why “wide”?**

* Keeps the UI filter from adding phase/extra roll‑off in the control band. The **AAF (≈258/170 Hz)** then becomes the dominant corner, closely matching the “wide” feel of MPU‑6000 DLPF 260/256 while ensuring proper anti‑aliasing.

---

## 3) Concrete settings (baseline)

### Gyro (rate PID inner loop critical)

* **ODR:** 4 kHz
* **Full‑scale:** as needed (±2000 dps common)
* **AAF target:** ≈ **258 Hz**

  * Params: **DELT = 6**, **DELTSQR = 36 (0x24)**, **BITSHIFT = 10**
* **UI filter:** **1st order**, **BW code = 0 (ODR/2)** → ~2000 Hz

### Accel (outer/leveling; less bandwidth needed)

* **ODR:** 1 kHz
* **Full‑scale:** as needed (±16 g common)
* **AAF target:** ≈ **170 Hz**

  * Params: **DELT = 4**, **DELTSQR = 16 (0x10)**, **BITSHIFT = 11**
* **UI filter:** **1st order**, **BW code = 0 (ODR/2)** → ~500 Hz

---

## 4) Registers you need to touch (addresses & fields)

**Common**

* **`REG_BANK_SEL` (0x76):** Select bank (0 = normal, 1 = gyro AAF static, 2 = accel AAF static).
* **`PWR_MGMT0` (0x4E, Bank 0):** Set **GYRO_MODE=LN (11)**, **ACCEL_MODE=LN (11)**.
* **`GYRO_CONFIG0` (0x4F, Bank 0):** Set **GYRO_ODR** (4 kHz) and **GYRO_FS_SEL**.
* **`ACCEL_CONFIG0` (0x50, Bank 0):** Set **ACCEL_ODR** (1 kHz) and **ACCEL_FS_SEL**.

**Gyro AAF (Bank 1)**

* **`GYRO_CONFIG_STATIC2` (0x0B):** bit1 **GYRO_AAF_DIS** (0 = enabled). Leave enabled.
* **`GYRO_CONFIG_STATIC3` (0x0C):** **GYRO_AAF_DELT** [5:0].
* **`GYRO_CONFIG_STATIC4` (0x0D):** **GYRO_AAF_DELTSQR** [7:0].
* **`GYRO_CONFIG_STATIC5` (0x0E):** bits7:4 **GYRO_AAF_BITSHIFT**, bits3:0 **GYRO_AAF_DELTSQR** [11:8].

**Accel AAF (Bank 2)**

* **`ACCEL_CONFIG_STATIC2` (0x03):** bit0 **ACCEL_AAF_DIS** (0 = enabled); bits6:1 **ACCEL_AAF_DELT**.
* **`ACCEL_CONFIG_STATIC3` (0x04):** **ACCEL_AAF_DELTSQR** [7:0].
* **`ACCEL_CONFIG_STATIC4` (0x05):** bits7:4 **ACCEL_AAF_BITSHIFT**, bits3:0 **ACCEL_AAF_DELTSQR** [11:8].

**UI Filter (Bank 0)**

* **`GYRO_CONFIG1` (0x51):** bits3:2 **GYRO_UI_FILT_ORD** (00=1st, 01=2nd, 10=3rd).
* **`ACCEL_CONFIG1` (0x53):** bits4:3 **ACCEL_UI_FILT_ORD** (00=1st, 01=2nd, 10=3rd).
* **`GYRO_ACCEL_CONFIG0` (0x52):** bits3:0 **GYRO_UI_FILT_BW**; bits7:4 **ACCEL_UI_FILT_BW** (code 0 = ODR/2, code 1 ≈ max(400, ODR)/4, etc.).

---

## 5) Minimal init sequence (write order)

1. **Bank 0:** Put sensors in **LN** and set **ODRs/FSRs**

   * `PWR_MGMT0`: GYRO_MODE=11, ACCEL_MODE=11
   * `GYRO_CONFIG0`: ODR=4 kHz, FS_SEL as desired
   * `ACCEL_CONFIG0`: ODR=1 kHz, FS_SEL as desired

2. **Bank 1 (Gyro AAF = 258 Hz)**

   * `REG_BANK_SEL` = 1
   * `GYRO_CONFIG_STATIC2`: ensure bit1=0 (enable AAF)
   * `GYRO_CONFIG_STATIC3` = **0x06** (DELT)
   * `GYRO_CONFIG_STATIC4` = **0x24** (DELTSQR[7:0])
   * `GYRO_CONFIG_STATIC5` = **0xA0** (BITSHIFT=10 → 0xA<<4; high nibble of DELTSQR = 0)

3. **Bank 2 (Accel AAF = 170 Hz)**

   * `REG_BANK_SEL` = 2
   * `ACCEL_CONFIG_STATIC2`: bits6:1 = **0b00100** (DELT=4), bit0=0 (enable)
   * `ACCEL_CONFIG_STATIC3` = **0x10** (DELTSQR[7:0])
   * `ACCEL_CONFIG_STATIC4` = **0xB0** (BITSHIFT=11 → 0xB<<4; high nibble of DELTSQR = 0)

4. **Bank 0 (UI wide, 1st‑order)**

   * `REG_BANK_SEL` = 0
   * `GYRO_CONFIG1`: bits3:2 = **00** (1st order)
   * `ACCEL_CONFIG1`: bits4:3 = **00** (1st order)
   * `GYRO_ACCEL_CONFIG0`: bits3:0 = **0000** (gyro BW = ODR/2), bits7:4 = **0000** (accel BW = ODR/2)

---

## 6) Optional: default‑UI alternative

If you prefer to leave UI at **power‑on defaults** (2nd order; BW code 1):

* Gyro UI at 4 kHz ODR → ~1000 Hz (well above 258 Hz) → AAF still dominates.
* Accel UI at 1 kHz ODR → ~250 Hz (above 170 Hz) → AAF still dominates, with slightly more post‑AAF smoothing.
* You can switch to the **wide** recipe later if the craft feels overly damped near the top of your control band.

---

## 7) Tuning playbook (field‑proven)

* Start with the baseline above.
* **If motors/ESCs sing, D‑term noisy:** step gyro AAF down one notch (≈213 Hz → 170 Hz). Consider softer mounting.
* **If craft feels sluggish but clean:** step gyro AAF up (≈303 Hz). Keep UI as 1st‑order wide.
* **Accel too twitchy / noisy in level mode:** drop accel AAF to 126–170 Hz range.
* **Switching ODRs (1k/2k/4k):** keep AAF target ≤ ~0.45×ODR and UI at ODR/2 (wide) so AAF remains the dominant corner.

---

## 8) Small helper API (optional design)

* Maintain **one LUT** of `{freq, DELT, DELTSQR, BITSHIFT}` shared by gyro/accel.
* Provide `setGyroAafHz(target, odr)` / `setAccelAafHz(target, odr)` that clamp vs ODR, pick nearest preset, and write the appropriate bank/registers.
* Provide `setUiWide1stOrder()` that sets UI order=1 and BW code=0 for both sensors.

This keeps init readable and avoids bit‑twiddling at call sites.

---

### Notes

* The AAF preset table (42 → ~3979 Hz) is the **same** for gyro & accel; only register locations differ.
* For MPU‑6000 parity (DLPF 260/256 feel), letting **AAF define the corner** and keeping **UI wide** is the closest analog while benefiting from the ICM‑42688‑P’s superior front‑end filtering.
