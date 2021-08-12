#ifndef PPU_H
#define PPU_H
#include "ROM.h"
#include <stdint.h>

#define PPU_STATUS_SPRITE_OVERFLOW (1 << 5)
#define PPU_STATUS_SPRITE_ZERO_HIT (1 << 6)
#define PPU_STATUS_VERTICAL_BLANK (1 << 7)

typedef union {
  struct {
    uint8_t UNUSED : 5;
    uint8_t SPRITE_OVERFLOW : 1;
    uint8_t SPRITE_ZERO_HIT : 1;
    uint8_t VERTICAL_BLANK : 1;
  };
  uint8_t reg;
} PPU_status;

typedef union {
  struct {
    uint8_t NAMETABLE_X : 1;
    uint8_t NAMETABLE_Y : 1;
    uint8_t INCREMENT_MODE : 1;
    uint8_t PATTERN_SPRITE : 1;
    uint8_t PATTERN_BACKGROUND : 1;
    uint8_t SPRITE_SIZE : 1;
    uint8_t SLAVE_MODE : 1; // UNUSED
    uint8_t ENABLE_NMI : 1;
  };
  uint8_t reg;
} PPU_control;

typedef union {
  struct {
    uint8_t GRAYSCALE : 1;
    uint8_t RENDER_BACKGROUND_LEFT : 1;
    uint8_t RENDER_SPRITES_LEFT : 1;
    uint8_t RENDER_BACKGROUND : 1;
    uint8_t RENDER_SPRITES : 1;
    uint8_t ENHANCE_RED : 1;
    uint8_t ENHANCE_GREEN : 1;
    uint8_t ENHANCE_BLUE : 1;
  };
  uint8_t reg;
} PPU_mask;

typedef union {
  struct {
    uint16_t coarse_x : 5;
    uint16_t coarse_y : 5;
    uint16_t nametable_x : 1;
    uint16_t nametable_y : 1;
    uint16_t fine_y : 3;
    uint16_t unused : 1;
  };
  uint16_t reg;
} PPU_loopy;

typedef struct {
  PPU_status status;
  PPU_control control;
  PPU_mask mask;
  PPU_loopy loopy;

  uint32_t sprite_ram_adr;
  PPU_loopy video_ram_adr;
  PPU_loopy tmp_video_ram_adr;
  uint8_t x_scroll;
  uint8_t write_toggle;
  uint16_t high_bit_shift;
  uint16_t low_bit_shift;
} PPU_state;

typedef struct {
  PPU_state state;

  int32_t scan_line;
  uint32_t cycle;
  uint32_t frame_count;
  uint64_t master_clock;

  uint8_t mem_read_buf;
  uint8_t palette_ram[0x20];
  uint8_t sprite_ram[0x100];
  uint8_t pattern_ram[2 * 0x1000];

  uint32_t pattern_table[2 * 128 * 128];

  NES_ROM *rom;
} PPU;

uint8_t PPU_read(PPU *ppu, uint16_t adr);
void PPU_write(PPU *ppu, uint16_t adr, uint8_t data);
uint32_t PPU_get_color_from_palette_ram(PPU *ppu, uint8_t palette_idx,
                                        uint8_t pixel_val);
void PPU_load_pattern_table(PPU *ppu, uint8_t table_n /*0 or 1*/);

typedef enum {
  PALETTE_2C02 = 0,
  PALETTE_2C03 = 1,
  PALETTE_2C04_0001 = 2,
  PALETTE_2C04_0002 = 3,
  PALETTE_2C04_0003 = 4,
  PALETTE_2C04_0004 = 5,
  PALETTE_2C05_01 = 6,
  PALETTE_2C05_02 = 7,
  PALETTE_2C05_03 = 8,
  PALETTE_2C05_04 = 9,
  PALETTE_2C05_05 = 10
} PPU_PALETTE;

uint32_t PPU_PALETTE_ARGB[11][64];

#endif // PPU_H