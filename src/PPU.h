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
  PPU_loopy vram_adr;
  PPU_loopy t_vram_adr;
  uint8_t x_scroll; // i.e. fine_x
  uint8_t write_toggle;

  uint8_t adr_write_latch;
  uint8_t data_buf; // reading from ppu is delayed by one cycle, buffer for
                    // that data

  uint8_t nmi;
} PPU_state;

/* typedef struct {
  uint8_t y;         // Y position of sprite
  uint8_t id;        // ID of tile from pattern memory
  uint8_t attribute; // Flags define how sprite should be rendered
  uint8_t x;         // X position of sprite
} Sprite_OA; */

typedef struct {
  PPU_state state;

  int32_t scan_line;
  uint32_t cycle;
  uint8_t odd_frame;
  uint8_t frame_complete;
  uint32_t frame_count;
  uint64_t master_clock;

  uint8_t palette_ram[0x20];
  uint8_t sprite_ram[0x100];
  uint8_t name_table[2 * 0x400];
  uint8_t pattern_ram[2 * 0x1000];

  uint32_t pattern_table_img[2 * 128 * 128];
  uint8_t pattern_table[2 * 128 * 128];

  uint32_t screen[256 * 240];

  // background rendering stuff
  uint8_t next_bg_tile_id;
  uint8_t next_bg_tile_attrib;
  uint8_t next_bg_tile_lsb;
  uint8_t next_bg_tile_msb;
  uint16_t high_bg_shifter;
  uint16_t low_bg_shifter;
  uint16_t high_bg_shifter_attrib;
  uint16_t low_bg_shifter_attrib;
  // Sprite rendering stuff

  // Object Attribute Memory
  // Each attrib has 4 bytes, in order:
  // Y position of sprite
  // ID of tile from pattern memory
  // Flags define how sprite should be rendered
  // X position of sprite
#define OAM_y(i) (4 * i + 0)
#define OAM_id(i) (4 * i + 1)
#define OAM_attrib(i) (4 * i + 2)
#define OAM_x(i) (4 * i + 3)
  uint8_t OAM[64 * 4];

  uint8_t OAM_adr;
  uint8_t sprite_scan_line_OA[8*4];
  uint8_t sprite_count;
  uint8_t sprite_pattern_low[8];
  uint8_t sprite_pattern_high[8];
  uint8_t sprite_0_hit_possible;
  uint8_t sprite_0_is_rendering;

  NES_ROM *rom;
} PPU;

void PPU_init(PPU *ppu, NES_ROM *rom);
void PPU_tick(PPU *ppu);
void PPU_reset(PPU *ppu);
uint8_t PPU_cpu_read(PPU *ppu, uint16_t adr);
void PPU_cpu_write(PPU *ppu, uint16_t adr, uint8_t data);
uint8_t PPU_read(PPU *ppu, uint16_t adr);
void PPU_write(PPU *ppu, uint16_t adr, uint8_t data);
uint32_t PPU_get_color_from_palette_ram(PPU *ppu, uint16_t palette_idx,
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

uint32_t PPU_PALETTE_RGBA[11][64];

#endif // PPU_H