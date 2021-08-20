#ifndef MAPPERS_H
#define MAPPERS_H

#include <stdint.h>
#include <stdio.h>

#include "common_types.h"

#define PRG_BANK_SIZE 0x4000
typedef uint8_t (*map_f)(void *mapper, uint16_t adr, uint16_t *mapped_adr, uint8_t *data);
typedef void (*map_r)(void* mapper);
typedef NES_MIRRORING_TYPE (*map_m)(void *mapper);

typedef struct {
  map_f cpu_read;
  map_f cpu_write;
  map_f ppu_read;
  map_f ppu_write;
  map_m mirror_mode;
  map_r reset;
  void *state;
} Mapper;

typedef enum {
  MAP_FALSE= 0,    // mapper returns this if no mapping
  MAP_TRUE = 1<<0, // set if mapped
  MAP_RAM  = 1<<1  // set if mapper sets the data
} MAP_RETURN_VAL;

//#####################################
// Mapper 000
//#####################################
typedef struct {
  // mapper state
  size_t PRG_size;
  size_t CHR_size;
  NES_MIRRORING_TYPE mirror_mode;
} Mapper000;

uint8_t Mapper000_cpu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr, uint8_t *data);
uint8_t Mapper000_cpu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
uint8_t Mapper000_ppu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr, uint8_t *data);
uint8_t Mapper000_ppu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
NES_MIRRORING_TYPE Mapper000_mirror(void *mapper_state);
void Mapper000_reset(void *mapper_state);

//#####################################
// Mapper 001 aka MMC1
//#####################################
typedef struct {

  uint8_t CHR_bank_select_4low;
  uint8_t CHR_bank_select_4high;
  uint8_t CHR_bank_select8;

  uint8_t PRG_bank_select16low;
  uint8_t PRG_bank_select16high;
  uint8_t PRG_bank_select32;

  uint8_t load_register;
  uint8_t load_register_count;
  uint8_t control_register;


  uint8_t *RAM;

  size_t PRG_size;
  size_t CHR_size;
  NES_MIRRORING_TYPE mirror_mode;
} Mapper001;

uint8_t Mapper001_cpu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
uint8_t Mapper001_cpu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
uint8_t Mapper001_ppu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
uint8_t Mapper001_ppu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
NES_MIRRORING_TYPE Mapper001_mirror(void *mapper_state);
void Mapper001_reset(void *mapper_state);

//#####################################
// Mapper 002
//#####################################
typedef struct {
  // mapper state
  uint8_t PRG_bank_select_low;
	uint8_t PRG_bank_select_high;

  size_t PRG_size;
  size_t CHR_size;
  NES_MIRRORING_TYPE mirror_mode;
} Mapper002;

uint8_t Mapper002_cpu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
uint8_t Mapper002_cpu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
uint8_t Mapper002_ppu_read(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
uint8_t Mapper002_ppu_write(void *mapper_state, uint16_t adr, uint16_t *mapped_adr,uint8_t *data);
NES_MIRRORING_TYPE Mapper002_mirror(void *mapper_state);
void Mapper002_reset(void *mapper_state);

#endif // MAPPERS_H