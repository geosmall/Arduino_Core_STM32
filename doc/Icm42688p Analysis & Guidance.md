# ICM‑42688‑P — Combined Analysis & Practical Guidance (incl. Betaflight parity)

This is a reference for developers working with the ICM‑42688‑P on flight‑controller targets. It blends:

* an **analysis of how Betaflight (BF)** configures the chip (what, where, why), and
* a pragmatic **guidance section** with presets, rationale, and verification steps for this codebase.

It emphasizes **intent, choices, and trade‑offs**, with lightweight.


## 1) Signal path and roles

**AAF (Anti‑Alias Filter)**

* Location: front‑end, before/with decimation.
* Job: set the *real* bandwidth limit that prevents high‑frequency energy from folding into the band of interest.
* Control: choose an entry from a datasheet table `(DELT, DELTSQR, BITSHIFT)` that yields a specific 3 dB corner. Same table for gyro & accel; registers differ.

**UI filter (User‑Interface filter)**

* Location: after AAF/decimation; last stage the host sees.
* Job: shape **latency vs extra smoothing** for the delivered samples.
* Control: **order** (1st/2nd/3rd) + **bandwidth code** (maps to a corner relative to ODR). Includes a **low‑latency** option that is effectively very wide.

> Rule of thumb: **AAF sets the physics** (anti‑alias); **UI trims the feel** (latency vs. smoothness). If in doubt, make **AAF** your primary corner and keep **UI** minimally intrusive.


## 2) What Betaflight does in drivers/accgyro/accgyro_spi_icm426xx.c

* **Gyro ODR:** Commonly 8 kHz on capable targets (4/2/1 kHz on others).
* **FSR:** Gyro ±2000 dps, Accel ±16 g.
* **Hardware anti‑alias (AAF):** Enabled and set near **~258 Hz** for gyro; BF also sets **accel AAF ≈ 258 Hz** (note: this differs from some FC guidance that prefers ≈170 Hz for accel).
* **UI filter (post‑AAF LPF):** **Explicitly set to low‑latency path** using **BW code = 15** for both gyro and accel; **filter order registers are left at reset (2nd‑order)**. This makes the UI stage effectively very wide / minimal‑delay relative to the control band; AAF does the real front‑end shaping.

Why: BF relies primarily on **software filters** (gyro PT1/biquad, D‑term filters, dynamic notch/RPM) and wants the sensor hardware to provide solid anti‑aliasing but **minimal extra delay**.

## Registers programmed by BF

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
* **AFSR disable (workaround):** BF clears `INTF_CONFIG1` bits **[7:6]** and sets **0x40** to **disable AFSR** (undocumented vendor feature that otherwise can cause repeated/stuck gyro samples).

  * **Register:** Bank0 `INTF_CONFIG1 (0x4D)`
  * **Write:** `val = read(0x4D); val &= ~0xC0; val |= 0x40; write(0x4D, val);`
  * Apply early in init, before final ODR/FSR setup.


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


## 4) How this interacts with BF’s software filters

* **AAF** defines the **front‑end corner** to prevent aliasing before the data hits UI/software.
* **UI filter** is put in **low‑latency (code 15)** to keep its effect minimal at typical ODRs; it contributes little additional roll‑off or delay within the gyro control band.
* Main shaping is done in **software** (gyro LPF(s), D‑term filters, dynamic notch, RPM filtering). This division of labor is why BF picks a **modest AAF** and then keeps UI effectively open.


## 5) Parity checklist (do this to match BF)

1. **Reset → LN mode** for gyro & accel.
2. **Set ODR** using BF’s LUT (8k→3, 4k→4, 2k→5, 1k→6).
3. **FSR:** gyro ±2000 dps, accel ±16 g.
4. **AAF:** program presets (e.g., **258 Hz**) for **both** gyro and accel.
5. **UI:** set `GYRO_ACCEL_CONFIG0` **BW code = 15** for both sensors; **leave order = 2nd‑order** (power‑on default).
6. Apply BF’s other interface bits (DRDY polarity/timing, disable AFSR).


## 6) Our presets (use‑case driven)

