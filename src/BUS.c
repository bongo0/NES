#include "BUS.h"

void BUS_cpu_write(NES_BUS *nes, uint16_t adr, uint8_t data) {

  if (ROM_cpu_write(nes->rom, adr, data))
    return;

  if (adr <= 0x1fff) {
    nes->ram[adr & 0x07ff] = data; // mirror the range every 2048
  } else if (adr >= 0x2000 && adr <= 0x3fff) {

    PPU_cpu_write(&nes->ppu, adr & 0x0007, data);
  }
}

uint8_t BUS_cpu_read(NES_BUS *nes, uint16_t adr) {

  uint8_t ret = 0;
  if (ROM_cpu_read(nes->rom, adr, &ret))
    return ret;

  if (adr <= 0x1fff) {
    return nes->ram[adr & 0x07ff]; // mirror the range every 2048
  } else if (adr >= 0x2000 && adr <= 0x3fff) {
    return PPU_cpu_read(&nes->ppu, adr & 0x0007);
  }
  return 0;
}