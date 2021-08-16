#ifndef BUS_H
#define BUS_H

#include "CPU_6502.h"
#include "PPU.h"
#include "ROM.h"

#include <stdint.h>

#if 1
#define CPU_RAM_SIZE 2048
#else
#define CPU_RAM_SIZE (1 << 16)
#endif

typedef struct bus {
  CPU_6502 cpu;
  PPU ppu;
  NES_ROM *rom;
  uint8_t ram[CPU_RAM_SIZE]; // CPU RAM
  uint64_t tick_counter;
} NES_BUS;

void BUS_cpu_write(NES_BUS *nes, uint16_t adr, uint8_t data);
uint8_t BUS_cpu_read(NES_BUS *nes, uint16_t adr);

void BUS_load_rom(NES_BUS *nes, NES_ROM *rom);

void BUS_init(NES_BUS *nes, NES_ROM *rom);
void BUS_reset(NES_BUS *nes);
void BUS_tick(NES_BUS *nes);

#endif // BUS_H