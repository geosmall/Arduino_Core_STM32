# IMU Presets — SAFE / SMOOTH / BALANCED / ACRO (ICM-42688-P · MPU-6000 · MPU-9250)

**FSR for all presets and chips:** Gyro **±2000 dps**, Accel **±16 g**.
**Baseline host ODRs:** Gyro **4 kHz** (except where limited), Accel **1 kHz**. **ACRO** may use **8 kHz** gyro where supported.

> **Corrections in this revision**
>
> * **MPU-9250:** `SMPLRT_DIV` **does not divide** the 8 kHz path when `DLPF_CFG=0`. Balanced/Acro now show **8 kHz** on-sensor with a note to **software-decimate to 4 kHz** if desired.
> * **MPU-6000 / ICM-42688-P:** re-checked rate rules and tables for parity with datasheets and common FC practice.

---

## ICM-42688-P (AAF + UI filter)

> **Init rule:** Disable **Auto-FSR** (AFSR) to prevent range-switch stalls → Bank0 `INTF_CONFIG1 (0x4D)` set bits **[7:6] = 01** (apply mask `0x40`).

| Preset       | Gyro ODR | Accel ODR | **Gyro AAF** *(DELT, DELTSQR, BITSHIFT)* | **Accel AAF** *(DELT, DELTSQR, BITSHIFT)*                                | **UI BW**   | **UI order** | Notes                              |
| ------------ | -------: | --------: | ---------------------------------------- | ------------------------------------------------------------------------ | ----------- | -----------: | ---------------------------------- |
| **SAFE**     |      1 k |       1 k | **126 Hz** → (4, 0x0010, 12)             | **84 Hz**  → (3, 0x0009, 12)                                             | **code 1**  |          2nd | Very tame; easiest bring-up        |
| **SMOOTH**   |      4 k |       1 k | **213 Hz** → (5, 0x0019, 11)             | **126 Hz** → (4, 0x0010, 12)                                             | **code 1**  |          2nd | Extra HW smoothing                 |
| **BALANCED** |      4 k |       1 k | **258 Hz** → (6, 0x0024, 10)             | **170 Hz** → (4, 0x0010, 11)                                             | **code 15** |      **1st** | Strong default for 2 kHz PID loops |
| **ACRO**     |  **8 k** |       1 k | **303 Hz** → (7, 0x0031, 10)             | **170 Hz** *(or 258 for BF parity)* → (4,0x0010,11) *(or (6,0x0024,10))* | **code 15** |      **1st** | Lowest added phase                 |

**Where to write:**

* **Gyro AAF (Bank1):** 0x0B enable, 0x0C DELT, 0x0D DELTSQR[7:0], 0x0E BITSHIFT[7:4] + DELTSQR[11:8]
* **Accel AAF (Bank2):** 0x03 DELT (bits6:1) + enable, 0x04 DELTSQR[7:0], 0x05 BITSHIFT[7:4] + DELTSQR[11:8]
* **UI (Bank0):** `GYRO_ACCEL_CONFIG0 (0x52)` BW codes (gyro bits[3:0], accel bits[7:4]); `GYRO_CONFIG1 (0x51)` order bits[3:2]; `ACCEL_CONFIG1 (0x53)` order bits[4:3]

**Guardrail:** **AAF ≤ 0.45×ODR** (absolute). For control loops, target **~0.05–0.20×ODR** (e.g., 213–303 Hz @ 4 kHz gyro).

---

## MPU-6000 (classic DLPF)

**Rate rules:**

* `CONFIG.DLPF_CFG = 0` → **base 8 kHz**; **GyroSR = 8 kHz / (1 + SMPLRT_DIV)**
* `CONFIG.DLPF_CFG = 1..6` → **base 1 kHz**; **GyroSR = 1 kHz / (1 + SMPLRT_DIV)**
  Accel path is 1 kHz; at ≥4 kHz gyro you’ll re-read accel between updates.

| Preset       |    `CONFIG.DLPF_CFG` | **SMPLRT_DIV** | **Gyro SR** | **Accel DLPF** | Notes                                           |
| ------------ | -------------------: | -------------: | ----------: | -------------: | ----------------------------------------------- |
| **SAFE**     |        **2** (98 Hz) |              0 |   **1 kHz** |      **94 Hz** | Calm bring-up                                   |
| **SMOOTH**   |       **1** (188 Hz) |              0 |   **1 kHz** |     **184 Hz** | More smoothing                                  |
| **BALANCED** | **0** (256/260 wide) |          **1** |   **4 kHz** |     **260 Hz** | 4 k “wide” feel; matches 42688P Balanced intent |
| **ACRO**     | **0** (256/260 wide) |          **0** |   **8 kHz** |     **260 Hz** | Lowest latency                                  |

