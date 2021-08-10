#include "ROM.h"

const char *NES_PPU_MODEL_STR[] = {
    "PPU_2C02",  "PPU_2C03",  "PPU_2C04A", "PPU_2C04B",
    "PPU_2C04C", "PPU_2C04D", "PPU_2C05A", "PPU_2C05B",
    "PPU_2C05C", "PPU_2C05D", "PPU_2C05E",
};

const char *NES_MIRRORING_TYPE_STR[] = {"MIRROR_FOUR_SCREENS",
                                        "MIRROR_VERTICAL", "MIRROR_HORIZONTAL"};

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

void ROM_init_mapper(NES_ROM *rom) {
  switch (rom->mapper_id) {
  case 0:
    rom->mapper.cpu_read = &Mapper000_cpu_read;
    rom->mapper.cpu_write = &Mapper000_cpu_write;
    rom->mapper.ppu_read = &Mapper000_ppu_read;
    rom->mapper.ppu_write = &Mapper000_ppu_write;
    rom->mapper.state = malloc(sizeof(Mapper000));
    ((Mapper000 *)rom->mapper.state)->CHR_size = rom->CHR_size;
    ((Mapper000 *)rom->mapper.state)->PRG_size = rom->PRG_size;
    break;
  default: break;
  }
}


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
        LOG_WARNING("Unknown system type, using NTSC: %s\n", file_name);
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
      LOG_WARNING("Unsupported PPU model 2C03: %s\n", file_name);
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
    LOG_ERROR("corrupted ROM file, larger than header tells: %s\n", file_name);
    free(rom->data);
    rom->size = 0;
    rom->data = NULL;
    return;
  } else if ((rom->PRG_size + rom->CHR_size + (rom->has_trainer ? 512 : 0)) +
                 NES_ROM_HEADER_SIZE <
             size) {
    LOG_ERROR("corrupted ROM file, smaller than header tells: %s\n", file_name);
    free(rom->data);
    rom->size = 0;
    rom->data = NULL;
    return;
  }

  // set up CHR and PRG data pointers
  if (rom->CHR_size == 0) {
    rom->CHR_p = NULL;
  } else {
    rom->CHR_p = &rom->data[NES_ROM_HEADER_SIZE + (rom->has_trainer ? 512 : 0) +
                            rom->PRG_size];
  }
  rom->PRG_p = &rom->data[NES_ROM_HEADER_SIZE + (rom->has_trainer ? 512 : 0)];

  ROM_init_mapper(rom);

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

void ROM_free(NES_ROM *rom) {
  free(rom->data);
  free(rom->mapper.state);
  rom->size = 0;
  rom->data = NULL;
}

uint8_t *ROM_get_CHR_p(NES_ROM *rom) {
  return &rom->data[NES_ROM_HEADER_SIZE + (rom->has_trainer ? 512 : 0) +
                    rom->PRG_size];
}

uint8_t *ROM_get_PRG_p(NES_ROM *rom) {
  return &rom->data[NES_ROM_HEADER_SIZE + (rom->has_trainer ? 512 : 0)];
}

uint8_t ROM_cpu_read(NES_ROM *rom, uint16_t adr, uint8_t *data_out) {
  uint16_t map_adr = 0;
  if(rom->mapper.cpu_read(rom->mapper.state,adr,&map_adr)){
    (*data_out) = rom->PRG_p[map_adr];
    return 1;
  }
  return 0;
}

uint8_t ROM_cpu_write(NES_ROM *rom, uint16_t adr, uint8_t data) {
    uint16_t map_adr = 0;
  if(rom->mapper.cpu_write(rom->mapper.state,adr,&map_adr)){
    rom->PRG_p[map_adr] = data;
    return 1;
  }
  return 0;
}

uint8_t ROM_ppu_read(NES_ROM *rom, uint16_t adr, uint8_t *data_out) {
  uint16_t map_adr = 0;
  if(rom->mapper.ppu_read(rom->mapper.state,adr,&map_adr)){
    (*data_out) = rom->CHR_p[map_adr];
    return 1;
  }
  return 0;
}

