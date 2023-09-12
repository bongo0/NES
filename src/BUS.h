#ifndef BUS_H
#define BUS_H

#include "CPU_6502.h"
#include "PPU.h"
//#include "ROM.h"
#include "APU.h"
#include <stdint.h>

#if 1
#define CPU_RAM_SIZE 2048
#else
#define CPU_RAM_SIZE (1 << 16)
#endif

typedef struct bus {
  CPU_6502 cpu;
  // CPU RAM
  uint8_t ram[CPU_RAM_SIZE];
  
  APU apu;

  PPU ppu;
  // PPU direct memory access helpers
  uint8_t dma_page_adr;
  uint8_t dma_adr;
  uint8_t dma_data;
  uint8_t dma_transfer;
  uint8_t dma_sync;
  
  NES_ROM *rom;

  // shift registers representing the controller state
  uint8_t controller_state[2]; // internal
  // controller state for interfacing
  // bit    7 6 5       4     3   2     1     0
  // button	A	B	Select	Start	Up	Down	Left	Right
  uint8_t controller[2];
  uint64_t tick_counter;

  // trace log func
  // quite spaghetti - mamma mia 
  void* trace_data;
  void (*trace_log)(void*);

} NES_BUS;

void BUS_cpu_write(NES_BUS *nes, uint16_t adr, uint8_t data);
uint8_t BUS_cpu_read(NES_BUS *nes, uint16_t adr);

void BUS_load_rom(NES_BUS *nes, NES_ROM *rom);

void BUS_init(NES_BUS *nes, NES_ROM *rom);
void BUS_reset(NES_BUS *nes);
void BUS_tick(NES_BUS *nes);
void BUS_free(NES_BUS *nes);

#endif // BUS_H