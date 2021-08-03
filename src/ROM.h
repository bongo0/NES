#ifndef NES_ROM_H
#define NES_ROM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

#define NES_ROM_SIZE 0x1000

#define NES_ROM_HEADER_SIZE 0x10

#define ROM_READ_ERROR 0
#define ROM_READ_SUCCESS 1

#define NES_ROM_TITLE "NES\x1A"

typedef enum mir_ {
  MIRROR_FOUR_SCREENS,
  MIRROR_VERTICAL,
  MIRROR_HORIZONTAL
} NES_MIRRORING_TYPE;

typedef enum reg_ { NTSC, PAL, DENDY, UNSPECIFIED } NES_REGION_TYPE;

typedef struct nes_rom {
  uint8_t *data;
  size_t size;

  size_t PRG_size;
  size_t CHR_size;

  NES_MIRRORING_TYPE mirror_type;
  NES_REGION_TYPE region_type;
  uint8_t has_trainer;
  uint8_t has_battery;

  uint16_t mapper_id;

  uint8_t version;
} NES_ROM;

void ROM_load_from_disc(char *file_name, NES_ROM *rom) {
  rom->version = 0;
  FILE *f;
  f = fopen(file_name, "rb");
  if (!f) {
    rom->data = NULL;
    rom->size = 0;
    LOG_ERROR("cannot open file %s\n", file_name);
    return;
  }

  const int end = fseek(f, 0, SEEK_END);
  if (end == -1) {
    rom->data = NULL;
    rom->size = 0;
    LOG_ERROR("cannot read file %s\n", file_name);
    fclose(f);
    return;
  }
  const long int size = ftell(f);
  if (size == -1) {
    rom->data = NULL;
    rom->size = 0;
    LOG_ERROR("cannot read file %s\n", file_name);
    fclose(f);
    return;
  }
  const int fseek_set_value = fseek(f, 0, SEEK_SET);
  if (fseek_set_value == -1) {
    rom->data = NULL;
    rom->size = 0;
    LOG_ERROR("cannot read file %s\n", file_name);
    fclose(f);
    return;
  }

  rom->data = malloc(size);
  if (rom->data == NULL) {
    rom->data = NULL;
    rom->size = 0;
    LOG_ERROR("cannot allocate %lu bytes for ROM %s\n", size, file_name);
    fclose(f);
    return;
  }

  const int64_t n_read = fread(rom->data, 1, size, f);
  fclose(f);

  if (n_read != size) {
    free(rom->data);
    rom->data = NULL;
    rom->size = 0;
    LOG_ERROR("cannot read file %s\n", file_name);
    return;
  }

  if (memcmp(NES_ROM_TITLE, rom->data, 4) != 0) {
    free(rom->data);
    rom->data = NULL;
    rom->size = 0;
    LOG_ERROR("Not a .NES file %s\n", file_name);
    return;
  }
  if ((rom->data[7] & 0x0c) == 0x08)
    rom->version = 20;

  rom->size = size;

// get program size
#define PRG 4
  if (rom->version == 20) {
    if ((rom->data[9] & 0x0f) == 0x0f) {
      uint8_t exp = rom->data[PRG] >> 2;
      uint8_t x = rom->data[PRG] & 0x03;
      x = 2 * x + 1;
      rom->PRG_size = x * (1 << exp);
    } else {
      rom->PRG_size = (((rom->data[9] & 0x0f) << 8) | rom->data[PRG]) * 0x4000;
    }
  } else {
    if (rom->data[PRG] == 0)
      rom->PRG_size = 256 * 0x4000;
    else
      rom->PRG_size = rom->data[PRG] * 0x4000;
  }

  // get chr size
#define CHR 5
  if (rom->version == 20) {
    if ((rom->data[9] & 0x0f) == 0x0f) {
      uint8_t exp = rom->data[CHR] >> 2;
      uint8_t x = rom->data[CHR] & 0x03;
      x = 2 * x + 1;
      rom->CHR_size = x * (1 << exp);
    } else {
      rom->CHR_size = (((rom->data[9] & 0x0f) << 8) | rom->data[CHR]) * 0x2000;
    }
  } else {
    rom->CHR_size = rom->data[CHR] * 0x2000;
  }

  // get mirroring type
  if (rom->data[6] & 0x08) {
    rom->mirror_type = MIRROR_FOUR_SCREENS;
  } else {
    rom->mirror_type =
        rom->data[6] & 0x01 ? MIRROR_VERTICAL : MIRROR_HORIZONTAL;
  }

  // get battery
  rom->has_battery = (rom->data[6] & 0x02) == 0x02;
  // get trainer
  rom->has_trainer = (rom->data[6] & 0x04) == 0x04;
  // get region info
  if (rom->version == 20) {
    switch (rom->data[12] & 0x03) {
    case 0: rom->region_type = NTSC; break;
    case 1: rom->region_type = PAL; break;
    case 2: rom->region_type = NTSC; break;
    case 3: rom->region_type = DENDY; break;
    }
  } else {
    rom->region_type = (rom->data[9] & 0x01) ? PAL : UNSPECIFIED;
  }
  // get mapper id
  if (rom->version == 20) {
    rom->mapper_id = ((rom->data[8] & 0x0f) << 8) | (0xf0 & rom->data[7]) |
                     (rom->data[6] >> 4);
  } else {
    rom->mapper_id = (0xf0 & rom->data[7]) | (rom->data[6] >> 4);
  }
  LOG_SUCCESS("loaded ROM ver %s %s %lu bytes PRG %lu CHR %lu\n",
              (rom->version == 20 ? "NES2.0" : "iNES"), file_name, size,
              rom->PRG_size, rom->CHR_size);
}

#endif // NES_ROM_H