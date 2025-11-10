# J-Link Programming & Auto‑Device Selection Guide

This doc standardizes how we program STM32 targets (F4/F7/G4/H7) **without hard‑coding the device**. We probe the MCU over SWD, derive the **line** from `DBGMCU_IDCODE`, read the **Flash size** (KB) to pick the **density letter** (xB/xC/xE/xG/xH/xI), then invoke J‑Link with the correct `-device`.

> **Key idea**: *Line + density* is sufficient for J‑Link’s flash loader. The **package letter** (ZI/VI/U**, etc.) does **not** matter for programming.

---

## Quick Start (TL;DR)

1. Put the scripts in your repo:

   * `tools/jlink/probe_all.jlink`
   * `tools/jlink/pick_device.sh` (Linux/macOS/WSL)
   * `tools/jlink/pick_device.ps1` (Windows PowerShell)
   * `tools/jlink/program.jlink` (your project’s actual flashing steps)
2. Run one of:

   ```bash
   ./tools/jlink/pick_device.sh && ./tools/jlink/flash.sh
   ```

   or

   ```powershell
   ./tools/jlink/pick_device.ps1
   ```
3. The picker prints something like `STM32H743xI` and then programs it.

---

## Addresses We Read

### DBGMCU_IDCODE (Line + Revision)

* **H7**: `0x5C001000`
* **F4/F7/G4 (classic)**: `0xE0042000`

Layout (all families):

* `DEV_ID` = bits **11:0** → MCU line (family)
* `REV_ID` = bits **31:16** → silicon revision

### Flash Size Register (16‑bit, value in KB!)

> Always read **16‑bit**; using 32‑bit reads can misreport values.

| Family | Address      |
| -----: | ------------ |
|     F4 | `0x1FFF7A22` |
|     F7 | `0x1FF0F442` |
|     G4 | `0x1FFF75E0` |
|     H7 | `0x1FF1E880` |

---

## Probe Script (works across F4/F7/G4/H7)

Save as **`tools/jlink/probe_all.jlink`**:

```jlink
if SWD
speed 4000
connect
halt

// --- IDCODEs ---
mem32 0x5C001000, 1   // H7 DBGMCU_IDCODE (try first)
mem32 0xE0042000, 1   // Classic DBGMCU_IDCODE (F4/F7/G4)

// --- Flash-size (16-bit, KB) ---
mem16 0x1FF1E880, 1   // H7 flash size (KB)
mem16 0x1FF0F442, 1   // F7 flash size (KB)
mem16 0x1FFF7A22, 1   // F4 flash size (KB)
mem16 0x1FFF75E0, 1   // G4 flash size (KB)

q
```

---

## Device Picker (Bash)

Save as **`tools/jlink/pick_device.sh`** and make executable (`chmod +x`):

```bash
#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
OUT=$(JLinkExe -CommanderScript "$SCRIPT_DIR/probe_all.jlink" | tr -d '\r')

# Extract values
ID_H7=$(grep -E "5C001000" <<<"$OUT" | awk '{print $3}')
ID_CL=$(grep -E "E0042000" <<<"$OUT" | awk '{print $3}')
FSZ_H7=$(grep -E "1FF1E880" <<<"$OUT" | awk '{print $3}')
FSZ_F7=$(grep -E "1FF0F442" <<<"$OUT" | awk '{print $3}')
FSZ_F4=$(grep -E "1FFF7A22" <<<"$OUT" | awk '{print $3}')
FSZ_G4=$(grep -E "1FFF75E0" <<<"$OUT" | awk '{print $3}')

# Decide IDCODE
ID=${ID_H7:-}
if [[ -z "${ID}" || "${ID}" == "0x00000000" ]]; then
  ID=${ID_CL:-}
fi
[[ -z "${ID}" ]] && { echo "No DBGMCU_IDCODE read."; exit 1; }

VAL=$((ID))
DEV_ID=$(( VAL & 0x0FFF ))
REV_ID=$(( (VAL >> 16) & 0xFFFF ))

# Pick flash size by whichever address responded
KB=""
FI=""
if [[ -n "${FSZ_H7:-}" && "${FSZ_H7}" != "0x0000" ]]; then KB=$((FSZ_H7)); FI="H7"; fi
if [[ -z "$KB" && -n "${FSZ_F7:-}" && "${FSZ_F7}" != "0x0000" ]]; then KB=$((FSZ_F7)); FI="F7"; fi
if [[ -z "$KB" && -n "${FSZ_F4:-}" && "${FSZ_F4}" != "0x0000" ]]; then KB=$((FSZ_F4)); FI="F4"; fi
if [[ -z "$KB" && -n "${FSZ_G4:-}" && "${FSZ_G4}" != "0x0000" ]]; then KB=$((FSZ_G4)); FI="G4"; fi
[[ -z "${KB}" ]] && { echo "No Flash-size (KB) read."; exit 1; }

# Map DEV_ID -> line (extend as you adopt more)
LINE=""
case $DEV_ID in
  # F4
  0x413) LINE="STM32F407" ;; # F405/407
  0x419) LINE="STM32F429" ;; # F42x/F43x
  0x431) LINE="STM32F411" ;;
  0x441) LINE="STM32F412" ;;
  0x463) LINE="STM32F413" ;; # /423
  0x421) LINE="STM32F446" ;;
  0x434) LINE="STM32F469" ;; # /479
  # F7
  0x449) LINE="STM32F767" ;; # F76x/F77x
  0x451) LINE="STM32F732" ;; # F72x/F73x
  # H7x3 line (H743/H753/H750 share this)
  0x450) LINE="STM32H743" ;;
  # Add other DEV_IDs here as needed
esac

# Fallback based on which flash-size address worked
if [[ -z "$LINE" ]]; then
  case "$FI" in
    H7) LINE="STM32H7";;
    F7) LINE="STM32F7";;
    F4) LINE="STM32F4";;
    G4) LINE="STM32G4";;
  esac
fi

# Density letter from KB
DENSITY="xB"  # 128 KB default
if   (( KB >= 2048 )); then DENSITY="xI"
elif (( KB >= 1536 )); then DENSITY="xH"
elif (( KB >= 1024 )); then DENSITY="xG"
elif (( KB >=  512 )); then DENSITY="xE"
elif (( KB >=  256 )); then DENSITY="xC"
elif (( KB >=  128 )); then DENSITY="xB"
fi

DEVICE="${LINE}${DENSITY}"
echo "DBGMCU_IDCODE=0x$(printf %08x ${VAL})  DEV_ID=0x$(printf %03x ${DEV_ID})  REV_ID=0x$(printf %04x ${REV_ID})  Flash=${KB}KB  → ${DEVICE}"

# Export for subsequent scripts
export JLINK_DEVICE="$DEVICE"
```

