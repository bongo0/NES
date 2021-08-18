#include "PPU.h"

void PPU_init(PPU *ppu, NES_ROM *rom) {
  ppu->state.data_buf = 0;
  ppu->state.adr_write_latch = 0;
  ppu->state.control.reg = 0;
  ppu->state.loopy.reg = 0;
  ppu->state.mask.reg = 0;
  ppu->state.data_buf = 0;
  ppu->state.sprite_ram_adr = 0;
  ppu->state.status.reg = 0;
  ppu->state.t_vram_adr.reg = 0;
  ppu->state.vram_adr.reg = 0;
  ppu->state.write_toggle = 0;
  ppu->state.x_scroll = 0;
  ppu->state.nmi = 0;

  ppu->scan_line = 0;
  ppu->cycle = 0;
  ppu->frame_count = 0;
  ppu->frame_complete = 0;
  ppu->master_clock = 0;

  ppu->next_bg_tile_id = 0;
  ppu->next_bg_tile_attrib = 0;
  ppu->next_bg_tile_lsb = 0;
  ppu->next_bg_tile_msb = 0;
  ppu->low_bg_shifter = 0;
  ppu->high_bg_shifter = 0;
  ppu->high_bg_shifter_attrib = 0;
  ppu->low_bg_shifter_attrib = 0;

  ppu->rom = rom;
}

void PPU_reset(PPU *ppu) {
  ppu->state.data_buf = 0;
  ppu->state.adr_write_latch = 0;
  ppu->state.control.reg = 0;
  ppu->state.loopy.reg = 0;
  ppu->state.mask.reg = 0;
  ppu->state.data_buf = 0;
  ppu->state.sprite_ram_adr = 0;
  ppu->state.status.reg = 0;
  ppu->state.t_vram_adr.reg = 0;
  ppu->state.vram_adr.reg = 0;
  ppu->state.write_toggle = 0;
  ppu->state.x_scroll = 0;
  ppu->state.nmi = 0;

  ppu->next_bg_tile_id = 0;
  ppu->next_bg_tile_attrib = 0;
  ppu->next_bg_tile_lsb = 0;
  ppu->next_bg_tile_msb = 0;
  ppu->low_bg_shifter = 0;
  ppu->high_bg_shifter = 0;
  ppu->high_bg_shifter_attrib = 0;
  ppu->low_bg_shifter_attrib = 0;

  ppu->scan_line = 0;
  ppu->cycle = 0;
  ppu->frame_count = 0;
  ppu->frame_complete = 0;
  ppu->master_clock = 0;
}

uint8_t PPU_cpu_read(PPU *ppu, uint16_t adr) {
  uint8_t data = 0x00;

  switch (adr) {
  case 0x0000: // Control
    break;
  case 0x0001: // Mask
    break;
  case 0x0002: // Status --- unused bits are noise usually what was last in the
               // data buffer
               //    ppu->state.status.VERTICAL_BLANK = 1;//tmp hack
    data = (ppu->state.status.reg & 0xe0) | (ppu->state.data_buf & 0x1f);
    ppu->state.status.VERTICAL_BLANK = 0;
    ppu->state.adr_write_latch = 0;
    break;
  case 0x0003: // OAM Address
    break;
  case 0x0004: // OAM Data
    break;
  case 0x0005: // Scroll
    break;
  case 0x0006: // PPU Address cant read
    break;
  case 0x0007: // PPU Data
    data = ppu->state.data_buf;
    ppu->state.data_buf = PPU_read(ppu, ppu->state.vram_adr.reg);

    // palette addressess not buffered
    if (ppu->state.vram_adr.reg >= 0x3f00)
      data = ppu->state.data_buf;

    // All writes from PPU data automatically increment the nametable
    // address depending upon the mode set in the control register.
    // If set to vertical mode, the increment is 32, so it skips
    // one whole nametable row; in horizontal mode it just increments
    // by 1, moving to the next column
    ppu->state.vram_adr.reg += (ppu->state.control.INCREMENT_MODE ? 32 : 1);
    break;
  }

  return data;
}

