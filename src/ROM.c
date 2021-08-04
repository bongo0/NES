#include "ROM.h"

const char *NES_PPU_MODEL_STR[] = {
    "PPU_2C02",  "PPU_2C03",  "PPU_2C04A", "PPU_2C04B",
    "PPU_2C04C", "PPU_2C04D", "PPU_2C05A", "PPU_2C05B",
    "PPU_2C05C", "PPU_2C05D", "PPU_2C05E",
};

const char *NES_MIRRORING_TYPE_STR[] = {
    "MIRROR_FOUR_SCREENS", "MIRROR_VERTICAL", "MIRROR_HORIZONTAL"};

const char *NES_SYSTEM_TYPE_STR[] = {
    "NTSC",      "PAL",         "DENDY", "FAMICOM",
    "VS_SYSTEM", "PLAY_CHOISE", "FDS",   "SYS_UNSPECIFIED"};

const char *NES_VS_SYSTEM_TYPE_STR[] = {"VS_DEFAULT",
                                               "RBIBASEBALL_PROTECTION",
                                               "TKOBOXING_PROTECTION",
                                               "SUPERXEVIOUS_PROTECTION",
                                               "ICECLIMBER_PROTECTION",
                                               "VS_DUAL_SYSTEM",
                                               "RAIDONBUNGELINGBAY_PROTECTION"};

const char *NES_INPUT_TYPE_STR[] = {"INP_UNSPECIFIED",
                                           "STANDARDCONTROLLERS",
                                           "FOURSCORE",
                                           "FOURPLAYERADAPTER",
                                           "VSSYSTEM",
                                           "VSSYSTEMSWAPPED",
                                           "VSSYSTEMSWAPAB",
                                           "VSZAPPER",
                                           "ZAPPER",
                                           "TWOZAPPERS",
                                           "BANDAIHYPERSHOT",
                                           "POWERPADSIDEA",
                                           "POWERPADSIDEB",
                                           "FAMILYTRAINERSIDEA",
                                           "FAMILYTRAINERSIDEB",
                                           "ARKANOIDCONTROLLERNES",
                                           "ARKANOIDCONTROLLERFAMICOM",
                                           "DOUBLEARKANOIDCONTROLLER",
                                           "KONAMIHYPERSHOT",
                                           "PACHINKOCONTROLLER",
                                           "EXCITINGBOXING",
                                           "JISSENMAHJONG",
                                           "PARTYTAP",
                                           "OEKAKIDSTABLET",
                                           "BARCODEBATTLER",
                                           "MIRACLEPIANO",
                                           "POKKUNMOGURAA",
                                           "TOPRIDER",
                                           "DOUBLEFISTED",
                                           "FAMICOM3DSYSTEM",
                                           "DOREMIKKOKEYBOARD",
                                           "ROB",
                                           "FAMICOMDATARECORDER",
                                           "TURBOFILE",
                                           "BATTLEBOX",
                                           "FAMILYBASICKEYBOARD",
                                           "PEC586KEYBOARD",
                                           "BIT79KEYBOARD",
                                           "SUBORKEYBOARD",
                                           "SUBORKEYBOARDMOUSE1",
                                           "SUBORKEYBOARDMOUSE2",
                                           "SNESMOUSE",
                                           "GENERICMULTICART",
                                           "SNESCONTROLLERS",
                                           "RACERMATEBICYCLE",
                                           "UFORCE",
                                           "LASTENTRY"};



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
    rom->version = NES20;

  rom->size = size; // total size