### Example flash wrapper (Bash)

Save as **`tools/jlink/flash.sh`**:

```bash
#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
: "${JLINK_DEVICE:?Run pick_device.sh first}"

JLinkExe -device "${JLINK_DEVICE}" -if SWD -speed 4000 -autoconnect 1 \
  -CommanderScript "$SCRIPT_DIR/program.jlink"
```

---

## Device Picker (PowerShell)

Save as **`tools/jlink/pick_device.ps1`**:

```powershell
$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$out = & JLink.exe -CommanderScript (Join-Path $here 'probe_all.jlink')

function Get-Token([string]$pattern){
  ($out | Select-String $pattern | Select-Object -First 1).Line.Split()[2]
}

$hexIdH7  = Get-Token '5C001000'
$hexIdCl  = Get-Token 'E0042000'
$hexFszH7 = Get-Token '1FF1E880'
$hexFszF7 = Get-Token '1FF0F442'
$hexFszF4 = Get-Token '1FFF7A22'
$hexFszG4 = Get-Token '1FFF75E0'

$hexId = if ($hexIdH7 -and $hexIdH7 -ne '0x00000000') { $hexIdH7 } else { $hexIdCl }
if (-not $hexId) { throw 'No DBGMCU_IDCODE read.' }

[uint32]$val = $hexId
$devId = $val -band 0x0FFF
$revId = ($val -shr 16) -band 0xFFFF

$kb = $null; $fi = ''
foreach($kv in @([pscustomobject]@{v=$hexFszH7; f='H7'},[pscustomobject]@{v=$hexFszF7; f='F7'},[pscustomobject]@{v=$hexFszF4; f='F4'},[pscustomobject]@{v=$hexFszG4; f='G4'})){
  if ($kv.v -and $kv.v -ne '0x0000') { [uint32]$kb = $kv.v; $fi=$kv.f; break }
}
if (-not $kb) { throw 'No Flash-size (KB) read.' }

$line = switch ($devId) {
  0x413 { 'STM32F407' }
  0x419 { 'STM32F429' }
  0x431 { 'STM32F411' }
  0x441 { 'STM32F412' }
  0x463 { 'STM32F413' }
  0x421 { 'STM32F446' }
  0x434 { 'STM32F469' }
  0x449 { 'STM32F767' }
  0x451 { 'STM32F732' }
  0x450 { 'STM32H743' }
  Default { '' }
}
if (-not $line) {
  $line = switch ($fi) { 'H7' {'STM32H7'} 'F7' {'STM32F7'} 'F4' {'STM32F4'} 'G4' {'STM32G4'} }
}

$density = 'xB'
if     ($kb -ge 2048) { $density = 'xI' }
elseif ($kb -ge 1536) { $density = 'xH' }
elseif ($kb -ge 1024) { $density = 'xG' }
elseif ($kb -ge  512) { $density = 'xE' }
elseif ($kb -ge  256) { $density = 'xC' }
elseif ($kb -ge  128) { $density = 'xB' }

$env:JLINK_DEVICE = "$line$density"
"DBGMCU_IDCODE=$('{0:x8}' -f $val)  DEV_ID=$('{0:x3}' -f $devId)  REV_ID=$('{0:x4}' -f $revId)  Flash=${kb}KB  → $($env:JLINK_DEVICE)"
```

