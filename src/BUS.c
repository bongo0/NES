#include "BUS.h"

void BUS_cpu_write(NES_BUS *nes, uint16_t adr, uint8_t data) {

  ROM_cpu_write(nes->rom, adr, data);
  //return;

  if (adr <= 0x1fff) {
    nes->ram[adr & 0x07ff] = data; // mirror the range every 2048
  } else if (adr >= 0x2000 && adr <= 0x3fff) {
    PPU_cpu_write(&nes->ppu, adr & 0x0007, data);
  } else if (adr == 0x4014) {
    nes->dma_page_adr = data;
    nes->dma_adr = 0;
    nes->dma_transfer = 1;
  } else if (adr == 0x4016 || adr == 0x4017) {
    nes->controller_state[adr & 1] = nes->controller[adr & 1];
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
  } else if (adr == 0x4016 || adr == 0x4017) {
    ret = (nes->controller_state[adr & 1] & 0x80) > 0;
    nes->controller_state[adr & 1] <<= 1;
  }
  return ret;
}

void BUS_init(NES_BUS *nes, NES_ROM *rom) {
  nes->rom = rom;

  PPU_init(&nes->ppu, rom);
  nes->tick_counter = 0;
  nes->dma_adr = 0;
  nes->dma_data = 0;
  nes->dma_page_adr = 0;
  nes->dma_sync = 1;
  nes->dma_transfer = 0;
  PPU_reset(&nes->ppu);
  ROM_reset_mapper(nes->rom);

  CPU_init(&nes->cpu, nes);// dont reset cpu here
}

void BUS_reset(NES_BUS *nes) {
  nes->tick_counter = 0;
  nes->dma_adr = 0;
  nes->dma_data = 0;
  nes->dma_page_adr = 0;
  nes->dma_sync = 1;
  nes->dma_transfer = 0;
  PPU_reset(&nes->ppu);
  ROM_reset_mapper(nes->rom);
  
  CPU_reset(&nes->cpu);
}
//static uint8_t test=1;
//static uint32_t test2=0;
void BUS_tick(NES_BUS *nes) {
  PPU_tick(&nes->ppu);
  // CPU runs every 3 PPU cycles
  if (nes->tick_counter % 3 == 0) {
    // dont tick if dma transfer is going on
    if (nes->dma_transfer) {
      
      // ticks every 2 cpu clock
      if (nes->dma_sync) {
        if (nes->tick_counter % 2 == 1)
          nes->dma_sync = 0;
      } else {
        // do the dma transfer
        if (nes->tick_counter % 2 == 0) {
          nes->dma_data =
              BUS_cpu_read(nes, (nes->dma_page_adr << 8) | (nes->dma_adr));
              //printf("%d dma read adr:%d    cycle:%lu\n",test2++,nes->dma_adr,nes->tick_counter);
        } else {
          nes->ppu.OAM[nes->dma_adr] = nes->dma_data;
          nes->dma_adr++;
          if(nes->dma_adr==0){
            nes->dma_transfer=0;
            nes->dma_sync=1;
            //if(test) printf("===================\n");
            //test=0;
          }
        }
      }
    } else {
      CPU_tick(&nes->cpu);
    }
  }

  // PPU emit interupt
  if (nes->ppu.state.nmi) {
    // printf("CPU NMI ");
    nes->ppu.state.nmi = 0;
    CPU_NMI(&nes->cpu);
  }
  nes->tick_counter++;
}



void BUS_free(NES_BUS *nes){
  
}