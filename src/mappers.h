#ifndef MAPPERS_H
#define MAPPERS_H

#include <stdint.h>

typedef uint8_t(*map_f)(void * mapper,uint16_t adr, uint16_t *mapped_adr);

typedef struct {
  map_f cpu_read;
  map_f cpu_write;
  map_f ppu_read;
  map_f ppu_write;
  void *state;
} Mapper;

typedef struct  {
  // mapper state
  // mapper 000 does not have registers
  uint8_t PRG_size;
  uint8_t CHR_size;
} Mapper000;

uint8_t Mapper000_cpu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr);
uint8_t Mapper000_cpu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr);
uint8_t Mapper000_ppu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr);
uint8_t Mapper000_ppu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr);

#endif // MAPPERS_H