Assume ICM‑42688‑P, Gyro ±2000 dps, Accel ±16 g. These cover 95% of builds.

| **Preset**              | **Gyro ODR** | **Accel ODR** |           **Gyro AAF** |                          **Accel AAF** | **UI order** |            **Gyro UI BW** | **Accel UI BW** | **When to pick**                                        |
| ----------------------- | -----------: | ------------: | ---------------------: | -------------------------------------: | -----------: | ------------------------: | --------------: | ------------------------------------------------------- |
| **Safe (bring‑up)**     |          1 k |           1 k |             **126 Hz** |                              **84 Hz** |          2nd |       **code 1** (≈ODR/4) |      **code 1** | First power‑on; slow props; unknown vibes; easy logging |
| **Balanced (default)**  |          2 k |           1 k |             **258 Hz** |                             **170 Hz** |          1st | **code 15** (low‑latency) |     **code 15** | General 2 kHz loops; clean frames; good feel vs. noise  |
| **Performance / Acro**  |          4 k |           1 k | **258 Hz** (or 303 Hz) | **170 Hz** (*or 258 Hz for BF parity*) |          1st |               **code 15** |     **code 15** | Stiff builds; strong software filtering; minimal lag    |
| **Quiet / Noisy frame** |          1 k |           1 k |             **170 Hz** |                             **126 Hz** |          2nd |                **code 1** |      **code 1** | Heavy props; soft mounts; prioritize calm signals       |

**Notes**

* Keep **AAF ≤ ~0.45×ODR** so anti‑aliasing holds when ODR changes.
* If motors “sing” or D‑term looks ragged → step **gyro AAF down** (258→213→170). If it feels sluggish but clean → **step gyro AAF up** (213→258→303) or keep UI in **code 15**.
* For strict **BF parity** on ICM‑42688‑P, use **Accel AAF = 258 Hz** (not 170 Hz).


## 7) Choosing policy per airframe (decision points)

1. **Loop & ODR**: start at Gyro 4 k / Accel 1 k for 2 kHz control; 8 k/4 k if the target and code budget allow.
2. **AAF primary corner**: pick the lowest value that doesn’t hurt feel. 170–303 Hz covers most cases; 258 Hz is a strong default.
3. **UI latency vs smoothness**: prefer **code 15** (low‑latency) when you trust software filters. Move to **code 0 (ODR/2)** or **code 1 (≈ODR/4)** only when you must add on‑chip smoothing.
4. **Accel goals**: level/loiter stability benefits from slightly lower accel AAF (e.g., 170 Hz) on smaller/noisier frames.


## 8) Verification & health checks (what to measure)

* **Readback audit**: dump all config registers once after init; keep as artifact.
* **DRDY/IO timing**: verify your SPI burst plus ISR jitter fits inside ODR (e.g., <250 µs at 4 kHz gyro).
* **Spectral check**: capture raw gyro FFT; confirm the AAF corner lands near the target and that UI switches (code 15 → code 1) behave as expected at high bands.
* **Aliasing probe**: excite above the AAF corner (e.g., motor tone sweep) and confirm attenuation rather than fold‑back in the band.
* **Flight signs**: noise‑driven whine → lower gyro AAF; sluggish feel → raise gyro AAF or widen UI; level‑mode jitter → reduce accel AAF.


## 9) Minimum you must do on init (independent of preset)

* **LN modes & ODR/FSR** for both sensors appropriate to loop targets.
* **AFSR off** early: clear bits [7:6] in `INTF_CONFIG1 (0x4D)`, set `0x40`. Prevents rare repeated samples seen in the field.
* **Bank discipline**: write AAF in the correct banks (gyro = Bank 1, accel = Bank 2). Always return to Bank 0 for UI settings and runtime.


## 10) Appendix — Register map you’ll actually use

**Common (Bank 0)**: `PWR_MGMT0 (0x4E)` LN mode; `GYRO_CONFIG0 (0x4F)` ODR/FS; `ACCEL_CONFIG0 (0x50)` ODR/FS; `GYRO_ACCEL_CONFIG0 (0x52)` UI BW codes; `GYRO_CONFIG1 (0x51)`/`ACCEL_CONFIG1 (0x53)` UI order; `INTF_CONFIG1 (0x4D)` AFSR.

