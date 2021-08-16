#include "mappers.h"
#include <stdio.h>
// ########################################################
//    MAPPER 000
// ########################################################

uint8_t Mapper000_cpu_read(void *mapper_state, uint16_t adr,
                           uint16_t *mapped_adr) {
  // if  PRG_size is 16KB
  //     CPU address Bus          PRG ROM
  //     0x8000 -> 0xBFFF: Map    0x0000 -> 0x3FFF
  //     0xC000 -> 0xFFFF: Mirror 0x0000 -> 0x3FFF
  // else if 32KB
  //     CPU address Bus          PRG ROM
  //     0x8000 -> 0xFFFF: Map    0x0000 -> 0x7FFF
  Mapper000 *mapper = (Mapper000 *)mapper_state;
  if (adr >= 0x8000 && adr <= 0xffff) {
    (*mapped_adr) = adr & (mapper->PRG_size > (1 << 14) ? 0x7FFF : 0x3FFF);
    return 1;
  }
  return 0;
}

uint8_t Mapper000_cpu_write(void *mapper_state, uint16_t adr,
                            uint16_t *mapped_adr) {
  Mapper000 *mapper = (Mapper000 *)mapper_state;
  if (adr >= 0x8000 && adr <= 0xffff) {
    (*mapped_adr) = adr & (mapper->PRG_size > (1 << 14) ? 0x7FFF : 0x3FFF);
    return 1;
  }
  return 0;
}

uint8_t Mapper000_ppu_read(void *mapper_state, uint16_t adr,
                           uint16_t *mapped_adr) {
    // no mapping needed
    if(adr>=0x000 && adr <= 0x1fff){
        (*mapped_adr)=adr;
        return 1;
    }
  return 0;
}

uint8_t Mapper000_ppu_write(void *mapper_state, uint16_t adr,
                            uint16_t *mapped_adr) {
    if(adr >= 0x0000 && adr <= 0x1fff){
        Mapper000 *mapper = (Mapper000 *)mapper_state;
        if(mapper->CHR_size == 0){
            // Using CHR RAM
            (*mapped_adr)=adr;
            return 1;
        }
    }
  return 0;
}



// ########################################################
//    MAPPER ---
// ########################################################
