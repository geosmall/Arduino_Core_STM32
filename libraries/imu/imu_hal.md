# IMU Filtering — Unified Presets (ICM‑42688‑P, MPU‑6000, MPU‑9250)

This document provides a **single, intent‑based abstraction** for configuring hardware filtering on three common IMUs:

* **TDK/Invensense ICM‑42688‑P** (AAF + UI filter)
* **MPU‑6000** (classic DLPF)
* **MPU‑9250** (MPU‑6500 class gyro + AK8963 mag; gyro/accel DLPF)

It defines four **named presets** that map cleanly across parts and preserve the same *latency ↔ noise tolerance* intent:

* **Safe** – very tame for bring‑up and unknown/noisy frames
* **Smooth** – extra on‑chip smoothing with moderate latency
* **Balanced** – strong default for 2 kHz loops (snappy but controlled)
* **Acro** – lowest added phase; rely on strong software filtering

**FSR defaults (all presets, all chips):** Gyro **±2000 dps**, Accel **±16 g**.

**Baseline ODRs:** Gyro **4 kHz**, Accel **1 kHz** (default, 8K override for capable parts).

---

## 1) Minimal API

```c
typedef enum { FILTER_SAFE, FILTER_SMOOTH, FILTER_BALANCED, FILTER_ACRO } ImuFilterPreset;
typedef enum { IMU_ICM42688P, IMU_MPU6000, IMU_MPU9250 } ImuModel;

bool imuApplyPreset(ImuModel model, ImuFilterPreset preset);
/* Implementation selects model‑specific register writes from exact LUT entries. */
```

**Design rules**

* Presets encode **intent**, not chip quirks.
* **AAF/DLPF** sets the real front‑end corner; UI/DLPF **trims** latency vs smoothing.
* Exact values only (no ranges). A small **LUT** per part keeps all magic numbers in one place.

---

## 2) ICM‑42688‑P (AAF + UI filter)

**Always (all presets):** Disable AFSR early → Bank0 `INTF_CONFIG1 (0x4D)`:

```c
uint8_t v = readReg(0x4D);
v &= ~0xC0;  // clear [7:6]
v |=  0x40;  // AFSR_DISABLE
writeReg(0x4D, v);
```

**UI fields (Bank0):** `GYRO_ACCEL_CONFIG0 (0x52)` → Gyro BW bits[3:0], Accel BW bits[7:4];
`GYRO_CONFIG1 (0x51)` bits[3:2] → gyro order; `ACCEL_CONFIG1 (0x53)` bits[4:3] → accel order.

> AAF registers:
> **Gyro (Bank1):** 0x0B enable, 0x0C DELT, 0x0D DELTSQR[7:0], 0x0E BITSHIFT[7:4]+DELTSQR[11:8]
> **Accel (Bank2):** 0x03 DELT in bits6:1 + enable, 0x04 DELTSQR[7:0], 0x05 BITSHIFT[7:4]+DELTSQR[11:8]

### Presets (Gyro 4 kHz, Accel 1 kHz; Acro allows 8 kHz gyro)

| Preset       | Gyro ODR | Accel ODR | **Gyro AAF** (DELT, DELTSQR, BITSHIFT) | **Accel AAF** (DELT, DELTSQR, BITSHIFT)                                     | **UI order** |   **UI BW** | Notes                                      |
| ------------ | -------: | --------: | -------------------------------------- | --------------------------------------------------------------------------- | -----------: | ----------: | ------------------------------------------ |
| **Safe**     |      4 k |       1 k | **126 Hz** → (4, 0x0010, 12)           | **84 Hz**  → (3, 0x0009, 12)                                                |          2nd |  **code 1** | Very tame bring‑up on unknown/noisy frames |
| **Smooth**   |      4 k |       1 k | **213 Hz** → (5, 0x0019, 11)           | **126 Hz** → (4, 0x0010, 12)                                                |          2nd |  **code 1** | Extra smoothing with moderate latency      |
| **Balanced** |      4 k |       1 k | **258 Hz** → (6, 0x0024, 10)           | **170 Hz** → (4, 0x0010, 11)                                                |          1st | **code 15** | Strong default for 2 kHz PID loops         |
| **Acro**     |  **8 k** |       1 k | **303 Hz** → (7, 0x0031, 10)           | **170 Hz** *(or 258 Hz for BF parity)* → (4,0x0010,11) *(or (6,0x0024,10))* |          1st | **code 15** | Lowest added phase; rely on SW filters     |

**Two “wide” UI choices (policy‑selectable):**

* **MPU‑like wide:** **UI order = 1st**, **UI BW = code 0 (ODR/2)** → small, defined UI corner with minimal phase; AAF remains primary.
* **BF‑parity wide:** **UI order = 2nd (reset)**, **UI BW = code 15** → very wide, minimal extra delay; rely on software filters.

**Guardrails**

* Treat **AAF ≤ 0.45×ODR** as an absolute max. For control loops aim **0.05–0.20×ODR** (e.g., **213–303 Hz at 4 kHz gyro**).