**Gyro AAF (Bank 1)**: `GYRO_CONFIG_STATIC2 (0x0B)` enable; `…STATIC3/4/5 (0x0C/0x0D/0x0E)` set **DELT / DELTSQR / BITSHIFT**.

**Accel AAF (Bank 2)**: `ACCEL_CONFIG_STATIC2 (0x03)` **DELT in bits 6:1** + enable; `…STATIC3/4 (0x04/0x05)` **DELTSQR / BITSHIFT**.

**UI codes (Bank 0, 0x52)**: Gyro bits [3:0], Accel bits [7:4].
Typical: **15 = low‑latency**, **0 = ODR/2**, **1 ≈ max(400, ODR)/4**.


## A1) Appendix — AAF presets you’ll use 90% of the time

| **BW (Hz)** | **DELT** | **DELTSQR** | **BITSHIFT** | **Notes**                                            |
| ----------: | -------: | ----------: | -----------: | ---------------------------------------------------- |
|         126 |        4 |      0x0010 |           12 | Safe/quiet baseline at 1 k ODR                       |
|         170 |        4 |      0x0010 |           11 | Balanced accel at 1 k; calmer level/loiter           |
|         213 |        5 |      0x0019 |           11 | Between balanced and BF parity                       |
|         258 |        6 |      0x0024 |           10 | Strong default; BF parity for gyro                   |
|         303 |        7 |      0x0031 |           10 | For very stiff builds with strong software filtering |

*(Full table in the datasheet if you need outliers.)*


## A2) Betaflight parity — what to set and why (concise)

* **AAF**: 258 Hz **for both** gyro & accel.  → Good anti‑aliasing; relies on software for fine shaping.
* **UI**: **code 15** (both), **order = reset default (2nd)**.  → Keeps the UI path nearly transparent.
* **AFSR**: **disable** in `INTF_CONFIG1`.  → Avoids rare “stuck” samples noted by multiple FC stacks.
* **ODR/FSR**: Use ODR LUT (8k→3, 4k→4, 2k→5, 1k→6). FSR Gyro ±2000 dps, Accel ±16 g.

This keeps driver behavior familiar to BF users and simplifies tuning advice and log interpretation.


## A3) FAQ and Notes

* **Does “UI wide” mean code 15 or code 0?**  For lowest added delay, prefer **code 15** (BF style). For a predictable UI corner without software filters, use **code 0 (ODR/2)** with **1st‑order**.
* **Why not just crank AAF high?**  Too‑high AAF raises alias risk relative to ODR and can surface noise the SW filters must then kill with extra phase cost. Start near **258 Hz**.
* **Do gyro and accel share the exact same AAF table?**  Yes — same tuple format; only register addresses/bit placements differ.
* **What breaks if I forget AFSR?**  You *might* see bit‑for‑bit repeated samples under certain timing/power conditions. Disable it early and move on.
* The AAF preset table (42 → ~3979 Hz) is the **same** for gyro & accel; only register locations differ.
* For MPU‑6000 parity (DLPF 260/256 feel), letting **AAF define the corner** and keeping **UI wide** is the closest analog while benefiting from the ICM‑42688‑P’s superior front‑end filtering.


## Bottom line

Pick a preset, disable AFSR (INTF_CONFIG1 0x4D → val = (val & ~0xC0) | 0x40), verify with FFT/flight signs, and iterate AAF one step at a time.

* **BF parity:** set both AAFs = 258 Hz, UI BW = code 15 (low-latency) for gyro & accel, leave UI order at reset (2nd).
* **MPU-6000-like feel:** use UI order = 1st and UI BW = code 0 (ODR/2) with Gyro AAF = 258 Hz / Accel AAF = 170 Hz.
* **Guardrails:** keep AAF ≤ 0.45×ODR (absolute max); for control-loop use, aim for 0.05–0.20×ODR (e.g., 213–303 Hz at 4 kHz ODR). Default FSR ±2000 dps / ±16 g unless you have a reason to go with a lower FSR range.