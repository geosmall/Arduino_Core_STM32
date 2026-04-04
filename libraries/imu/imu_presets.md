# Betaflight-Oriented IMU Filtering Guide (ICM‑42688‑P, MPU‑6000, MPU‑9250, ICM‑20602)

This guide consolidates what we learned into one **clean, BF‑style reference** for configuring hardware filters and sample rates across four popular IMUs. It’s structured for developers who want consistent **intent** (latency vs noise) with minimal chip‑specific surprises.

* **ICM‑42688‑P** — dual stage (**AAF** + **UI**) with optional Auto‑FSR (disable it)
* **MPU‑6000** — classic **DLPF** with true 8 k/4 k paths via divider
* **MPU‑9250** — classic **DLPF**, but **no 4 k via divider** on wide/8 k path
* **ICM‑20602** — MPU‑6500‑class: DLPF + a very wide/bypass path

> **FSR default for all modes:** Gyro **±2000 dps**, Accel **±16 g**.
> **Baseline loop assumption:** Gyro ODR target **4 kHz** (8 kHz for Acro), Accel **1 kHz**.

---

## 1) Bottom line presets (one intent, all chips)

Choose one of these and you’ll get nearly the same “feel” regardless of IMU. **Program the hardware first; finish with software filters (PT1/biquads) per your airframe.**

| Preset       | Intent                      | ICM‑42688‑P                                                             | MPU‑6000                                                    | MPU‑9250                                                                   | ICM‑20602                                                      |
| ------------ | --------------------------- | ----------------------------------------------------------------------- | ----------------------------------------------------------- | -------------------------------------------------------------------------- | -------------------------------------------------------------- |
| **SAFE**     | Bring‑up, very noisy frames | Gyro/Acc ODR 1 k; AAF **126/84 Hz**; UI **code 1**, **2nd‑order**       | DLPF **98/94 Hz**, SR **1 k**                               | DLPF **92/92 Hz**, SR **1 k**                                              | DLPF **≈92/92 Hz**, SR **1 k**                                 |
| **SMOOTH**   | Extra on‑chip smoothing     | Gyro ODR 4 k, Acc 1 k; AAF **213/126 Hz**; UI **code 1**, **2nd‑order** | DLPF **188/184 Hz**, SR **1 k**                             | DLPF **184/184 Hz**, SR **1 k**                                            | DLPF **≈176/184 Hz**, SR **1 k**                               |
| **BALANCED** | Default for 2 k PID         | Gyro ODR 4 k; AAF **258/170 Hz**; UI **code 15**, **1st‑order**         | Gyro **DLPF=0**, **SMPLRT_DIV=1** → **4 k**; Acc **260 Hz** | **Wide 8 k** (DLPF=0), **read at 4 k in SW** (decimate ×2); Acc **184 Hz** | **Wide** gyro (bypass), **read at 4 k in SW**; Acc **≈184 Hz** |
| **ACRO**     | Minimum phase               | Gyro ODR **8 k**; AAF **303/170 Hz**; UI **code 15**, **1st‑order**     | Gyro **8 k** (DLPF=0, DIV=0); Acc **260 Hz**                | **Wide 8 k** (DLPF=0), Acc **184/460 Hz**                                  | **Wide 8 k** (bypass), Acc **≈184/218 Hz**                     |

> **Notes**
> • On **MPU‑9250** and **ICM‑20602**, when the gyro is in **wide/8 k** mode the **divider does not produce 4 k**. To fly at a 4 k read cadence, **software‑decimate** the 8 k stream.
> • On **MPU‑6000**, the **divider *does*** produce **4 k** from the 8 k path (DLPF=0 + `SMPLRT_DIV=1`).
> • On **ICM‑42688‑P**, AAF is the **main anti‑alias corner**; UI is best kept **wide (code 15)** unless you need extra on‑chip smoothing.

---

## 2) Common concepts (quick refresher)

* **ODR vs DRDY:** The **gyro ODR** sets how often new data is produced; **DRDY** typically pulses at that ODR. Your task rate (read cadence) can be the same or a software decimation of it.
* **Hardware vs software filtering:** Hardware filters (DLPF/AAF/UI) define the front‑end corner and **group delay floor**; software filters finalize noise/phase tradeoffs.
* **Do not over‑close** the hardware corner near Nyquist of your control loop—leave room for software filters.

### ⚠️ Critical distinction: MPU-6000 vs. MPU-9250 / ICM-20602 sample-rate divider behavior

* **MPU-6000 (6000-class):**  
`CONFIG.DLPF_CFG=0` selects the **8 kHz** gyro path and the **`SMPLRT_DIV` divider still applies** → e.g. `SMPLRT_DIV=1` → **4 kHz**, `=0` → **8 kHz**. There are **no** `FCHOICE_B` bits on the MPU-6000; the DLPF code alone controls path and bandwidth.  
*Note:* `SMPLRT_DIV` divides the **gyro** output rate; the **accelerometer** path is ~**1 kHz** and not divided by `SMPLRT_DIV`.

