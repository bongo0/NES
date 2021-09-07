#ifndef GUI_H
#define GUI_H
#include "../deps/Nuklear/conf_nuklear.h"
#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

#include "CPU_6502.h"
#include "PPU.h"
#include "APU.h"
#include "disassembler.h"

#define BYTE_TO_BINARY_FORMAT "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                   \
  (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),                        \
      (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'),                    \
      (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'),                    \
      (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

struct nk_color RGBAu32_to_nk_color(uint32_t in);

#define STYLE_DARK_GREY 0x1e1b1cFF  // - dark grey
#define STYLE_GREY 0x544f4cFF       // - grey
#define STYLE_LIGHT_GREY 0xc2bebbFF // - light grey
#define STYLE_RED 0xb23323FF        // - red
#define STYLE_YELLOW 0xd9b128FF     // - yellow
#define STYLE_BLUE 0x193549FF       // - blue

typedef struct {
  size_t x, y;
} pair_size_t;

typedef struct {
  // CPU
  nk_bool show_CPU_state;
  // RAM
  nk_bool show_CPU_RAM;
  nk_bool show_NAM_RAM;
  nk_bool show_OAM_RAM;
  nk_bool show_OAS_RAM;
  nk_bool show_MAP_RAM;
  // PPU
  nk_bool show_PPU_state;
  nk_bool show_Pattern_table;
  // Debug
  nk_bool write_tracelog;
  nk_bool show_disasm;

  NES_BUS *nes;
} GUI_state;

// Global state
typedef struct {
  SDL_Window *window;
  // SDL_Renderer *renderer;
  int win_width, win_height;
  SDL_GLContext gl_ctx;
  struct nk_context *nk_ctx;
  struct nk_font_atlas *atlas;
  struct nk_colorf bg;

  GUI_state state;
} GUI_context;

void GUI_init(GUI_context *ctx, NES_BUS *nes);
int GUI_process_events(GUI_context *ctx);
void GUI_render_begin(GUI_context *ctx);
void GUI_render_end(GUI_context *ctx);
void GUI_quit(GUI_context *ctx);

// GUI Elements
void GUI_menu_bar(GUI_context *ctx);
void GUI_cpu_state(GUI_context *gui_ctx, NES_BUS *nes);
void GUI_ppu_state(GUI_context *gctx, const PPU *ppu);
void GUI_asm_txt(GUI_context *gui_ctx, Disassembly6502 *dasm, NES_BUS *nes);
void GUI_memory(GUI_context *gui_ctx);

void GUI_draw_image(int x, int y, int width, int height, GLuint texture_id,
                    uint8_t blendFlag);

nk_bool GUI_color_txt_button(GUI_context *ctx, uint32_t rgba, const char *txt,
                             nk_flags txt_align);
uint16_t GUI_palette_table(GUI_context *ctx);

uint16_t GUI_memory_view(GUI_context *gctx, uint8_t *memory, size_t nbytes,
                         pair_size_t interesting_range, size_t interesting_pos);
uint16_t GUI_cpu_ram_view(GUI_context *gctx, NES_BUS *nes);

void GUI_palette_view(GUI_context *gctx, NES_BUS *nes);

struct nk_image GUI_image_rgba(GUI_context *gctx, uint32_t *img_data,
                               uint32_t w, uint32_t h);
void GUI_image_refresh(GUI_context *ctx, struct nk_image *img,
                       uint32_t *img_data, uint32_t w, uint32_t h);

void GUI_apu_dB(GUI_context *gctx, APU *apu);

#endif // GUI_H