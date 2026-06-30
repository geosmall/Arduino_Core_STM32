# UF2 Bootloaders

Pre-built UF2 bootloaders for STM32 flight controller boards. These bootloaders enable drag-and-drop firmware updates and INI config storage via USB mass storage.

## Available Bootloaders

| Target | MCU | HSE | USB ID | File |
|--------|-----|-----|--------|------|
| NOXE V3 | STM32F411CE | 8MHz | `cafe:d411` | `bootuf2-noxe_v3-v*.bin` |
| Revolution F405 | STM32F405RG | 8MHz | `cafe:d405` | `bootuf2-revo_f405-v*.bin` |
| NERO F7 | STM32F722RE | 8MHz | `cafe:d722` | `bootuf2-nero_f7-v*.bin` |
| BetaFPV G473 | STM32G473CE | 8MHz | `cafe:d473` | `bootuf2-betafpv_g473-v*.bin` |
| MATEK H743 | STM32H743VI | 8MHz | `cafe:d743` | `bootuf2-matek_h743-v*.bin` |

The USB identity is `cafe` (VID) with a per-board PID (above); in bootloader mode the
device reports manufacturer string `bootuf2`.

Each target ships as `.bin` and `.hex` (identical image, load address `0x08000000`),
produced and version-stamped by `build_sync_bootloaders.sh`:

- **`.bin`** — used by all flashing methods below (Arduino IDE, J-Link, dfu-util).
- **`.hex`** — same image in Intel HEX form, provided for downstream/external tools
  that consume Intel HEX (e.g. STM32CubeProgrammer).

> **Important:** Bootloaders are HSE (crystal) specific. Boards with different crystal frequencies require a different bootloader build.

## Shared Header

`ini_flash_config.h` is the shared INI config flash format used by both bootloader and application. It is copied here by `build_sync_bootloaders.sh` and included via `-I{runtime.platform.path}/bootloaders` in `platform.txt`.

## Flashing the Bootloader

### Via Arduino IDE

1. Select your board in **Tools > Board**
2. Select programmer in **Tools > Programmer** (J-Link or ST-Link)
3. Click **Tools > Burn Bootloader**

### Via J-Link

```bash
JLinkExe -device <mcu> -if SWD -speed 4000 -autoconnect 1
> erase
> loadbin bootuf2-<target>-v<version>.bin 0x08000000
> r
> g
> exit
```

### Via dfu-util (USB, no SWD probe — e.g. converting a Betaflight board)

Installs the bootloader over USB on a board currently running Betaflight, without a
J-Link/ST-Link. Uses the `.bin`.

1. Enter the STM32 ROM DFU bootloader: in the Betaflight CLI type `bl` (or hold BOOT0
   and power on). The board enumerates as `0483:df11`.
2. Mass-erase, then download with reset:
   ```bash
   dfu-util -a 0 -s 0x08000000:mass-erase:force
   dfu-util -R -a 0 --dfuse-address 0x08000000 -D bootuf2-<target>-v<version>.bin
   ```
   The mass-erase is required: it clears the old Betaflight application at the app slot,
   so bootuf2 finds an empty slot and stays in UF2 mode instead of jumping into stale
   firmware. The board then reboots into the UF2 bootloader and mounts as a mass-storage
   drive (`cafe:<pid>` — see the USB ID column above), ready for `.uf2` application uploads.

> **Note:** betaflight-configurator's DFU flasher is **not** a reliable way to install
> bootuf2 — on STM32G4 its DFU flash aborts ("addresses not found" / failed address load)
> from inconsistent descriptor handling, even though the image is valid. Use `dfu-util`.

## Bootloader Entry Methods

| Method | How | When to Use |
|--------|-----|-------------|
| **Button hold** | Hold KEY/BOOT0 while pressing RESET | Primary method, works without app |
| **CLI command** | Type `bl` in Serial Monitor | When app is running |
| **No valid app** | Automatic | Flash is empty or corrupted |

## Using the Bootloader

Once in bootloader mode, a USB mass storage drive appears with:

- `INFO_UF2.TXT` — bootloader version and board info
- `CURRENT.UF2` — current application firmware (read back)
- `CONFIG.INI` — current INI config from flash (read back)

### Firmware upload
Drag and drop a `.uf2` firmware file onto the drive. The bootloader flashes and reboots automatically.

### Config upload
Config is uploaded as a UF2 file container (not raw `.ini`). Use `bootuf2/tools/ini2uf2.py` to wrap, or the Arduino IDE "Upload config.ini" method. Config write does **not** reset the board — the drive stays mounted.

## Technical Details

| Property | F4 | F7 | G4 | H7 |
|----------|----|----|----|----|
| App start address | 0x08010000 | 0x08010000 | 0x08010000 | 0x08020000 |
| UF2 Family ID | 0x57755a57 | 0x53b80f00 | 0x4c71240a | 0x6db66082 |
| Bootloader size | ~21 KB | ~22 KB | ~21 KB | ~35 KB |

Double-tap magic: `0xf01669ef` (all families)

## License & Attribution

These bootloaders are based on [TinyUF2](https://github.com/adafruit/tinyuf2) and [TinyUSB](https://github.com/hathach/tinyusb), both under MIT License.

**TinyUF2** - Copyright (c) 2020 Ha Thach
**TinyUSB** - Copyright (c) 2018, hathach (tinyusb.org)