uint8_t ROM_ppu_write(NES_ROM *rom, uint16_t adr, uint8_t data) {
    uint16_t map_adr = 0;
  if(rom->mapper.ppu_write(rom->mapper.state,adr,&map_adr)){
    rom->CHR_p[map_adr] = data;
    return 1;
  }
  return 0;
}

const char *PALETTE[] = {
    "\x80\x80\x80", "\x00\x3D\xA6", "\x00\x12\xB0", "\x44\x00\x96",
    "\xA1\x00\x5E", "\xC7\x00\x28", "\xBA\x06\x00", "\x8C\x17\x00",
    "\x5C\x2F\x00", "\x10\x45\x00", "\x05\x4A\x00", "\x00\x47\x2E",
    "\x00\x41\x66", "\x00\x00\x00", "\x05\x05\x05", "\x05\x05\x05",
    "\xC7\xC7\xC7", "\x00\x77\xFF", "\x21\x55\xFF", "\x82\x37\xFA",
    "\xEB\x2F\xB5", "\xFF\x29\x50", "\xFF\x22\x00", "\xD6\x32\x00",
    "\xC4\x62\x00", "\x35\x80\x00", "\x05\x8F\x00", "\x00\x8A\x55",
    "\x00\x99\xCC", "\x21\x21\x21", "\x09\x09\x09", "\x09\x09\x09",
    "\xFF\xFF\xFF", "\x0F\xD7\xFF", "\x69\xA2\xFF", "\xD4\x80\xFF",
    "\xFF\x45\xF3", "\xFF\x61\x8B", "\xFF\x88\x33", "\xFF\x9C\x12",
    "\xFA\xBC\x20", "\x9F\xE3\x0E", "\x2B\xF0\x35", "\x0C\xF0\xA4",
    "\x05\xFB\xFF", "\x5E\x5E\x5E", "\x0D\x0D\x0D", "\x0D\x0D\x0D",
    "\xFF\xFF\xFF", "\xA6\xFC\xFF", "\xB3\xEC\xFF", "\xDA\xAB\xEB",
    "\xFF\xA8\xF9", "\xFF\xAB\xB3", "\xFF\xD2\xB0", "\xFF\xEF\xA6",
    "\xFF\xF7\x9C", "\xD7\xE8\x95", "\xA6\xED\xAF", "\xA2\xF2\xDA",
    "\x99\xFF\xFC", "\xDD\xDD\xDD", "\x11\x11\x11", "\x11\x11\x11"};

//#define LOADBMP_IMPLEMENTATION
//#include "loadbmp.h"
#define COLOR_IDX(tile, row, pix)                                              \
  ((((CHR_p[16 * tile + row + 0] & (0x80 >> pix)) >> (7 - pix))) << 1 |        \
   ((CHR_p[16 * tile + row + 8] & (0x80 >> pix)) >> (7 - pix)))

void ROM_dump_CHR_to_BMP(NES_ROM *rom, uint8_t *palette) {
  uint8_t *CHR_p = ROM_get_CHR_p(rom);
  const uint8_t tile_size_bpm = 8 * 8 * 3;
  // const uint8_t tile_size_chr = 16;

  int32_t ntiles = rom->CHR_size / 16;
  LOG("BMP n-tiles: %d\n", ntiles);
  size_t bmp_file_size = tile_size_bpm * ntiles;

  uint8_t *BMP_buf = malloc(bmp_file_size);

  size_t buf_i = 0;
  for (int tile = 0; tile < ntiles; ++tile) {
    for (int row = 0; row < 8; ++row) {
      for (int pix = 0; pix < 8; ++pix) {
        uint8_t *color = &palette[COLOR_IDX(tile, row, pix)];
        BMP_buf[buf_i++] = color[0];
        BMP_buf[buf_i++] = color[1];
        BMP_buf[buf_i++] = color[2];
        // printf("%d ", COLOR_IDX(tile, row, pix));
      }
      // printf("\n");
    }
  }

  //  unsigned int err =
  //      loadbmp_encode_file("image.bmp", BMP_buf, 8, 8 * ntiles, LOADBMP_RGB);
  //
  //  if (err)
  //    LOG_ERROR("LoadBMP Load Error: %u\n", err);
}