// get program size
#define PRG 4
  if (rom->version == NES20) {
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
  if (rom->version == NES20) {
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
  // get system info
  if (rom->version == NES20) {
    switch (rom->data[7] & 0x03) {
    case 0: GET_NES_SYS; break;
    case 1: rom->system_type = VS_SYSTEM; break;
    case 2: rom->system_type = PLAY_CHOISE; break;
    case 3:
      switch (rom->data[13]) {
      case 0: GET_NES_SYS; break;
      case 1: rom->system_type = VS_SYSTEM; break;
      case 2: rom->system_type = PLAY_CHOISE; break;
      default:
        LOG_WARNING("Unknown system type, using NTSC");
        rom->system_type = NTSC;
        break;
      }
      break;
    }
  } else {
    if (rom->data[7] & 0x01) {
      rom->system_type = VS_SYSTEM;
    } else if (rom->data[7] & 0x02) {
      rom->system_type = PLAY_CHOISE;
    } else {
      rom->system_type = (rom->data[9] & 0x01) ? PAL : SYS_UNSPECIFIED;
    }
  }

  // get mapper id
  if (rom->version == NES20) {
    rom->mapper_id = ((rom->data[8] & 0x0f) << 8) | (0xf0 & rom->data[7]) |
                     (rom->data[6] >> 4);
  } else {
    rom->mapper_id = (0xf0 & rom->data[7]) | (rom->data[6] >> 4);
  }

  // sub mapper
  if (rom->version == NES20) {
    rom->sub_mapper_id = (rom->data[8] & 0xF0) >> 4;
  } else {
    rom->sub_mapper_id = 0;
  }

  // input type
  if (rom->version == NES20) {
    if (rom->data[15] < LASTENTRY) {
      rom->input_type = rom->data[15];
    } else {
      rom->input_type = INP_UNSPECIFIED;
    }
  } else {
    rom->input_type = INP_UNSPECIFIED;
  }

  // vs system type
  if (rom->version == NES20) {
    if ((rom->data[13] >> 4) <= 0x06) {
      rom->vs_type = rom->data[13] >> 4;
    } else {
      LOG_WARNING("Unknown NES2.0 VS type %02x, setting to default\n",
                  rom->data[13] >> 4);
      rom->vs_type = VS_DEFAULT;
    }
  } else {
    rom->vs_type = VS_DEFAULT;
  }

  // vs system PPU model
  if (rom->version == NES20) {
    switch (rom->data[13] & 0x0f) {
    case 0: rom->ppu_model = PPU_2C03;
    case 1:
      LOG_WARNING("Unsupported PPU model 2C03");
      rom->ppu_model = PPU_2C03;
      break;
    case 2: rom->ppu_model = PPU_2C04A; break;
    case 3: rom->ppu_model = PPU_2C04B; break;
    case 4: rom->ppu_model = PPU_2C04C; break;
    case 5: rom->ppu_model = PPU_2C04D; break;
    case 6: rom->ppu_model = PPU_2C03; break;
    case 7: rom->ppu_model = PPU_2C03; break;
    case 8: rom->ppu_model = PPU_2C05A; break;
    case 9: rom->ppu_model = PPU_2C05B; break;
    case 10: rom->ppu_model = PPU_2C05C; break;
    case 11: rom->ppu_model = PPU_2C05D; break;
    case 12: rom->ppu_model = PPU_2C05E; break;
    default:
      LOG_WARNING("Unsupported PPU model %02x\n", rom->data[13] & 0x0f);
      break;
    }
  } else {
    rom->ppu_model = PPU_2C03;
  }

  // get save ram size
  if (rom->version == NES20) {
    uint8_t tmp = (rom->data[10] & 0xf0) >> 4;
    rom->save_ram_size = (tmp == 0) ? 0 : 128 * (uint32_t)(2 << (tmp - 1));
  } else {
    rom->save_ram_size = 0;
  }

  // get work ram size
  if (rom->version == NES20) {
    uint8_t tmp = rom->data[10] & 0x0f;
    rom->work_ram_size = (tmp == 0) ? 0 : 128 * (uint32_t)(2 << (tmp - 1));
  } else {
    rom->work_ram_size = 0;
  }

  // get chr ram size
  if (rom->version == NES20) {
    uint8_t tmp = rom->data[11] & 0x0f;
    rom->chr_ram_size = (tmp == 0) ? 0 : 128 * (uint32_t)(2 << (tmp - 1));
  } else {
    rom->chr_ram_size = 0;
  }
  // get save chr ram size
  if (rom->version == NES20) {
    uint8_t tmp = (rom->data[11] & 0xf0) >> 4;
    rom->save_chr_ram_size = (tmp == 0) ? 0 : 128 * (uint32_t)(2 << (tmp - 1));
  } else {
    rom->save_chr_ram_size = 0;
  }

  
  // validation stuff
  if ((rom->PRG_size + rom->CHR_size + (rom->has_trainer ? 512 : 0)) +
          NES_ROM_HEADER_SIZE >
      size) {
    LOG_ERROR("corrupted ROM file, larger than header tells\n");
  } else if ((rom->PRG_size + rom->CHR_size + (rom->has_trainer ? 512 : 0)) +
                 NES_ROM_HEADER_SIZE <
             size) {
    LOG_ERROR("corrupted ROM file, smaller than header tells\n");
  }

  // logging stuff
  LOG_SUCCESS(
      "loaded ROM ver %s %s\n"
      "\ttotal %lu bytes\n"
      "\tPRG: %lu\n"
      "\tCHR: %lu\n"
      "\tregion: %s\n"
      "\thas battery: %s\n"
      "\thas trainer: %s\n"
      "\tmapper_id: %u\n"
      "\tsub_mapper_id: %u\n"
      "\tmirror type: %s\n"
      "\tinput type: %s\n"
      "\tvs type: %s\n"
      "\tppu model: %s\n"
      "\tsave ram size %u\n"
      "\twork ram size %u\n"
      "\tchr ram size %u\n"
      "\tsave chr ram size %u\n",
      (rom->version == NES20 ? "NES2.0" : "iNES"), file_name, size,
      rom->PRG_size, rom->CHR_size, NES_SYSTEM_TYPE_STR[rom->system_type],
      rom->has_battery ? "yes" : "no", rom->has_trainer ? "yes" : "no",
      rom->mapper_id, rom->sub_mapper_id,
      NES_MIRRORING_TYPE_STR[rom->mirror_type],
      NES_INPUT_TYPE_STR[rom->input_type], NES_VS_SYSTEM_TYPE_STR[rom->vs_type],
      NES_PPU_MODEL_STR[rom->ppu_model], rom->save_ram_size, rom->work_ram_size,
      rom->chr_ram_size, rom->save_chr_ram_size);
}

void ROM_free(NES_ROM *rom) { free(rom->data); }
