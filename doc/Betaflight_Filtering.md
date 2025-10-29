# Betaflight × ICM‑42688‑P — Hardware Filtering Notes

This note summarizes how Betaflight configures and *can* use the ICM‑42688‑P’s **hardware filters** (AAF + UI filter), and how that fits alongside Betaflight’s **software filtering**. It’s meant as a practical crib sheet while you port or compare against BF behavior.

---

## 1) Big picture

* **Betaflight relies primarily on software filters** (gyro low‑pass PT1/biquad, D‑term filters, dynamic notch, RPM filtering). Hardware filtering on the IMU is used conservatively: enough **anti‑aliasing** and minimal added delay.
* On ICM‑42688‑P specifically, BF community/devs settled on an **AAF around ~258 Hz** as a good default and leave the **UI filter wide** at high ODRs so the hardware UI filter doesn’t add extra lag inside the control band.
* Newer BF builds **recognize ICM‑42688‑P** (overflow protection, etc.). Some test branches exposed **CLI variables** to adjust AAF/UI quickly for experimentation.

---

## 2) Defaults you’ll typically see (ICM‑42688‑P)

> The following reflects common BF practice on 4.4/4.5 era + test branches and matches TDK’s recommended programming model.

**Gyro**

* **ODR:** often **8 kHz** on capable targets (lower on some MCUs/targets).
* **AAF (anti‑alias):** approx **258 Hz** (2nd‑order IIR implemented by DELT/DELTSQR/BITSHIFT) to keep front‑end noise bounded and avoid aliasing before decimation.
* **UI filter:** **kept wide** at high ODR (1st‑ or 2nd‑order, BW near **ODR/2**) so it doesn’t materially shape the passband. Practical effect at 8 kHz is minimal; BF’s software filters do the heavy lifting.

**Accel**

* Accel is generally read at a lower ODR and more heavily filtered in software; when configured, the **UI filter is also kept wide** and **AAF set lower (e.g., ~170 Hz)** to suppress high‑frequency vibration.

**Overflow protection**

* ICM‑42688‑P is treated as having **overflow/saturation protection**, so BF caps readings at FSR when saturated rather than wrapping.

---

## 3) Knobs Betaflight exposed during testing (mapping to registers)

Some development/testing builds added CLI variables that map directly to the ICM‑42688‑P registers (handy for bench validation). Even if you don’t use BF, mirror these names in your own CLI to stay familiar:

* **`gyro_delt`**  → **GYRO_AAF_DELT** (Bank 1, `GYRO_CONFIG_STATIC3`)
* **`gyro_deltsqr`** → **GYRO_AAF_DELTSQR** (Bank 1, `STATIC4`/`STATIC5` low+high bits)
* **`gyro_bitshift`** → **GYRO_AAF_BITSHIFT** (Bank 1, `STATIC5` bits7:4)
* **`gyro_ui_ord`** → **GYRO_UI_FILT_ORD** (Bank 0, `GYRO_CONFIG1` bits3:2; 0=1st, 1=2nd, 2=3rd)
* **`gyro_ui`** → **GYRO_UI_FILT_BW code** (Bank 0, `GYRO_ACCEL_CONFIG0` bits3:0)

**Typical defaults (gyro):**

* `gyro_delt=6`, `gyro_deltsqr=36`, `gyro_bitshift=10`  → **~258 Hz AAF**
* `gyro_ui_ord=1` (2nd order), `gyro_ui=<wide code>` at high ODR

> If you run **8 kHz ODR**, the **UI filter bandwidth codes** at “wide” settings resolve near **ODR/2**, which is already far above the control band; thus UI filter has little effect compared to AAF + software filters.

---

## 4) Register checklist (for parity with BF practice)

1. **ODR/FSR**

* `GYRO_CONFIG0` (Bank 0): set **ODR** (8k/4k/… per target), **FS_SEL** (±2000 dps typical).
* `ACCEL_CONFIG0` (Bank 0): set accel **ODR**/**FS_SEL** (often ≤1–2 kHz).

2. **AAF (front‑end anti‑alias)**

* **Gyro** (Bank 1): `GYRO_CONFIG_STATIC2` (enable AAF), `…STATIC3/4/5` (set **DELT/DELTSQR/BITSHIFT**). Use the **258 Hz** tuple above for baseline.
* **Accel** (Bank 2): `ACCEL_CONFIG_STATIC2/3/4` similarly; **170 Hz** is a good starting point.

3. **UI filter (post‑AAF shaper)**

* Bank 0: `GYRO_CONFIG1` (**order**), `ACCEL_CONFIG1` (**order**), `GYRO_ACCEL_CONFIG0` (**BW codes**). For high ODRs keep **1st or 2nd order** and **BW≈ODR/2**.

---

## 5) How this coexists with Betaflight’s software filtering

* **Hardware AAF** ≈ defines the *front‑end corner* and prevents aliasing.
* **Hardware UI** = dial it back (wide) so it doesn’t add group delay in the gyro control band when ODR is high.
* **Betaflight software filters** (gyro low‑pass, dynamic notch, RPM) then do the main cleanup based on the airframe. That’s where most tuning happens.

**Practical takeaway**

* Start with **AAF≈258 Hz (gyro)**, **≈170 Hz (accel)**. Keep **UI wide** at high ODRs.
* Only tighten hardware UI if you must reduce latency on the MCU (rare) or you run lower ODRs where UI bandwidth codes meaningfully change the passband.

---

## 6) Notes & gotchas

* At very **high ODRs (e.g., 8 kHz)**, changing UI BW codes may have **negligible effect** on the actual passband; check the TDK UI tables and confirm with logs/FFT.
* Ensure **clean power** and **separate LDO** for the IMU; many “ICM is noisy” reports were board‑power issues, not the sensor.
* BF 4.5+ treats ICM‑42688‑P as having **overflow protection**; make sure your stack doesn’t misinterpret saturated readings.

---

## 7) Porting recipe (to make your code BF‑familiar)

* Provide an IMU init option "**BF‑style (ICM‑42688‑P)**" that performs:

  * Gyro **ODR 8 kHz** (or 4 kHz on weaker targets), **FSR ±2000 dps**
  * Gyro **AAF ≈ 258 Hz**; Accel **AAF ≈ 170 Hz**
  * **UI order 1st–2nd**, **UI BW = ODR/2** for gyro/accel
* Offer a **CLI** mirroring BF test names (`gyro_delt`, `gyro_deltsqr`, `gyro_bitshift`, `gyro_ui_ord`, `gyro_ui`) and a small LUT to convert desired AAF Hz ↔ register triplet.
* Keep your **software filters** close to BF defaults so logs/tuning advice transfer cleanly.

---

### Quick reference (ICM‑42688‑P, key regs)

* **AAF Gyro:** Bank 1 `GYRO_CONFIG_STATIC2/3/4/5`
* **AAF Accel:** Bank 2 `ACCEL_CONFIG_STATIC2/3/4`
* **UI:** Bank 0 `GYRO_CONFIG1`, `ACCEL_CONFIG1`, `GYRO_ACCEL_CONFIG0`

> Tip: Log raw gyro FFTs at your target ODR with UI‑wide vs UI‑tighter to verify the UI codes actually move the measured 3 dB corner on your setup.