---

## 3) MPU‑6000 (classic DLPF)

**Rate semantics**

* **DLPF disabled** → `CONFIG.DLPF_CFG = 0` and `FCHOICE_B[1:0] = 00` → internal gyro **8 kHz**; host **Sample Rate = 8 kHz / (1 + SMPLRT_DIV)`**.
* **DLPF enabled** → `CONFIG.DLPF_CFG = 1..6` → internal gyro **1 kHz**; host **Sample Rate = 1 kHz / (1 + SMPLRT_DIV)`**.
* **Accelerometer output rate** is **1 kHz**; at host rates >1 kHz the same accel sample can repeat.

**DLPF reference (CONFIG.DLPF_CFG)**

| DLPF_CFG | Accel BW (Hz) | Gyro BW (Hz) | Gyro Delay (ms) | Gyro Fs (kHz) |
| -------: | ------------: | -----------: | --------------: | ------------: |
|        0 |           260 |          256 |            0.98 |             8 |
|        1 |           184 |          188 |             1.9 |             1 |
|        2 |            94 |           98 |             2.8 |             1 |
|        3 |            44 |           42 |             4.8 |             1 |
|        4 |            21 |           20 |             8.3 |             1 |
|        5 |            10 |           10 |            13.4 |             1 |
|        6 |             5 |            5 |            18.6 |             1 |

### Presets (baseline: target 4 kHz gyro / 1 kHz accel; Acro allows 8 kHz)

| Preset       | **Gyro DLPF (CONFIG.DLPF_CFG)** | **Accel DLPF** | **SMPLRT_DIV** | **Effective host gyro** | Notes                                    |
| ------------ | ------------------------------- | -------------- | -------------: | ----------------------: | ---------------------------------------- |
| **Safe**     | **98 Hz**  (value **2**)        | **94 Hz**      |          **0** |               **1 kHz** | Calm bring‑up; DLPF enabled ⇒ 1 kHz host |
| **Smooth**   | **188 Hz** (value **1**)        | **184 Hz**     |          **0** |               **1 kHz** | Smoother logs; DLPF enabled              |
| **Balanced** | **256/260 wide** (value **0**)  | **260 Hz**     |          **1** |               **4 kHz** | “MPU‑wide” feel @ 4 kHz host             |
| **Acro**     | **256/260 wide** (value **0**)  | **260 Hz**     |          **0** |               **8 kHz** | Lowest latency; tight SPI + SW filters   |

**Key registers**

* `CONFIG (0x1A).DLPF_CFG`, `SMPLRT_DIV (0x19)`
* `GYRO_CONFIG (0x1B).FS_SEL` → {0:±250, 1:±500, 2:±1000, 3:±2000 dps}
* `ACCEL_CONFIG (0x1C).AFS_SEL` → {0:±2, 1:±4, 2:±8, 3:±16 g}

**Programming shortcuts**

* **4 kHz host (Balanced):** `DLPF_CFG=0`, `SMPLRT_DIV=1`; set FS_SEL=3, AFS_SEL=3.
* **8 kHz host (Acro):** `DLPF_CFG=0`, `SMPLRT_DIV=0`; same FSRs.
* **1 kHz host (Safe/Smooth):** `DLPF_CFG=2 or 1`, `SMPLRT_DIV=0`; set accel DLPF accordingly.

---

## 4) MPU‑9250 (MPU‑6500‑class gyro; AK8963 mag)

**DLPF options**

* **Gyro:** `CONFIG.DLPF_CFG = 0..6` → **250/184/92/41/20/10/5 Hz**
* **Accel:** `ACCEL_CONFIG2.A_DLPF_CFG = 0..6` → **460/184/92/41/20/10/5 Hz**

**Rate rules**

* With DLPF **enabled** → internal gyro **1 kHz**; host rate = `1 kHz / (1 + SMPLRT_DIV)`.
* With gyro “**wide/250**” (DLPF_CFG=0) → behaves like DLPF‑off for rate purposes; host = `8 kHz / (1 + SMPLRT_DIV)`.

### Presets

| Preset       | **Gyro DLPF (CONFIG)**   | **Accel DLPF (ACCEL_CONFIG2)** | **SMPLRT_DIV** | **Effective host gyro** | Notes                               |
| ------------ | ------------------------ | ------------------------------ | -------------: | ----------------------: | ----------------------------------- |
| **Safe**     | **92 Hz**  (value **2**) | **92 Hz**  (value **2**)       |          **0** |               **1 kHz** | Calm bring‑up                       |
| **Smooth**   | **184 Hz** (value **1**) | **184 Hz** (value **1**)       |          **0** |               **1 kHz** | Smoother logs                       |
| **Balanced** | **250 Hz** (value **0**) | **184 Hz** (value **1**)       |          **1** |               **4 kHz** | Wide gyro @ 4 kHz host, accel 1 kHz |
| **Acro**     | **250 Hz** (value **0**) | **184 Hz** (value **1**)       |          **0** |               **8 kHz** | Lowest latency; strong SW filters   |

