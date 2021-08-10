#ifndef GUI_H
#define GUI_H
#include "../deps/Nuklear/conf_nuklear.h"
#define WIN_WIDTH 1200
#define WIN_HEIGHT 800

// Global state
typedef struct GUI_context {
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
void asm_txt(GUI_context *gui_ctx, char *text, size_t text_len);
#endif // GUI_H