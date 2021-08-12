#ifndef GUI_H
#define GUI_H
#include "../deps/Nuklear/conf_nuklear.h"
#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

#include "CPU_6502.h"

#define BYTE_TO_BINARY_FORMAT "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                   \
  (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),                        \
      (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'),                    \
      (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'),                    \
      (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

// Global state
typedef struct {
  SDL_Window *window;
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
void Menu_bar(GUI_context *ctx);
void cpu_state(GUI_context *gui_ctx, const CPU_state state);
void asm_txt(GUI_context *gui_ctx, const char **text, uint16_t size, const CPU_state state);
#endif // GUI_H