void PPU_cpu_write(PPU *ppu, uint16_t adr, uint8_t data) {
  switch (adr) {
  case 0x0000: // Control
    ppu->state.control.reg = data;
    ppu->state.t_vram_adr.nametable_x = ppu->state.control.NAMETABLE_X;
    ppu->state.t_vram_adr.nametable_y = ppu->state.control.NAMETABLE_Y;
    break;
  case 0x0001: // Mask
    ppu->state.mask.reg = data;
    break;
  case 0x0002: // Status -- cant write to this
    break;
  case 0x0003: // OAM Address
    break;
  case 0x0004: // OAM Data
    break;
  case 0x0005:                             // Scroll
    if (ppu->state.adr_write_latch == 0) { // flip flops between x/y
      ppu->state.x_scroll = data & 0x07;
      ppu->state.t_vram_adr.coarse_x = data >> 3;
      ppu->state.adr_write_latch = 1;
    } else {
      ppu->state.t_vram_adr.fine_y = data & 0x07;
      ppu->state.t_vram_adr.coarse_y = data >> 3;
      ppu->state.adr_write_latch = 0;
    }
    break;
  case 0x0006: // PPU Address
    if (ppu->state.adr_write_latch == 0) {
      ppu->state.t_vram_adr.reg =
          (uint16_t)(ppu->state.t_vram_adr.reg & 0x00ff) |
          (uint16_t)((data & 0x3f) << 8);
      ppu->state.adr_write_latch = 1;
    } else {
      ppu->state.t_vram_adr.reg = (ppu->state.t_vram_adr.reg & 0xff00) | data;
      ppu->state.vram_adr = ppu->state.t_vram_adr;
      ppu->state.adr_write_latch = 0;
    }
    break;
  case 0x0007: // PPU Data
    PPU_write(ppu, ppu->state.vram_adr.reg, data);
    // All writes from PPU data automatically increment the nametable
    // address depending upon the mode set in the control register.
    // If set to vertical mode, the increment is 32, so it skips
    // one whole nametable row; in horizontal mode it just increments
    // by 1, moving to the next column
    ppu->state.vram_adr.reg += (ppu->state.control.INCREMENT_MODE ? 32 : 1);
    break;
  }
}

uint8_t PPU_read(PPU *ppu, uint16_t adr) {
  adr &= 0x3fff;
  uint8_t data;
  if (ROM_ppu_read(ppu->rom, adr, &data))
    return data;

  if (adr <= 0x1fff) { // pattern
    return ppu->pattern_ram[adr];
  } else if (adr >= 0x2000 && adr <= 0x3eff) { // name table
    adr &= 0x0FFF;
    if (ppu->rom->mirror_type == MIRROR_VERTICAL) {
      if (adr >= 0x0000 && adr <= 0x03FF)
        return ppu->name_table[0 + (adr & 0x03FF)];
      if (adr >= 0x0400 && adr <= 0x07FF)
        return ppu->name_table[0x400 + (adr & 0x03FF)];
      if (adr >= 0x0800 && adr <= 0x0BFF)
        return ppu->name_table[0 + (adr & 0x03FF)];
      if (adr >= 0x0C00 && adr <= 0x0FFF)
        return ppu->name_table[0x400 + (adr & 0x03FF)];
    } else if (ppu->rom->mirror_type == MIRROR_HORIZONTAL) {
      if (adr >= 0x0000 && adr <= 0x03FF)
        return ppu->name_table[0 + (adr & 0x03FF)];
      if (adr >= 0x0400 && adr <= 0x07FF)
        return ppu->name_table[0 + (adr & 0x03FF)];
      if (adr >= 0x0800 && adr <= 0x0BFF)
        return ppu->name_table[0x400 + (adr & 0x03FF)];
      if (adr >= 0x0C00 && adr <= 0x0FFF)
        return ppu->name_table[0x400 + (adr & 0x03FF)];
    }
    // TODO rest mirroring modes
    return 0;
  } else if (adr >= 0x3f00 && adr <= 0x3fff) { // palette
    adr &= 0x001f;
    // mirroring addresses
    if (adr == 0x0010)
      adr = 0x0000;
    if (adr == 0x0014)
      adr = 0x0004;
    if (adr == 0x0018)
      adr = 0x0008;
    if (adr == 0x001C)
      adr = 0x000C;
    return ppu->palette_ram[adr]; //& (ppu->state.mask.grayscale ? 0x30 : 0x3F);
  }
  return 0;
}

