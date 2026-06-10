# UF2 Bootloaders

Pre-built UF2 bootloaders for STM32 flight controller boards. These bootloaders enable drag-and-drop firmware updates and INI config storage via USB mass storage.

## Available Bootloaders

| Target | MCU | HSE | File |
|--------|-----|-----|------|
| NOXE V3 | STM32F411CE | 8MHz | `bootuf2-noxe_v3-v*.bin` |
| Revolution F405 | STM32F405RG | 8MHz | `bootuf2-revo_f405-v*.bin` |
| NERO F7 | STM32F722RE | 8MHz | `bootuf2-nero_f7-v*.bin` |
| BetaFPV G473 | STM32G473CE | 8MHz | `bootuf2-betafpv_g473-v*.bin` |
| MATEK H743 | STM32H743VI | 8MHz | `bootuf2-matek_h743-v*.bin` |

Each target ships in two formats with identical content:

- **`.bin`** — used by the Arduino IDE "Burn Bootloader" flow and direct J-Link/ST-Link flashing (load address `0x08000000`).
- **`.hex`** — same image in Intel HEX form (load address embedded). Provided for tools that do not accept raw binaries, notably betaflight-configurator (see below).

Both are produced and version-stamped by `build_sync_bootloaders.sh`.

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

### Via betaflight-configurator (replacing Betaflight, no SWD probe)

Use the `.hex` artifact to install the bootloader over USB on a board currently
running Betaflight, without a J-Link/ST-Link. betaflight-configurator accepts
`.hex`/`.uf2` but **not** raw `.bin`.

1. **Firmware Flasher** tab → **Load Firmware [Local]** → select `bootuf2-<target>-v<version>.hex`.
2. Connect; the configurator reboots the FC into the STM32 ROM DFU (`0483:df11`) — no BOOT0 jumper needed.
3. **Enable "Full chip erase".** This is mandatory: a partial (page-only) erase
   leaves the old Betaflight application at the app slot, which the UF2 bootloader's
   validity check then jumps into, bricking boot. A full erase clears the app slot
   so the bootloader stays in UF2 mode.
4. **Flash Firmware.** The board reboots into the UF2 bootloader and enumerates as
   a mass-storage drive (`239a:006f`), ready for `.uf2` application uploads.

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
