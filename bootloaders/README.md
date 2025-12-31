# UF2 Bootloaders

Pre-built UF2 bootloaders for STM32 flight controller boards. These bootloaders enable drag-and-drop firmware updates via USB mass storage.

## Available Bootloaders

| Target | MCU | HSE | File |
|--------|-----|-----|------|
| BlackPill F411CE | STM32F411CE | **8MHz** | `bootloader-blackpill_f411ce_8mhz-v*.bin` |
| NOXE V3 | STM32F411CE | 8MHz | `bootloader-noxe_v3-v*.bin` |
| Revolution F405 | STM32F405RG | 8MHz | `bootloader-revo_f405-v*.bin` |
| NERO F7 | STM32F722RE | 8MHz | `bootloader-nero_f7-v*.bin` |
| MATEK H743 | STM32H743VI | 8MHz | `bootloader-matek_h743-v*.bin` |

> **Important:** Bootloaders are HSE (crystal) specific. The BlackPill bootloader is for **8MHz crystal** boards (WeAct Studio). Boards with 25MHz crystals require a different bootloader build.

## Flashing the Bootloader

### Via J-Link

```bash
JLinkExe -device STM32F411CE -if SWD -speed 4000 -autoconnect 1
> loadbin bootloader-blackpill_f411ce_8mhz-v1.0.0.bin 0x08000000
> r
> g
> exit
```

### Via ST-Link (STM32CubeProgrammer)

```bash
STM32_Programmer_CLI -c port=SWD -w bootloader-blackpill_f411ce_8mhz-v1.0.0.bin 0x08000000 -v -rst
```

### Via Arduino IDE

1. Select your board in **Tools > Board**
2. Select programmer in **Tools > Programmer** (J-Link or ST-Link)
3. Click **Tools > Burn Bootloader**

## Bootloader Entry Methods

| Method | How | When to Use |
|--------|-----|-------------|
| **Button hold** | Hold KEY/BOOT0 while pressing RESET | Primary method, works without app |
| **CLI command** | Type `bl` in Serial Monitor | When app is running |
| **No valid app** | Automatic | Flash is empty or corrupted |

### Button Hold Entry (Primary)

Works at bootloader level - no application required:

1. Hold the KEY button (usually PA0 or BOOT0)
2. Press and release RESET
3. Release KEY button
4. USB drive appears (e.g., `STM32F4BOOT`)

### CLI Command Entry

Works from running application with USB CDC serial:

1. Open Serial Monitor (115200 baud)
2. Type `bl` and press Enter
3. Application writes magic value and resets
4. USB drive appears

## Using the Bootloader

Once in bootloader mode:

1. A USB mass storage drive appears on your computer
2. Drag and drop a `.uf2` firmware file onto the drive
3. The bootloader automatically flashes and reboots

## Building UF2 Firmware

Arduino sketches are automatically converted to UF2 format when using the **UF2 Bootloader** upload method:

1. Select **Tools > Upload Method > UF2 Bootloader**
2. Click Upload (or use the upload button)
3. Arduino IDE creates `.uf2` file and copies it to the bootloader drive

## Troubleshooting

### Drive doesn't appear

- Ensure bootloader is flashed correctly (verify with J-Link/ST-Link)
- Try a different USB cable (some are charge-only)
- Check USB connection on the board

### Upload fails

- Ensure the `.uf2` file is for the correct board family
- Wait for the drive to fully mount before copying
- Try ejecting and re-entering bootloader mode

### Application doesn't start

- Verify the application was compiled with correct flash offset (0x10000 for most boards)
- Check that `BL_TINYUF2` build flag is set in boards.txt

## Technical Details

| Property | Value |
|----------|-------|
| Bootloader size | ~20-28 KB |
| App start address | 0x08010000 (64KB offset) |
| UF2 Family IDs | F4: 0x57755a57, F7: 0x53b80f00, H7: 0x6db66082 |
| Double-tap magic | 0xf01669ef |