* **MPU-9250 / ICM-20602 (6500-class):**  
The divider **only** applies when the **DLPF path is engaged** (**`FCHOICE_B=00`** and **`DLPF_CFG ∈ {1..6}`**), i.e., the **1 kHz** internal path. When you select the **wide / 8 kHz** gyro path (`DLPF_CFG=0` or bypass via **`FCHOICE_B≠00`**), **`SMPLRT_DIV` does not divide**; output remains **8 kHz**. If you want a **4 kHz** read cadence in wide mode, **decimate in software**.

* **Accelerometer (6500-class, per-chip specifics):**  
`ACCEL_CONFIG2` governs accel filtering. With the accel DLPF engaged (**`ACCEL_FCHOICE_B=0`**), the accel output is **1 kHz** with selectable LPF codes (e.g., 184/92/41/20/10/5 Hz). With bypass (**`ACCEL_FCHOICE_B=1`**), the accel path is **very wide** and **not** rate-divided by `SMPLRT_DIV`:  
– **MPU-9250:** accel ODR **4 kHz**, ~**1.13 kHz** 3 dB BW (bypass).  
– **ICM-20602:** accel ODR **4 kHz**, ~**1.05 kHz** 3 dB BW (bypass).


---

## 3) ICM‑42688‑P (AAF + UI)

**Do this first (critical):** **Disable Auto‑FSR (AFSR)** to avoid “sticky” samples during internal range switching.

* Bank0 `INTF_CONFIG1 (0x4D)` → set bits **[7:6] = 01** (write mask `0x40`).

**Programming fields**

* **AAF (Gyro, Bank1):** `0x0B` enable, `0x0C` DELT, `0x0D` DELTSQR[7:0], `0x0E` BITSHIFT[7:4]+DELTSQR[11:8]
* **AAF (Accel, Bank2):** `0x03` DELT(bits6:1)+enable, `0x04` DELTSQR[7:0], `0x05` BITSHIFT[7:4]+DELTSQR[11:8]
* **UI (Bank0):** `GYRO_ACCEL_CONFIG0 (0x52)` BW codes (gyro bits[3:0], accel bits[7:4]); `GYRO_CONFIG1 (0x51)` order bits[3:2]; `ACCEL_CONFIG1 (0x53)` order bits[4:3]

**Recommended values (exact tuples)**

* **SAFE** — Gyro/Acc ODR 1 k; **Gyro AAF 126 Hz** → `(4, 0x0010, 12)`; **Acc AAF 84 Hz** → `(3, 0x0009, 12)`; **UI code 1**, **2nd‑order**
* **SMOOTH** — Gyro ODR 4 k; **213/126 Hz**; **UI code 1**, **2nd‑order**
* **BALANCED** — Gyro ODR 4 k; **258/170 Hz**; **UI code 15**, **1st‑order** (low extra phase)
* **ACRO** — Gyro ODR **8 k**; **303/170 Hz**; **UI code 15**, **1st‑order**

**Guardrails**

* Treat **AAF ≤ 0.45×ODR** as hard max. For control loops, prefer **0.05–0.20×ODR** (e.g., 213–303 Hz at 4 k gyro).
* Keep **FSR ±2000 dps / ±16 g** unless you have solid headroom measurements.

---

## 4) MPU‑6000 (classic DLPF)

**Rate rules**

* `CONFIG.DLPF_CFG = 0` ⇒ **8 k** gyro base. With `SMPLRT_DIV=1` you get **4 k**; with `0` you get **8 k**.
* `CONFIG.DLPF_CFG = 1..6` ⇒ **1 k** gyro base, then `SR = 1000/(1+DIV)`.

**Presets**

* **SAFE** — Gyro **98 Hz** (code 2), Acc **94 Hz**, **1 k** SR
* **SMOOTH** — Gyro **188 Hz** (1), Acc **184 Hz**, **1 k** SR
* **BALANCED** — **DLPF=0**, `DIV=1` → **4 k**; Acc **260 Hz**
* **ACRO** — **DLPF=0**, `DIV=0` → **8 k**; Acc **260 Hz**

**Notes**

* Accel path is **1 k**; at 4–8 k gyro you’ll re‑read accel between updates.
* Keep `FCHOICE_B=00` (DLPF path) unless you intentionally want full bypass behavior.

---

## 5) MPU‑9250 (MPU‑6500‑class)

**Critical difference vs MPU‑6000**

* `CONFIG.DLPF_CFG = 0` puts gyro on an **8 k wide path**, **but the divider does not make 4 k**. Output stays **8 k**; **software‑decimate to 4 k** if desired.
* `CONFIG.DLPF_CFG = 1..6` is the **1 k** path where the divider applies.

**Presets**

* **SAFE** — Gyro **92 Hz** (2), Acc **92 Hz**, **1 k** SR
* **SMOOTH** — Gyro **184 Hz** (1), Acc **184 Hz**, **1 k** SR
* **BALANCED** — Gyro **DLPF=0 wide 8 k**, **read at 4 k in SW**; Acc **184 Hz**
* **ACRO** — Gyro **DLPF=0 wide 8 k**, Acc **184 Hz** (or **460 Hz** if you want wider)

