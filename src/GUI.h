#ifndef GUI_H
#define GUI_H
#include "../deps/Nuklear/conf_nuklear.h"
#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

#include "CPU_6502.h"
#include "PPU.h"

#define BYTE_TO_BINARY_FORMAT "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                   \
  (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),                        \
      (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'),                    \
      (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'),                    \
      (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

struct nk_color RGBAu32_to_nk_color(uint32_t in);

// Global state
typedef struct {
  SDL_Window *window;
  // SDL_Renderer *renderer;
  int win_width, win_height;
  SDL_GLContext gl_ctx;
  struct nk_context *nk_ctx;
  struct nk_font_atlas *atlas;
  struct nk_colorf bg;
} GUI_context;

void GUI_init(GUI_context *ctx);
int GUI_process_events(GUI_context *ctx);
void GUI_render_begin(GUI_context *ctx);
void GUI_render_end(GUI_context *ctx);
void GUI_quit(GUI_context *ctx);

// GUI Elements
void GUI_menu_bar(GUI_context *ctx);
void GUI_cpu_state(GUI_context *gui_ctx, NES_BUS *nes);
void GUI_ppu_state(GUI_context *gctx, const PPU *ppu);
void GUI_asm_txt(GUI_context *gui_ctx, char **text, uint16_t size,
                 const CPU_state state);

void GUI_draw_image(int x, int y, int width, int height, GLuint texture_id,
                    uint8_t blendFlag);

nk_bool GUI_color_txt_button(GUI_context *ctx, uint32_t rgba, const char *txt);
uint16_t GUI_palette_table(GUI_context *ctx);

uint16_t GUI_memory_view(GUI_context *gctx, uint8_t *memory, size_t nbytes);
void GUI_palette_view(GUI_context *gctx, NES_BUS *nes);

struct nk_image GUI_image_rgba(GUI_context *gctx, uint32_t *img_data,
                               uint32_t w, uint32_t h);
void GUI_image_refresh(GUI_context *ctx, struct nk_image *img,
                       uint32_t *img_data, uint32_t w, uint32_t h);
#endif // GUI_H