void PPU_write(PPU *ppu, uint16_t adr, uint8_t data) {
  adr &= 0x3fff;

  if (ROM_ppu_write(ppu->rom, adr, data))
    return;

  if (adr <= 0x1fff) { // pattern
    ppu->pattern_ram[adr] = data;
  } else if (adr >= 0x2000 && adr <= 0x3eff) { // name table
    adr &= 0x0FFF;
    if (ppu->rom->mirror_type == MIRROR_VERTICAL) {
      if (adr >= 0x0000 && adr <= 0x03FF)
        ppu->name_table[0 + (adr & 0x03FF)] = data;
      if (adr >= 0x0400 && adr <= 0x07FF)
        ppu->name_table[0x400 + (adr & 0x03FF)] = data;
      if (adr >= 0x0800 && adr <= 0x0BFF)
        ppu->name_table[0 + (adr & 0x03FF)] = data;
      if (adr >= 0x0C00 && adr <= 0x0FFF)
        ppu->name_table[0x400 + (adr & 0x03FF)] = data;
    } else if (ppu->rom->mirror_type == MIRROR_HORIZONTAL) {
      if (adr >= 0x0000 && adr <= 0x03FF)
        ppu->name_table[0 + (adr & 0x03FF)] = data;
      if (adr >= 0x0400 && adr <= 0x07FF)
        ppu->name_table[0 + (adr & 0x03FF)] = data;
      if (adr >= 0x0800 && adr <= 0x0BFF)
        ppu->name_table[0x400 + (adr & 0x03FF)] = data;
      if (adr >= 0x0C00 && adr <= 0x0FFF)
        ppu->name_table[0x400 + (adr & 0x03FF)] = data;
    }
  } else if (adr >= 0x3f00 && adr <= 0x3fff) { // palette
    adr &= 0x001f;
    // mirroring addresses
    if (adr == 0x0010)
      adr = 0x0000;
    if (adr == 0x0014)
      adr = 0x0004;
    if (adr == 0x0018)
      adr = 0x0008;
    if (adr == 0x001C)
      adr = 0x000C;
    ppu->palette_ram[adr] = data;
    // printf("palette write adr:%04x val: %d\n", adr, data);
  }
}

uint32_t PPU_get_color_from_palette_ram(PPU *ppu, uint16_t palette_idx,
                                        uint8_t pixel_val) {
  uint8_t color_idx = PPU_read(ppu, 0x3f00 + (palette_idx << 2) + pixel_val) &
                      0x3f; /*0x3f00 = base palette address*/
  return PPU_PALETTE_RGBA[0][color_idx];
}

void PPU_load_pattern_table(PPU *ppu, uint8_t table_n /*0 or 1*/) {
  // uint32_t test[4]={0x222222ff, 0x888888ff, 0xaaaaaaff, 0xffffffff};
  for (int tile_y = 0; tile_y < 16; ++tile_y) {
    for (int tile_x = 0; tile_x < 16; ++tile_x) {

      uint16_t idx = 16 * 16 * tile_y + 16 * tile_x;
      for (int row = 0; row < 8; ++row) {
        uint8_t low = PPU_read(ppu, table_n * 0x1000 + idx + row + 0);
        uint8_t high = PPU_read(ppu, table_n * 0x1000 + idx + row + 8);
        for (int col = 0; col < 8; ++col) {
          int x = 8 * tile_x + 7 - col; // invert on x-dir
          int y = 8 * tile_y + row;
          ppu->pattern_table_img[128 * table_n + x + 128 * 2 * y] =
              PPU_get_color_from_palette_ram(ppu, 0, (low & 1) + (high & 1));
          ppu->pattern_table[128 * table_n + x + 128 * 2 * y] =
              (low & 1) + (high & 1); // the palette value 0,1,2 or 3.
          low >>= 1;
          high >>= 1;
        }
      }
    }
  }
}

// Increment the background tile "pointer" one tile/column horizontally
static inline void inc_scroll_x(PPU *ppu) {

  if (ppu->state.mask.RENDER_BACKGROUND || ppu->state.mask.RENDER_SPRITES) {
    if (ppu->state.vram_adr.coarse_x == 31) {
      ppu->state.vram_adr.coarse_x = 0;
      ppu->state.vram_adr.nametable_x = ~ppu->state.vram_adr.nametable_x;
    } else {
      ppu->state.vram_adr.coarse_x++;
    }
  }
}

static inline void inc_scroll_y(PPU *ppu) {
  if (ppu->state.mask.RENDER_BACKGROUND || ppu->state.mask.RENDER_SPRITES) {
    if (ppu->state.vram_adr.fine_y < 7) {
      ppu->state.vram_adr.fine_y++;
    } else {
      ppu->state.vram_adr.fine_y = 0;
      if (ppu->state.vram_adr.coarse_y == 29) {
        ppu->state.vram_adr.coarse_y = 0;
        ppu->state.vram_adr.nametable_y = ~ppu->state.vram_adr.nametable_y;
      } else if (ppu->state.vram_adr.coarse_y == 31) {
        ppu->state.vram_adr.coarse_y = 0;
      } else {
        ppu->state.vram_adr.coarse_y++;
      }
    }
  }
}

static inline void transfer_adr_x(PPU *ppu) {
  if (ppu->state.mask.RENDER_BACKGROUND || ppu->state.mask.RENDER_SPRITES) {
    {
      ppu->state.vram_adr.nametable_x = ppu->state.t_vram_adr.nametable_x;
      ppu->state.vram_adr.coarse_x = ppu->state.t_vram_adr.coarse_x;
    }
  }
}