**Where to write:**

* `CONFIG (0x1A).DLPF_CFG`, `SMPLRT_DIV (0x19)`
* `GYRO_CONFIG (0x1B).FS_SEL=3 (±2000 dps)`; `ACCEL_CONFIG (0x1C).AFS_SEL=3 (±16 g)`

---

## MPU-9250 (MPU-6500-class gyro + AK8963)

**Rate rules (datasheet-accurate):**

* `CONFIG.DLPF_CFG = 0` → **internal base 8 kHz (wide)**. **On this path, `SMPLRT_DIV` does *not* divide the 8 kHz base**; output remains 8 kHz. Use **software decimation** if you want 4 kHz reads.
* `CONFIG.DLPF_CFG = 1..6` → **internal base 1 kHz**; only **here** does `SMPLRT_DIV` apply: **`GyroSR = 1000 / (1 + SMPLRT_DIV)`**.
* Accel LPF via `ACCEL_CONFIG2.A_DLPF_CFG` (1 kHz update when LPF engaged). Keep `GYRO_CONFIG.FCHOICE_B=00` to use DLPF tables.

| Preset       | **Gyro DLPF** (`CONFIG`) | **SMPLRT_DIV** | **Gyro SR** | **Accel DLPF** (`ACCEL_CONFIG2`)                        | Notes                                                            |
| ------------ | ------------------------ | -------------: | ----------: | ------------------------------------------------------- | ---------------------------------------------------------------- |
| **SAFE**     | **92 Hz**  (code **2**)  |              0 |   **1 kHz** | **92 Hz**  (code **2**)                                 | Very tame                                                        |
| **SMOOTH**   | **184 Hz** (code **1**)  |              0 |   **1 kHz** | **184 Hz** (code **1**)                                 | More smoothing                                                   |
| **BALANCED** | **250 Hz** (code **0**)  |          **0** |   **8 kHz** | **184 Hz** (code **1**)                                 | **Read at 4 kHz in driver (software decimate ×2)** to match feel |
| **ACRO**     | **250 Hz** (code **0**)  |          **0** |   **8 kHz** | **184 Hz** (code **1**) *(or 460 Hz if you want wider)* | Lowest latency                                                   |

**Where to write:**

* `CONFIG (0x1A).DLPF_CFG`, `SMPLRT_DIV (0x19)`, `GYRO_CONFIG (0x1B).FS_SEL`, `GYRO_CONFIG (0x1B).FCHOICE_B=00`
* `ACCEL_CONFIG2 (0x1D): ACCEL_FCHOICE_B=0, A_DLPF_CFG=0..6` (0=460, 1=184, 2=92, …)

> **Footnote:** Per the register map, `SMPLRT_DIV` is **only used for the 1 kHz internal sampling path** (DLPF 1..6). For `DLPF_CFG=0` (8 kHz wide), the divider **does not** reduce the 8 kHz base—use software decimation if you need 4 kHz.

---

## Optional: single-source LUT structure

```c
typedef enum { FILTER_SAFE, FILTER_SMOOTH, FILTER_BALANCED, FILTER_ACRO } ImuPreset;
typedef enum { IMU_ICM42688P, IMU_MPU6000, IMU_MPU9250 } ImuModel;

typedef struct {
  ImuPreset preset;
  uint16_t gyro_rate_hz;
  uint16_t accel_rate_hz;
  // 42688P specifics
  uint8_t  icm_ui_bw_code_gyro, icm_ui_bw_code_accel; // 0..15 (15 = low-latency)
  uint8_t  icm_ui_order_gyro,   icm_ui_order_accel;   // 1 or 2
  uint8_t  icm_g_delT, icm_g_bitshift;  uint16_t icm_g_delTsqr;
  uint8_t  icm_a_delT, icm_a_bitshift;  uint16_t icm_a_delTsqr;
  // MPU specifics
  uint8_t  mpu_dlpf_cfg_gyro, mpu_dlpf_cfg_accel;
  uint8_t  mpu_smpr_div;
} ImuLutRow;
```

---

## Practical tuning tips

* **Betaflight parity (42688P):** AAF **258/258** (gyro/accel), UI **code 15**, UI order **2nd** (reset default).
* If the craft feels **sluggish but clean**, step **gyro AAF up** (213→258→303) or switch UI from **code 1 → code 15** (42688P).
* If **D-term/motor song** is high, step **gyro AAF down** (258→213→170) or move MPU DLPF from 250/184 → 184/92.
* Always re-check **AAF ≤ 0.45×ODR** when you change ODR.
