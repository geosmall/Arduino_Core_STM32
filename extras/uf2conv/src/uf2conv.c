/*
 * UF2 Converter - Binary to UF2 format converter
 * Based on Microsoft's UF2 reference implementation (MIT License)
 * https://github.com/microsoft/uf2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define UF2_MAGIC_START0  0x0A324655UL
#define UF2_MAGIC_START1  0x9E5D5157UL
#define UF2_MAGIC_END     0x0AB16F30UL
#define UF2_FLAG_FAMILY   0x00002000UL

typedef struct {
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;
    uint32_t familyID;
    uint8_t  data[476];
    uint32_t magicEnd;
} UF2_Block;

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [options] <input.bin>\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -b <addr>  Base address (default: 0x08010000)\n");
    fprintf(stderr, "  -f <id>    Family ID (default: 0x57755a57 for STM32F4)\n");
    fprintf(stderr, "  -o <file>  Output file (default: flash.uf2)\n");
}

int main(int argc, char **argv) {
    uint32_t baseAddr = 0x08010000;
    uint32_t familyID = 0x57755a57;
    const char *outname = "flash.uf2";
    const char *inname = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            baseAddr = strtoul(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            familyID = strtoul(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outname = argv[++i];
        } else if (argv[i][0] != '-') {
            inname = argv[i];
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (!inname) { usage(argv[0]); return 1; }

    FILE *fin = fopen(inname, "rb");
    if (!fin) { fprintf(stderr, "Error: Cannot open %s\n", inname); return 1; }

    fseek(fin, 0L, SEEK_END);
    uint32_t fileSize = ftell(fin);
    fseek(fin, 0L, SEEK_SET);

    FILE *fout = fopen(outname, "wb");
    if (!fout) { fprintf(stderr, "Error: Cannot create %s\n", outname); fclose(fin); return 1; }

    UF2_Block bl;
    memset(&bl, 0, sizeof(bl));
    bl.magicStart0 = UF2_MAGIC_START0;
    bl.magicStart1 = UF2_MAGIC_START1;
    bl.magicEnd = UF2_MAGIC_END;
    bl.flags = UF2_FLAG_FAMILY;
    bl.familyID = familyID;
    bl.targetAddr = baseAddr;
    bl.payloadSize = 256;
    bl.numBlocks = (fileSize + 255) / 256;

    uint32_t blockNo = 0;
    while (fread(bl.data, 1, 256, fin) > 0) {
        bl.blockNo = blockNo++;
        fwrite(&bl, 1, sizeof(bl), fout);
        bl.targetAddr += 256;
        memset(bl.data, 0, sizeof(bl.data));
    }

    fclose(fout);
    fclose(fin);
    printf("Converted %s to %s (%d blocks, base 0x%08X, family 0x%08X)\n",
           inname, outname, blockNo, baseAddr, familyID);
    return 0;
}