### Example `program.jlink`

Customize to your image and verify/erase policy:

```jlink
r
h
loadbin ../build/firmware.bin, 0x08000000
verifybin ../build/firmware.bin, 0x08000000
r
q
```

---

## Common DEV_IDs (cheat table)

> Add to this list as we support more parts.

| DEV_ID | Line (use with density letter)                   |
| :----: | ------------------------------------------------ |
| 0x0413 | STM32F405/407 line → `STM32F407x?`               |
| 0x0419 | STM32F42x/43x line → `STM32F429x?`               |
| 0x0431 | STM32F411 line → `STM32F411x?`                   |
| 0x0441 | STM32F412 line → `STM32F412x?`                   |
| 0x0463 | STM32F413/423 line → `STM32F413x?`/`STM32F423x?` |
| 0x0421 | STM32F446 line → `STM32F446x?`                   |
| 0x0434 | STM32F469/479 line → `STM32F469x?`               |
| 0x0449 | STM32F76x/77x line → `STM32F767x?`               |
| 0x0451 | STM32F72x/73x line → `STM32F732x?`               |
| 0x0450 | STM32H7x3 line (H743/H753/H750) → `STM32H743x?`  |

**H743 vs H750?** Both have `DEV_ID=0x0450`. Use Flash size:

* **128 KB** → `STM32H750xB`
* **2048 KB** → `STM32H743xI` (also valid for H753)

---

## Density Letter Mapping

Use the KB value you read to select the density:

| KB (min) | Density |
| -------: | :------ |
|     2048 | xI      |
|     1536 | xH      |
|     1024 | xG      |
|      512 | xE      |
|      256 | xC      |
|      128 | xB      |

> This table is intentionally coarse and works across F4/F7/G4/H7 lines we use. Extend if you encounter other sizes.

---

## Windows + Make/CMake Integration

Add a target that runs the picker and then flashes using the exported `JLINK_DEVICE`:

**CMake example**

```cmake
add_custom_target(flash
  COMMAND ${CMAKE_COMMAND} -E env POWERSHELL_EXECUTABLE=powershell
          powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/tools/jlink/pick_device.ps1
  COMMAND JLink.exe -device $ENV{JLINK_DEVICE} -if SWD -speed 4000 -autoconnect 1 \
          -CommanderScript ${CMAKE_SOURCE_DIR}/tools/jlink/program.jlink
  USES_TERMINAL
)
```

**Make example (GNU/Linux/macOS)**

```make
flash:
	@tools/jlink/pick_device.sh
	@JLinkExe -device $$JLINK_DEVICE -if SWD -speed 4000 -autoconnect 1 \
	  -CommanderScript tools/jlink/program.jlink
```

---

## Troubleshooting & Notes

* **Always 16‑bit read** for Flash size (`mem16`). 32‑bit reads can return bogus values on some families.
* If `DBGMCU_IDCODE` read returns **0x00000000** at `0x5C001000` (H7), we also read `0xE0042000`. Some boot states/low‑power modes zero the H7 address.
* If **no Flash size was read**, make sure the target is **halted** and not in a protected bootloader.
* **REV_ID** can be useful to gate workarounds for specific errata, but it’s not needed for device selection.
* **Package letter** (e.g., `…ZI`, `…VI`) doesn’t affect J‑Link’s flash algorithm choice; ignore it.
* **Crypto presence** (e.g., H753 vs H743) isn’t relevant for flashing. Density split is enough.

---

## Security/Production Tips

* For production, log: timestamp, `DEV_ID`, `REV_ID`, Flash KB, chosen device, and UID words (optional).
* Optional: Read **UID (96‑bit)** and store it alongside build info.

  * F4 example base: `0x1FFF7A10` (read 3×32‑bit words).
  * H7 has a different UID base but same 3×32‑bit pattern.
* Consider adding a **mass‑erase** step before programming if devices can be previously used.

---

## Appendix: Sample `program.jlink`

Adjust addresses and artifacts to your project (QSPI vs internal flash, etc.):

```jlink
r
h
// Internal flash at 0x08000000
loadbin ../build/firmware.bin, 0x08000000
verifybin ../build/firmware.bin, 0x08000000
reset
q
```

---

## Maintenance

* Extend the **DEV_ID → line** table as we add new MCU families/variants.
* If we adopt L4/G0/G0B/etc., add their Flash‑size addresses and DEV_IDs.
* Keep `program.jlink` minimal and project‑specific: erase policy, verify policy, start address.

---

**End of doc.**