**Notes**

* Keep `GYRO_CONFIG.FCHOICE_B=00` to use DLPF table; non‑zero selects analog‑wide modes.
* Accel **ACCEL_CONFIG2.A_DLPF_CFG**: 460/184/92/41/20/10/5 Hz — choose 184 Hz for Balanced/Acro.

---

## 6) ICM‑20602 (MPU‑6500‑class)

**Rate rules**

* **Wide/bypass**: DLPF bypass (non‑zero `FCHOICE_B`) or `DLPF_CFG=0/7` → **very wide** gyro, internal ~**8 k** path. **Divider doesn’t produce 4 k** here → **software‑decimate**.
* **DLPF path**: `FCHOICE_B=00` and `DLPF_CFG=1..6` → **1 k** internal; set SR via `SMPLRT_DIV`.

**Presets**

* **SAFE** — DLPF **≈92/92 Hz**, **1 k** SR
* **SMOOTH** — DLPF **≈176/184 Hz**, **1 k** SR
* **BALANCED** — **Wide** gyro (bypass/0), **read at 4 k in SW**; Acc **≈184 Hz**
* **ACRO** — **Wide 8 k** gyro; Acc **≈184/218 Hz**

**Notes**

* Accel has DLPF table; a bypass option gives ~1 kHz‑class BW.
* Keep SPI/I²C budgets in mind if you actually read at 8 k.

---

## 7) Betaflight parity & migration tips

* **ICM‑42688‑P BF‑parity** — Set **both AAFs = 258 Hz**, **UI BW code = 15**, **UI order = 2nd**; Gyro ODR 8 k/4 k per target; Acc ODR 1 k.
* **From MPU‑6000 tune to 42688P** — Start with **Balanced** (258/170 Hz AAF; UI 15/1st), then retune SW filters. If you need more damping, flip UI to **code 1/2nd** before reducing AAF.
* **From 9250/20602 to 42688P** — If you were flying **8 k wide** before, keep it and **decimate in SW** to 4 k while matching AAF ≈ **258 Hz**.

---

## 8) Implementation checklist

1. **FSR**: Gyro ±2000 dps; Accel ±16 g.
2. **ICM‑42688‑P only**: Disable **AFSR** (`INTF_CONFIG1[7:6]=01`).
3. **Program ODR & filter block** (per table for your preset).
4. **Confirm DRDY**: Verify pulse rate vs intended SR (logic analyzer).
5. **FFT/log review**: AAF/DLPF corner seen where expected; no alias ridge near Nyquist.
6. **Flight signs**: If sluggish but clean → widen UI or raise AAF/DLPF one step. If motor song/D‑term high → lower AAF/DLPF one step.

---

## 9) Copy‑paste snippets

**ICM‑42688‑P — disable Auto‑FSR**

```c
uint8_t v = readReg(0x4D);  // INTF_CONFIG1 (Bank0)
v &= ~0xC0;                 // clear [7:6]
v |=  0x40;                 // set [7:6] = 01 (disable Auto‑FSR)
writeReg(0x4D, v);
```

**MPU‑6000 — 4 k “wide” gyro**

```c
write(CONFIG, 0x00);        // DLPF_CFG=0 → 8k base, 256/260 Hz BW
write(SMPLRT_DIV, 0x01);    // 8k/(1+1) = 4k gyro SR
```

**MPU‑9250 — 8 k wide + SW decimate to 4 k**

```c
write(CONFIG, 0x00);        // DLPF_CFG=0 → 8k wide
// read every other DRDY in driver to get effective 4k cadence
```

**ICM‑20602 — wide gyro + SW decimate**

```c
// Either FCHOICE_B != 0 or DLPF_CFG = 0/7 for wide path
// Then read at 8k and decimate in software to 4k if desired
```

---

## 10) Appendix — field references (quick map)

* **ICM‑42688‑P**: `INTF_CONFIG1 (0x4D)`, `GYRO_CONFIG1 (0x51)`, `GYRO_ACCEL_CONFIG0 (0x52)`, `ACCEL_CONFIG1 (0x53)`, AAF regs (Bank1/2 listed above).
* **MPU‑6000**: `PWR_MGMT_1 (0x6B)`, `SMPLRT_DIV (0x19)`, `CONFIG (0x1A)`, `GYRO_CONFIG (0x1B)`, `ACCEL_CONFIG (0x1C)`.
* **MPU‑9250**: `SMPLRT_DIV (0x19)`, `CONFIG (0x1A)`, `GYRO_CONFIG (0x1B, FCHOICE_B)`, `ACCEL_CONFIG2 (0x1D, A_DLPF_CFG)`.
* **ICM‑20602**: `SMPLRT_DIV (0x19)`, `CONFIG (0x1A)`, `GYRO_CONFIG (0x1B, FCHOICE_B)`, `ACCEL_CONFIG2 (0x1D)`.
