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

void BUS_init(NES_BUS *nes, NES_ROM *rom){
  nes->rom=rom;
  nes->tick_counter=0;
  PPU_init(&nes->ppu, rom);
  CPU_init(&nes->cpu, nes);

  CPU_reset(&nes->cpu);
  PPU_reset(&nes->ppu);

}

void BUS_reset(NES_BUS *nes){
  nes->tick_counter=0;
  ROM_reset_mapper(nes->rom);
  CPU_reset(&nes->cpu);
  PPU_reset(&nes->ppu);
}

void BUS_tick(NES_BUS *nes){
  PPU_tick(&nes->ppu);

  // CPU runs every 3 PPU cycles
  if(nes->tick_counter%3==0){
    //printf("tick:%d cpucycle:%d\n",nes->tick_counter,nes->cpu.state.cycle_count);
    CPU_tick(&nes->cpu);
  }

  // PPU emit interupt
  if(nes->ppu.state.nmi){
      //printf("CPU NMI ");
    nes->ppu.state.nmi=0;
    CPU_NMI(&nes->cpu);
  }
  nes->tick_counter++;
}