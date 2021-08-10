#ifndef PPU_H
#define PPU_H

#include <stdint.h>

typedef struct ppu {
  int _;
} PPU;

typedef enum {
  PALETTE_2C02 = 0,
  PALETTE_2C03 = 1,
  PALETTE_2C04_0001 = 2,
  PALETTE_2C04_0002 = 3,
  PALETTE_2C04_0003 = 4,
  PALETTE_2C04_0004 = 5,
  PALETTE_2C05_01 = 6,
  PALETTE_2C05_02 = 7,
  PALETTE_2C05_03 = 8,
  PALETTE_2C05_04 = 9,
  PALETTE_2C05_05 = 10
} PPU_PALETTE;

uint32_t PPU_PALETTE_ARGB[11][64];

#endif // PPU_H