static inline void transfer_adr_y(PPU *ppu) {
  if (ppu->state.mask.RENDER_BACKGROUND || ppu->state.mask.RENDER_SPRITES) {
    {
      ppu->state.vram_adr.fine_y = ppu->state.t_vram_adr.fine_y;
      ppu->state.vram_adr.nametable_y = ppu->state.t_vram_adr.nametable_y;
      ppu->state.vram_adr.coarse_y = ppu->state.t_vram_adr.coarse_y;
    }
  }
}

static inline void setup_shifters(PPU *ppu) {

  ppu->low_bg_shifter = (ppu->low_bg_shifter & 0xff00) | ppu->next_bg_tile_lsb;
  ppu->high_bg_shifter =
      (ppu->high_bg_shifter & 0xff00) | ppu->next_bg_tile_msb;

  ppu->low_bg_shifter_attrib =
      (ppu->low_bg_shifter_attrib & 0xFF00) |
      ((ppu->next_bg_tile_attrib & 0b01) ? 0xFF : 0x00);
  ppu->high_bg_shifter_attrib =
      (ppu->high_bg_shifter_attrib & 0xFF00) |
      ((ppu->next_bg_tile_attrib & 0b10) ? 0xFF : 0x00);
}

static inline void update_shifters(PPU *ppu) {
  if (ppu->state.mask.RENDER_BACKGROUND) {
    ppu->low_bg_shifter <<= 1;
    ppu->high_bg_shifter <<= 1;

    ppu->high_bg_shifter_attrib <<= 1;
    ppu->low_bg_shifter_attrib <<= 1;
  }
}

void PPU_tick(PPU *ppu) {
  // All but 1 of the secanlines is visible to the user. The pre-render
  // scanline at -1, is used to configure the "shifters" for the first visible
  // scanline, 0.
  if (ppu->scan_line >= -1 && ppu->scan_line < 240) {
    if (ppu->scan_line == 0 && ppu->cycle == 0) {
      // "Odd Frame" cycle skip
      ppu->cycle = 1;
    }
    if (ppu->scan_line == -1 && ppu->cycle == 1) {
      // Effectively start of new frame, so clear vertical blank flag
      ppu->state.status.VERTICAL_BLANK = 0;
    }
    // ...
    if ((ppu->cycle >= 2 && ppu->cycle < 258) ||
        (ppu->cycle >= 321 && ppu->cycle < 338)) {
      update_shifters(ppu);
      switch ((ppu->cycle - 1) % 8) {
      case 0:
        setup_shifters(ppu);
        // Fetch the next background tile ID
        // "(vram_addr.reg & 0x0FFF)" : Mask to 12 bits that are relevant
        // "| 0x2000"                 : Offset into nametable space on PPU
        // address bus
        ppu->next_bg_tile_id =
            PPU_read(ppu, 0x2000 | (ppu->state.vram_adr.reg & 0x0FFF));
        break;
      case 2:
        ppu->next_bg_tile_attrib =
            PPU_read(ppu, 0x23C0 | (ppu->state.vram_adr.nametable_y << 11) |
                              (ppu->state.vram_adr.nametable_x << 10) |
                              ((ppu->state.vram_adr.coarse_y >> 2) << 3) |
                              (ppu->state.vram_adr.coarse_x >> 2));
        if (ppu->state.vram_adr.coarse_y & 0x02)
          ppu->next_bg_tile_attrib >>= 4;
        if (ppu->state.vram_adr.coarse_x & 0x02)
          ppu->next_bg_tile_attrib >>= 2;
        ppu->next_bg_tile_attrib &= 0x03;
        break;
      case 4:
        ppu->next_bg_tile_lsb =
            PPU_read(ppu, (ppu->state.control.PATTERN_BACKGROUND << 12) +
                              ((uint16_t)ppu->next_bg_tile_id << 4) +
                              (ppu->state.vram_adr.fine_y) + 0);
        break;
      case 6:
        ppu->next_bg_tile_msb =
            PPU_read(ppu, (ppu->state.control.PATTERN_BACKGROUND << 12) +
                              ((uint16_t)ppu->next_bg_tile_id << 4) +
                              (ppu->state.vram_adr.fine_y) + 8);
        break;
      case 7: inc_scroll_x(ppu); break;
      }
    }
    if (ppu->cycle == 256)
      inc_scroll_y(ppu);
    if (ppu->cycle == 257) {
      setup_shifters(ppu);
      transfer_adr_x(ppu);
    }
    if (ppu->cycle == 338 || ppu->cycle == 340) {
      ppu->next_bg_tile_id =
          PPU_read(ppu, 0x2000 | (ppu->state.vram_adr.reg & 0x0FFF));
    }
    if (ppu->scan_line == -1 && ppu->cycle >= 280 && ppu->cycle < 305)
      transfer_adr_y(ppu);
  }

  if (ppu->scan_line >= 241 && ppu->scan_line < 261) {
    if (ppu->scan_line == 241 && ppu->cycle == 1) {
      ppu->state.status.VERTICAL_BLANK = 1;
      if (ppu->state.control.ENABLE_NMI)
        ppu->state.nmi = 1;
    }
  }

  // do composition
  uint8_t bg_pix = 0;
  uint8_t bg_pal = 0;
  if (ppu->state.mask.RENDER_BACKGROUND) {
    uint16_t scroll = 0x8000 >> ppu->state.x_scroll;
    uint8_t p0 = (ppu->low_bg_shifter & scroll) > 0;
    uint8_t p1 = (ppu->high_bg_shifter & scroll) > 0;

    bg_pix = (p1 << 1) | p0;
    uint8_t pal0 = (ppu->low_bg_shifter_attrib & scroll) > 0;
    uint8_t pal1 = (ppu->high_bg_shifter_attrib & scroll) > 0;
    bg_pal = (pal1 << 1) | pal0;
    if (ppu->cycle - 1 < 256 && ppu->scan_line < 240 && ppu->scan_line >= 0)
      ppu->screen[(/*X*/ ppu->cycle - 1) + 256 * (/*Y*/ ppu->scan_line)] =
          PPU_get_color_from_palette_ram(ppu, bg_pal, bg_pix);
  }
  // if(bg_pal!=0||bg_pix!=0)printf("pal: %d pix: %d",bg_pal,bg_pix);
  // set the pixel val to the screen
  // printf("pix: %d pal: %d\n",bg_pix,bg_pal);

  // advance counters
  ppu->cycle++;
  if (ppu->cycle >= 341) {
    ppu->cycle = 0;
    ppu->scan_line++;
    if (ppu->scan_line >= 261) {
      ppu->scan_line = -1;
      ppu->frame_complete = 1;
      ppu->frame_count++;
    }
  }
}

