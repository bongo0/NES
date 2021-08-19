#ifndef NES_ROM_H
#define NES_ROM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "mappers.h"
#include "common_types.h"

#define NES_ROM_SIZE 0x1000

#define NES_ROM_HEADER_SIZE 0x10

#define ROM_READ_ERROR 0
#define ROM_READ_SUCCESS 1

#define NES_ROM_TITLE "NES\x1A"

#define NES20 20

#define GET_NES_SYS                                                            \
  switch (rom->data[12] & 0x03) {                                              \
  case 0: rom->system_type = NTSC; break;                                      \
  case 1: rom->system_type = PAL; break;                                       \
  case 2: rom->system_type = NTSC; break;                                      \
  case 3: rom->system_type = DENDY; break;                                     \
  }

const char *NES_PPU_MODEL_STR[11];
const char *NES_MIRRORING_TYPE_STR[5];
const char *NES_SYSTEM_TYPE_STR[8];
const char *NES_VS_SYSTEM_TYPE_STR[7];
const char *NES_INPUT_TYPE_STR[47];



// NES ROM / CARTRIDGE
typedef struct {
  uint8_t version;
  
  uint8_t *data;
  size_t size;
  uint8_t *PRG_p;
  size_t PRG_size;
  uint8_t *CHR_p;
  size_t CHR_size; // if zero -> uses CHR RAM
  uint32_t save_ram_size;
  uint32_t work_ram_size;
  uint32_t chr_ram_size;
  uint32_t save_chr_ram_size;

  NES_MIRRORING_TYPE mirror_type;
  NES_SYSTEM_TYPE system_type;
  NES_INPUT_TYPE input_type;
  NES_VS_SYSTEM_TYPE vs_type;
  NES_PPU_MODEL ppu_model;

  uint8_t has_trainer;
  uint8_t has_battery;

  uint16_t mapper_id;
  uint8_t sub_mapper_id;
  Mapper mapper;
} NES_ROM;

void ROM_load_from_disc(char *file_name, NES_ROM *rom);
uint8_t ROM_cpu_read(NES_ROM *rom, uint16_t adr, uint8_t *data_out);
uint8_t ROM_cpu_write(NES_ROM *rom, uint16_t adr, uint8_t data);
uint8_t ROM_ppu_read(NES_ROM *rom, uint16_t adr, uint8_t *data_out);
uint8_t ROM_ppu_write(NES_ROM *rom, uint16_t adr, uint8_t data);
void ROM_reset_mapper(NES_ROM *rom);
void ROM_free(NES_ROM *rom);



void ROM_dump_CHR_to_BMP(NES_ROM *rom, uint8_t *palette);
#endif // NES_ROM_H