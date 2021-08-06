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

#define NES20 20

typedef enum ppu_model {
  PPU_2C02 = 0,
  PPU_2C03 = 1,
  PPU_2C04A = 2,
  PPU_2C04B = 3,
  PPU_2C04C = 4,
  PPU_2C04D = 5,
  PPU_2C05A = 6,
  PPU_2C05B = 7,
  PPU_2C05C = 8,
  PPU_2C05D = 9,
  PPU_2C05E = 10
} NES_PPU_MODEL;

typedef enum mir_ {
  MIRROR_FOUR_SCREENS,
  MIRROR_VERTICAL,
  MIRROR_HORIZONTAL
} NES_MIRRORING_TYPE;


typedef enum reg_ {
  NTSC,
  PAL,
  DENDY,
  FAMICOM,
  VS_SYSTEM,
  PLAY_CHOISE,
  FDS,
  SYS_UNSPECIFIED
} NES_SYSTEM_TYPE;

typedef enum vs_system_type {
  VS_DEFAULT,
  RBIBASEBALL_PROTECTION,
  TKOBOXING_PROTECTION,
  SUPERXEVIOUS_PROTECTION,
  ICECLIMBER_PROTECTION,
  VS_DUAL_SYSTEM,
  RAIDONBUNGELINGBAY_PROTECTION
} NES_VS_SYSTEM_TYPE;


#define GET_NES_SYS                                                            \
  switch (rom->data[12] & 0x03) {                                              \
  case 0: rom->system_type = NTSC; break;                                      \
  case 1: rom->system_type = PAL; break;                                       \
  case 2: rom->system_type = NTSC; break;                                      \
  case 3: rom->system_type = DENDY; break;                                     \
  }

typedef enum inp_ {
  INP_UNSPECIFIED,
  STANDARDCONTROLLERS,
  FOURSCORE,
  FOURPLAYERADAPTER,
  VSSYSTEM,
  VSSYSTEMSWAPPED,
  VSSYSTEMSWAPAB,
  VSZAPPER,
  ZAPPER,
  TWOZAPPERS,
  BANDAIHYPERSHOT,
  POWERPADSIDEA,
  POWERPADSIDEB,
  FAMILYTRAINERSIDEA,
  FAMILYTRAINERSIDEB,
  ARKANOIDCONTROLLERNES,
  ARKANOIDCONTROLLERFAMICOM,
  DOUBLEARKANOIDCONTROLLER,
  KONAMIHYPERSHOT,
  PACHINKOCONTROLLER,
  EXCITINGBOXING,
  JISSENMAHJONG,
  PARTYTAP,
  OEKAKIDSTABLET,
  BARCODEBATTLER,
  MIRACLEPIANO,
  POKKUNMOGURAA,
  TOPRIDER,
  DOUBLEFISTED,
  FAMICOM3DSYSTEM,
  DOREMIKKOKEYBOARD,
  ROB,
  FAMICOMDATARECORDER,
  TURBOFILE,
  BATTLEBOX,
  FAMILYBASICKEYBOARD,
  PEC586KEYBOARD,
  BIT79KEYBOARD,
  SUBORKEYBOARD,
  SUBORKEYBOARDMOUSE1,
  SUBORKEYBOARDMOUSE2,
  SNESMOUSE,
  GENERICMULTICART,
  SNESCONTROLLERS,
  RACERMATEBICYCLE,
  UFORCE,
  LASTENTRY
} NES_INPUT_TYPE;

const char *NES_PPU_MODEL_STR[11];
const char *NES_MIRRORING_TYPE_STR[3];
const char *NES_SYSTEM_TYPE_STR[8];
const char *NES_VS_SYSTEM_TYPE_STR[7];
const char *NES_INPUT_TYPE_STR[47];

typedef struct nes_rom {
  uint8_t *data;
  size_t size;

  size_t PRG_size;
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

  uint8_t version;
} NES_ROM;

void ROM_load_from_disc(char *file_name, NES_ROM *rom);
void ROM_free(NES_ROM *rom);

void ROM_dump_CHR_to_BMP(NES_ROM *rom, uint8_t *palette);
#endif // NES_ROM_H