// clang-format off
uint32_t PPU_PALETTE_RGBA[11][64] = {
/* 2C02      */{ 0x666666FF, 0x002A88FF, 0x1412A7FF, 0x3B00A4FF, 0x5C007EFF, 0x6E0040FF, 0x6C0600FF, 0x561D00FF, 0x333500FF, 0x0B4800FF, 0x005200FF, 0x004F08FF, 0x00404DFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xADADADFF, 0x155FD9FF, 0x4240FFFF, 0x7527FEFF, 0xA01ACCFF, 0xB71E7BFF, 0xB53120FF, 0x994E00FF, 0x6B6D00FF, 0x388700FF, 0x0C9300FF, 0x008F32FF, 0x007C8DFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFEFFFF, 0x64B0FFFF, 0x9290FFFF, 0xC676FFFF, 0xF36AFFFF, 0xFE6ECCFF, 0xFE8170FF, 0xEA9E22FF, 0xBCBE00FF, 0x88D800FF, 0x5CE430FF, 0x45E082FF, 0x48CDDEFF, 0x4F4F4FFF, 0x000000FF, 0x000000FF, 0xFFFEFFFF, 0xC0DFFFFF, 0xD3D2FFFF, 0xE8C8FFFF, 0xFBC2FFFF, 0xFEC4EAFF, 0xFECCC5FF, 0xF7D8A5FF, 0xE4E594FF, 0xCFEF96FF, 0xBDF4ABFF, 0xB3F3CCFF, 0xB5EBF2FF, 0xB8B8B8FF, 0x000000FF, 0x000000FF },
/* 2C03      */{ 0x6D6D6DFF, 0x002491FF, 0x0000DAFF, 0x6D48DAFF, 0x91006DFF, 0xB6006DFF, 0xB62400FF, 0x914800FF, 0x6D4800FF, 0x244800FF, 0x006D24FF, 0x009100FF, 0x004848FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xB6B6B6FF, 0x006DDAFF, 0x0048FFFF, 0x9100FFFF, 0xB600FFFF, 0xFF0091FF, 0xFF0000FF, 0xDA6D00FF, 0x916D00FF, 0x249100FF, 0x009100FF, 0x00B66DFF, 0x009191FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0x6DB6FFFF, 0x9191FFFF, 0xDA6DFFFF, 0xFF00FFFF, 0xFF6DFFFF, 0xFF9100FF, 0xFFB600FF, 0xDADA00FF, 0x6DDA00FF, 0x00FF00FF, 0x48FFDAFF, 0x00FFFFFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0xB6DAFFFF, 0xDAB6FFFF, 0xFFB6FFFF, 0xFF91FFFF, 0xFFB6B6FF, 0xFFDA91FF, 0xFFFF48FF, 0xFFFF6DFF, 0xB6FF48FF, 0x91FF6DFF, 0x48FFDAFF, 0x91DAFFFF, 0x000000FF, 0x000000FF, 0x000000FF },
/* 2C04_0001 */{ 0xFFB6B6FF, 0xDA6DFFFF, 0xFF0000FF, 0x9191FFFF, 0x009191FF, 0x244800FF, 0x484848FF, 0xFF0091FF, 0xFFFFFFFF, 0x6D6D6DFF, 0xFFB600FF, 0xB6006DFF, 0x91006DFF, 0xDADA00FF, 0x6D4800FF, 0xFFFFFFFF, 0x6DB6FFFF, 0xDAB66DFF, 0x6D2400FF, 0x6DDA00FF, 0x91DAFFFF, 0xDAB6FFFF, 0xFFDA91FF, 0x0048FFFF, 0xFFDA00FF, 0x48FFDAFF, 0x000000FF, 0x480000FF, 0xDADADAFF, 0x919191FF, 0xFF00FFFF, 0x002491FF, 0x00006DFF, 0xB6DAFFFF, 0xFFB6FFFF, 0x00FF00FF, 0x00FFFFFF, 0x004848FF, 0x00B66DFF, 0xB600FFFF, 0x000000FF, 0x914800FF, 0xFF91FFFF, 0xB62400FF, 0x9100FFFF, 0x0000DAFF, 0xFF9100FF, 0x000000FF, 0x000000FF, 0x249100FF, 0xB6B6B6FF, 0x006D24FF, 0xB6FF48FF, 0x6D48DAFF, 0xFFFF00FF, 0xDA6D00FF, 0x004800FF, 0x006DDAFF, 0x009100FF, 0x242424FF, 0xFFFF6DFF, 0xFF6DFFFF, 0x916D00FF, 0x91FF6DFF },
/* 2C04_0002 */{ 0x000000FF, 0xFFB600FF, 0x916D00FF, 0xB6FF48FF, 0x91FF6DFF, 0xFF6DFFFF, 0x009191FF, 0xB6DAFFFF, 0xFF0000FF, 0x9100FFFF, 0xFFFF6DFF, 0xFF91FFFF, 0xFFFFFFFF, 0xDA6DFFFF, 0x91DAFFFF, 0x009100FF, 0x004800FF, 0x6DB6FFFF, 0xB62400FF, 0xDADADAFF, 0x00B66DFF, 0x6DDA00FF, 0x480000FF, 0x9191FFFF, 0x484848FF, 0xFF00FFFF, 0x00006DFF, 0x48FFDAFF, 0xDAB6FFFF, 0x6D4800FF, 0x000000FF, 0x6D48DAFF, 0x91006DFF, 0xFFDA91FF, 0xFF9100FF, 0xFFB6FFFF, 0x006DDAFF, 0x6D2400FF, 0xB6B6B6FF, 0x0000DAFF, 0xB600FFFF, 0xFFDA00FF, 0x6D6D6DFF, 0x244800FF, 0x0048FFFF, 0x000000FF, 0xDADA00FF, 0xFFFFFFFF, 0xDAB66DFF, 0x242424FF, 0x00FF00FF, 0xDA6D00FF, 0x004848FF, 0x002491FF, 0xFF0091FF, 0x249100FF, 0x000000FF, 0x00FFFFFF, 0x914800FF, 0xFFFF00FF, 0xFFB6B6FF, 0xB6006DFF, 0x006D24FF, 0x919191FF },
/* 2C04_0003 */{ 0xB600FFFF, 0xFF6DFFFF, 0x91FF6DFF, 0xB6B6B6FF, 0x009100FF, 0xFFFFFFFF, 0xB6DAFFFF, 0x244800FF, 0x002491FF, 0x000000FF, 0xFFDA91FF, 0x6D4800FF, 0xFF0091FF, 0xDADADAFF, 0xDAB66DFF, 0x91DAFFFF, 0x9191FFFF, 0x009191FF, 0xB6006DFF, 0x0048FFFF, 0x249100FF, 0x916D00FF, 0xDA6D00FF, 0x00B66DFF, 0x6D6D6DFF, 0x6D48DAFF, 0x000000FF, 0x0000DAFF, 0xFF0000FF, 0xB62400FF, 0xFF91FFFF, 0xFFB6B6FF, 0xDA6DFFFF, 0x004800FF, 0x00006DFF, 0xFFFF00FF, 0x242424FF, 0xFFB600FF, 0xFF9100FF, 0xFFFFFFFF, 0x6DDA00FF, 0x91006DFF, 0x6DB6FFFF, 0xFF00FFFF, 0x006DDAFF, 0x919191FF, 0x000000FF, 0x6D2400FF, 0x00FFFFFF, 0x480000FF, 0xB6FF48FF, 0xFFB6FFFF, 0x914800FF, 0x00FF00FF, 0xDADA00FF, 0x484848FF, 0x006D24FF, 0x000000FF, 0xDAB6FFFF, 0xFFFF6DFF, 0x9100FFFF, 0x48FFDAFF, 0xFFDA00FF, 0x004848FF },
/* 2C04_0004 */{ 0x916D00FF, 0x6D48DAFF, 0x009191FF, 0xDADA00FF, 0x000000FF, 0xFFB6B6FF, 0x002491FF, 0xDA6D00FF, 0xB6B6B6FF, 0x6D2400FF, 0x00FF00FF, 0x00006DFF, 0xFFDA91FF, 0xFFFF00FF, 0x009100FF, 0xB6FF48FF, 0xFF6DFFFF, 0x480000FF, 0x0048FFFF, 0xFF91FFFF, 0x000000FF, 0x484848FF, 0xB62400FF, 0xFF9100FF, 0xDAB66DFF, 0x00B66DFF, 0x9191FFFF, 0x249100FF, 0x91006DFF, 0x000000FF, 0x91FF6DFF, 0x6DB6FFFF, 0xB6006DFF, 0x006D24FF, 0x914800FF, 0x0000DAFF, 0x9100FFFF, 0xB600FFFF, 0x6D6D6DFF, 0xFF0091FF, 0x004848FF, 0xDADADAFF, 0x006DDAFF, 0x004800FF, 0x242424FF, 0xFFFF6DFF, 0x919191FF, 0xFF00FFFF, 0xFFB6FFFF, 0xFFFFFFFF, 0x6D4800FF, 0xFF0000FF, 0xFFDA00FF, 0x48FFDAFF, 0xFFFFFFFF, 0x91DAFFFF, 0x000000FF, 0xFFB600FF, 0xDA6DFFFF, 0xB6DAFFFF, 0x6DDA00FF, 0xDAB6FFFF, 0x00FFFFFF, 0x244800FF },
/* 2C05_01   */{ 0x6D6D6DFF, 0x002491FF, 0x0000DAFF, 0x6D48DAFF, 0x91006DFF, 0xB6006DFF, 0xB62400FF, 0x914800FF, 0x6D4800FF, 0x244800FF, 0x006D24FF, 0x009100FF, 0x004848FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xB6B6B6FF, 0x006DDAFF, 0x0048FFFF, 0x9100FFFF, 0xB600FFFF, 0xFF0091FF, 0xFF0000FF, 0xDA6D00FF, 0x916D00FF, 0x249100FF, 0x009100FF, 0x00B66DFF, 0x009191FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0x6DB6FFFF, 0x9191FFFF, 0xDA6DFFFF, 0xFF00FFFF, 0xFF6DFFFF, 0xFF9100FF, 0xFFB600FF, 0xDADA00FF, 0x6DDA00FF, 0x00FF00FF, 0x48FFDAFF, 0x00FFFFFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0xB6DAFFFF, 0xDAB6FFFF, 0xFFB6FFFF, 0xFF91FFFF, 0xFFB6B6FF, 0xFFDA91FF, 0xFFFF48FF, 0xFFFF6DFF, 0xB6FF48FF, 0x91FF6DFF, 0x48FFDAFF, 0x91DAFFFF, 0x000000FF, 0x000000FF, 0x000000FF },
/* 2C05_02   */{ 0x6D6D6DFF, 0x002491FF, 0x0000DAFF, 0x6D48DAFF, 0x91006DFF, 0xB6006DFF, 0xB62400FF, 0x914800FF, 0x6D4800FF, 0x244800FF, 0x006D24FF, 0x009100FF, 0x004848FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xB6B6B6FF, 0x006DDAFF, 0x0048FFFF, 0x9100FFFF, 0xB600FFFF, 0xFF0091FF, 0xFF0000FF, 0xDA6D00FF, 0x916D00FF, 0x249100FF, 0x009100FF, 0x00B66DFF, 0x009191FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0x6DB6FFFF, 0x9191FFFF, 0xDA6DFFFF, 0xFF00FFFF, 0xFF6DFFFF, 0xFF9100FF, 0xFFB600FF, 0xDADA00FF, 0x6DDA00FF, 0x00FF00FF, 0x48FFDAFF, 0x00FFFFFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0xB6DAFFFF, 0xDAB6FFFF, 0xFFB6FFFF, 0xFF91FFFF, 0xFFB6B6FF, 0xFFDA91FF, 0xFFFF48FF, 0xFFFF6DFF, 0xB6FF48FF, 0x91FF6DFF, 0x48FFDAFF, 0x91DAFFFF, 0x000000FF, 0x000000FF, 0x000000FF },
/* 2C05_03   */{ 0x6D6D6DFF, 0x002491FF, 0x0000DAFF, 0x6D48DAFF, 0x91006DFF, 0xB6006DFF, 0xB62400FF, 0x914800FF, 0x6D4800FF, 0x244800FF, 0x006D24FF, 0x009100FF, 0x004848FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xB6B6B6FF, 0x006DDAFF, 0x0048FFFF, 0x9100FFFF, 0xB600FFFF, 0xFF0091FF, 0xFF0000FF, 0xDA6D00FF, 0x916D00FF, 0x249100FF, 0x009100FF, 0x00B66DFF, 0x009191FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0x6DB6FFFF, 0x9191FFFF, 0xDA6DFFFF, 0xFF00FFFF, 0xFF6DFFFF, 0xFF9100FF, 0xFFB600FF, 0xDADA00FF, 0x6DDA00FF, 0x00FF00FF, 0x48FFDAFF, 0x00FFFFFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0xB6DAFFFF, 0xDAB6FFFF, 0xFFB6FFFF, 0xFF91FFFF, 0xFFB6B6FF, 0xFFDA91FF, 0xFFFF48FF, 0xFFFF6DFF, 0xB6FF48FF, 0x91FF6DFF, 0x48FFDAFF, 0x91DAFFFF, 0x000000FF, 0x000000FF, 0x000000FF },
/* 2C05_04   */{ 0x6D6D6DFF, 0x002491FF, 0x0000DAFF, 0x6D48DAFF, 0x91006DFF, 0xB6006DFF, 0xB62400FF, 0x914800FF, 0x6D4800FF, 0x244800FF, 0x006D24FF, 0x009100FF, 0x004848FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xB6B6B6FF, 0x006DDAFF, 0x0048FFFF, 0x9100FFFF, 0xB600FFFF, 0xFF0091FF, 0xFF0000FF, 0xDA6D00FF, 0x916D00FF, 0x249100FF, 0x009100FF, 0x00B66DFF, 0x009191FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0x6DB6FFFF, 0x9191FFFF, 0xDA6DFFFF, 0xFF00FFFF, 0xFF6DFFFF, 0xFF9100FF, 0xFFB600FF, 0xDADA00FF, 0x6DDA00FF, 0x00FF00FF, 0x48FFDAFF, 0x00FFFFFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0xB6DAFFFF, 0xDAB6FFFF, 0xFFB6FFFF, 0xFF91FFFF, 0xFFB6B6FF, 0xFFDA91FF, 0xFFFF48FF, 0xFFFF6DFF, 0xB6FF48FF, 0x91FF6DFF, 0x48FFDAFF, 0x91DAFFFF, 0x000000FF, 0x000000FF, 0x000000FF },
/* 2C05_05   */{ 0x6D6D6DFF, 0x002491FF, 0x0000DAFF, 0x6D48DAFF, 0x91006DFF, 0xB6006DFF, 0xB62400FF, 0x914800FF, 0x6D4800FF, 0x244800FF, 0x006D24FF, 0x009100FF, 0x004848FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xB6B6B6FF, 0x006DDAFF, 0x0048FFFF, 0x9100FFFF, 0xB600FFFF, 0xFF0091FF, 0xFF0000FF, 0xDA6D00FF, 0x916D00FF, 0x249100FF, 0x009100FF, 0x00B66DFF, 0x009191FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0x6DB6FFFF, 0x9191FFFF, 0xDA6DFFFF, 0xFF00FFFF, 0xFF6DFFFF, 0xFF9100FF, 0xFFB600FF, 0xDADA00FF, 0x6DDA00FF, 0x00FF00FF, 0x48FFDAFF, 0x00FFFFFF, 0x000000FF, 0x000000FF, 0x000000FF, 0xFFFFFFFF, 0xB6DAFFFF, 0xDAB6FFFF, 0xFFB6FFFF, 0xFF91FFFF, 0xFFB6B6FF, 0xFFDA91FF, 0xFFFF48FF, 0xFFFF6DFF, 0xB6FF48FF, 0x91FF6DFF, 0x48FFDAFF, 0x91DAFFFF, 0x000000FF, 0x000000FF, 0x000000FF }
};
// clang-format on