**Key registers**

* `CONFIG (0x1A).DLPF_CFG`, `ACCEL_CONFIG2 (0x1D).A_DLPF_CFG`, `SMPLRT_DIV (0x19)`
* `GYRO_CONFIG (0x1B).FS_SEL`, `ACCEL_CONFIG (0x1C).AFS_SEL`

---

## 5) Cross‑chip intent parity (why this works)

* **Intent carries over:** *Safe/Smooth* trade rate for calm signals; *Balanced/Acro* maximize bandwidth/latency. The *feel* is consistent across chips even though mechanisms differ (AAF+UI vs DLPF).
* **ICM‑42688‑P uses AAF as the primary corner** (e.g., 258/170 Hz) and keeps UI wide (code 15) unless you explicitly want more on‑chip smoothing (Safe/Smooth use code 1 or code 0 + 1st order for MPU‑like feel).
* **MPU‑6000/9250 quirks are hidden behind presets:** you get the right DLPF + divider for the host rate you expect.

---

## 6) Drop‑in LUT skeleton (single source of truth)

```c
typedef struct {
  ImuFilterPreset preset;
  uint16_t gyro_rate_hz;   // intended host-visible rate
  uint16_t accel_rate_hz;  // intended host-visible rate
  // model-specific fields...
} ImuPreset;

const ImuPreset ICM42688P_LUT[] = {
/* SAFE */     {FILTER_SAFE,     4000, 1000 /* + AAF (4,0x0010,12)/(3,0x0009,12), UI order=2nd, UI BW=1 */},
/* SMOOTH */   {FILTER_SMOOTH,   4000, 1000 /* + AAF (5,0x0019,11)/(4,0x0010,12), UI order=2nd, UI BW=1 */},
/* BALANCED */ {FILTER_BALANCED, 4000, 1000 /* + AAF (6,0x0024,10)/(4,0x0010,11), UI order=1st, UI BW=15 */},
/* ACRO */     {FILTER_ACRO,     8000, 1000 /* + AAF (7,0x0031,10)/(4,0x0010,11 or 6,0x0024,10), UI order=1st, UI BW=15 */},
};

const ImuPreset MPU6000_LUT[] = {
/* SAFE */     {FILTER_SAFE,     1000, 1000 /* DLPF: Gyro 98(2), Accel 94; DIV=0 */},
/* SMOOTH */   {FILTER_SMOOTH,   1000, 1000 /* DLPF: Gyro 188(1), Accel 184; DIV=0 */},
/* BALANCED */ {FILTER_BALANCED, 4000, 1000 /* DLPF: Gyro wide(0), Accel 260; DIV=1 */},
/* ACRO */     {FILTER_ACRO,     8000, 1000 /* DLPF: Gyro wide(0), Accel 260; DIV=0 */},
};

const ImuPreset MPU9250_LUT[] = {
/* SAFE */     {FILTER_SAFE,     1000, 1000 /* Gyro 92(2), Accel 92(2); DIV=0 */},
/* SMOOTH */   {FILTER_SMOOTH,   1000, 1000 /* Gyro 184(1), Accel 184(1); DIV=0 */},
/* BALANCED */ {FILTER_BALANCED, 4000, 1000 /* Gyro 250(0), Accel 184(1); DIV=1 */},
/* ACRO */     {FILTER_ACRO,     8000, 1000 /* Gyro 250(0), Accel 184(1); DIV=0 */},
};
```

---

## 7) Validation checklist (bring‑up)

* **Sanity:** WHOAMI, FSRs, ODRs match preset; DRDY rate matches expectation.
* **FFT/logs:** AAF/DLPF corner visible (e.g., ~170–303 Hz typical for gyro at 4 kHz ODR on 42688P). No hard alias ridges near Nyquist.
* **Phase feel:** craft not sluggish with chosen UI/DLPF. If sluggish, open UI (code 15) or raise DLPF/AAF one step.
* **Noise:** if D‑term/motor song high → lower **gyro AAF** (42688P) or pick a **lower DLPF code** (MPU). If too damped but clean → raise one step.
* **Parity toggles:**

  * **Betaflight on 42688P:** AAF **258/258** (gyro/accel), UI **code 15**, order **2nd**.
  * **MPU‑6000 ‘wide’ feel on 42688P:** UI **code 0 (ODR/2)**, **1st‑order**, AAF **≈258 gyro / 170 accel**.

---

## 8) Bottom line

Pick a preset, program the **front‑end corner** (AAF or DLPF), keep the **UI/DLPF** wide unless you need on‑chip smoothing, and verify with FFT + flight signs. For fast parity:

* **Betaflight (42688P):** AAF **258/258**, UI **code 15**, order **2nd**.
* **MPU‑6000 classic (8k/256):** DLPF **0**, `SMPLRT_DIV=0..1` for **8k/4k** host.
* **General default:** **Balanced** preset.
