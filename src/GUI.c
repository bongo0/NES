#include "GUI.h"
#include "BUS.h"
#include "logger.h"

struct nk_color RGBAu32_to_nk_color(uint32_t in) {
  struct nk_color col;
  col.a = (in & 0xFF);
  col.b = ((in >> 8) & 0xFF);
  col.g = ((in >> 16) & 0xFF);
  col.r = (nk_byte)((in >> 24) & 0xFF);
  return col;
}

struct nk_colorf RGBAu32_to_nk_colorf(uint32_t in) {
  struct nk_colorf col;
  col.a = (float)(in & 0xFF) / 255;
  col.b = (float)((in >> 8) & 0xFF) / 255;
  col.g = (float)((in >> 16) & 0xFF) / 255;
  col.r = (float)(nk_byte)((in >> 24) & 0xFF) / 255;
  return col;
}

static inline char byte_to_printable_ascii(uint8_t b) {
  if (b >= 0x20 && b <= 0x7E)
    return b;
  else
    return '.';
}
#define bta(b) byte_to_printable_ascii(b)

void GUI_init(GUI_context *ctx, NES_BUS *nes) {

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    LOG_ERROR("SDL INIT ERROR: %s\n", SDL_GetError());
    GUI_quit(ctx);
    return;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  //SDL_GL_SetSwapInterval(0);
  ctx->window =
      SDL_CreateWindow("NES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WIN_WIDTH, WIN_HEIGHT,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                           SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
  if (ctx->window == NULL) {
    LOG_ERROR("SDL_CreateWindow ERROR: %s\n", SDL_GetError());
    GUI_quit(ctx);
    return;
  }
  ctx->gl_ctx = SDL_GL_CreateContext(ctx->window);
  if (ctx->gl_ctx == NULL) {
    LOG_ERROR("SDL_GL_CreateContext ERROR: %s\n", SDL_GetError());
    GUI_quit(ctx);
    return;
  }

  // OpenGL init
  glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
  glewExperimental = 1;
  if (glewInit() != GLEW_OK) {
    LOG_ERROR("Failed to setup GLEW\n");
    GUI_quit(ctx);
    return;
  }

  ctx->nk_ctx = nk_sdl_init(ctx->window);

  {
    nk_sdl_font_stash_begin(&ctx->atlas);
    // FONT
    /*int fs=19;
    struct nk_font_config fc = nk_font_config(fs);
     fc.oversample_h=1;
     fc.oversample_v=1;
     fc.pixel_snap=1;
    struct nk_font *font = nk_font_atlas_add_from_file(ctx->atlas,
     "~/NES.ttf", fs, &fc); */

    nk_sdl_font_stash_end();
    // nk_style_set_font(ctx->nk_ctx, &font->handle);
  }

  // nuklear style stuff
  ctx->nk_ctx->style.button.rounding = 0;

  ctx->bg = RGBAu32_to_nk_colorf(STYLE_DARK_GREY);

  // GUI state
  ctx->state.nes = nes;

  ctx->state.show_CPU_RAM = nk_false;
  ctx->state.show_NAM_RAM = nk_false;
  ctx->state.show_OAM_RAM = nk_false;
  ctx->state.show_OAS_RAM = nk_false;
  ctx->state.show_MAP_RAM = nk_false;
  ctx->state.write_tracelog = nk_false;
  ctx->state.show_disasm = nk_false;

  ctx->state.show_PPU_state = nk_false;
  ctx->state.show_CPU_state = nk_false;

  ctx->state.show_gains=nk_false;
  ctx->state.show_controller0=nk_false;
  ctx->state.show_controller1=nk_false;

}

int GUI_process_events(GUI_context *ctx) {
  SDL_Event event;
  int close = 0;
  nk_input_begin(ctx->nk_ctx);
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: close = 1; break;
    default:
      nk_sdl_handle_event(&event);
      close = 0;
      break;
    }
  }
  nk_input_end(ctx->nk_ctx);
  return close;
}

void GUI_render_begin(GUI_context *ctx) {
  SDL_GetWindowSize(ctx->window, &ctx->win_width, &ctx->win_height);
  //LOG_WARNING("width: %d height: %d \n",ctx->win_width,ctx->win_height);
}

void GUI_render_end(GUI_context *ctx) {
  SDL_GetWindowSize(ctx->window, &ctx->win_width, &ctx->win_height);
  glViewport(0, 0, ctx->win_width, ctx->win_height);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(ctx->bg.r, ctx->bg.g, ctx->bg.b, ctx->bg.a);
  // SDL_GetWindowSize(ctx->window, &ctx->win_width, &ctx->win_height);
  /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
   * with blending, scissor, face culling, depth test and viewport and
   * defaults everything back into a default state.
   * Make sure to either a.) save and restore or b.) reset your own state
   * after rendering the UI. */
  nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
  SDL_GL_SwapWindow(ctx->window);
}

void GUI_quit(GUI_context *ctx) {
  nk_sdl_shutdown();
  SDL_GL_DeleteContext(ctx->gl_ctx);
  // SDL_DestroyRenderer(ctx->renderer);
  SDL_DestroyWindow(ctx->window);
  SDL_Quit();
}

// ################################
//         GUI Elements
//#################################

void GUI_menu_bar(GUI_context *gui_ctx) {
  struct nk_context *ctx = gui_ctx->nk_ctx;
  if (nk_begin(ctx, "Menu", nk_rect(0, 0, gui_ctx->win_width, 30),
               NK_WINDOW_NO_SCROLLBAR)) {
    nk_menubar_begin(ctx);
    nk_layout_row_begin(ctx, NK_STATIC, 25, 6);
    nk_layout_row_push(ctx, 45);

    if (nk_menu_begin_label(ctx, "FILE", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      if (nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT)) {
      }
      if (nk_menu_item_label(ctx, "Save state", NK_TEXT_LEFT)) {
      }
      if (nk_menu_item_label(ctx, "Load state", NK_TEXT_LEFT)) {
      }
      nk_menu_end(ctx);
    }

    if (nk_menu_begin_label(ctx, "OPTIONS", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      nk_checkbox_label(ctx, "Controller 0", &gui_ctx->state.show_controller0);
      nk_checkbox_label(ctx, "Controller 1", &gui_ctx->state.show_controller1);

      nk_menu_end(ctx);
    }

    if (nk_menu_begin_label(ctx, "MEMORY", NK_TEXT_LEFT, nk_vec2(210, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      nk_checkbox_label(ctx, "CPU RAM", &gui_ctx->state.show_CPU_RAM);
      nk_checkbox_label(ctx, "Name table RAM", &gui_ctx->state.show_NAM_RAM);
      nk_checkbox_label(ctx, "Object Attribute Memory",
                        &gui_ctx->state.show_OAM_RAM);
      nk_checkbox_label(ctx, "Object Attribute Scanline",
                        &gui_ctx->state.show_OAS_RAM);
      nk_checkbox_label(ctx, "Mapper", &gui_ctx->state.show_MAP_RAM);

      nk_menu_end(ctx);
    }



    if (nk_menu_begin_label(ctx, "CPU", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      nk_checkbox_label(ctx, "CPU state", &gui_ctx->state.show_CPU_state);
      nk_checkbox_label(ctx, "Disassembly", &gui_ctx->state.show_disasm);
      nk_checkbox_label(ctx, "Trace log", &gui_ctx->state.write_tracelog);

      nk_menu_end(ctx);
    }

    if (nk_menu_begin_label(ctx, "PPU", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      nk_checkbox_label(ctx, "Pattern table",
                        &gui_ctx->state.show_Pattern_table);
      nk_checkbox_label(ctx, "PPU state", &gui_ctx->state.show_PPU_state);

      nk_menu_end(ctx);
    }

  if (nk_menu_begin_label(ctx, "APU", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      nk_checkbox_label(ctx, "Volume", &gui_ctx->state.show_gains);
      nk_menu_end(ctx);
    }

    nk_layout_row_end(ctx);
    nk_menubar_end(ctx);
  }
  nk_end(ctx);
}

void GUI_cpu_state(GUI_context *gui_ctx, NES_BUS *nes) {
  struct nk_context *ctx = gui_ctx->nk_ctx;
  CPU_state state = nes->cpu.state;
  if (nk_begin(ctx, "cpu", nk_rect(0, 30, 300, 150),
               NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE |
                   NK_WINDOW_SCALABLE)) {

    nk_layout_row_static(ctx, 8, 17, 8);
    char str[64];

    //    NVss DIZC
    //    |||| ||||
    //    |||| |||+- Carry
    //    |||| ||+-- Zero
    //    |||| |+--- Interrupt Disable
    //    |||| +---- Decimal
    //    ||++------ No CPU effect, see: the B flag
    //    |+-------- Overflow
    //    +--------- Negative
    // nk_layout_row_static(ctx, 0, 8, 8);
    struct nk_rect bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "N", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Negative");

    bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "O", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Overflow");

    bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "U", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Unused");

    bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "B", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Branch");

    bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "D", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Decimal");

    bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "I", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Interupt disable");

    bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "Z", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Zero");

    bounds = nk_widget_bounds(ctx);
    nk_label(ctx, "C", NK_TEXT_LEFT);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
      nk_tooltip(ctx, "  Carry");

    nk_layout_row_static(ctx, 8, 230, 1);
    snprintf(str, 64, "%c  %c  %c  %c  %c  %c  %c  %c  = 0x%02X",
             BYTE_TO_BINARY(state.P), state.P);
    nk_label(ctx, str, NK_TEXT_LEFT);

    nk_layout_row_static(ctx, 20, 230, 1);
    char str2[64];
    snprintf(str2, 64, "PC: %04X  SP: %02X  " /*"cycle: %lu"*/, state.PC,
             state.SP);
    nk_label(ctx, str2, NK_TEXT_LEFT);

    nk_layout_row_static(ctx, 8, 230, 1);
    char str3[64];
    snprintf(str3, 64, "A: %02X X: %02X Y: %02X", state.A, state.X, state.Y);
    nk_label(ctx, str3, NK_TEXT_LEFT);

    nk_layout_row_static(ctx, 8, 230, 1);
    char str4[64];
    snprintf(str4, 64, "Cycles: %lu", state.cycles_accumulated);
    nk_label(ctx, str4, NK_TEXT_LEFT);
  }
  nk_end(ctx);
}

void GUI_memory(GUI_context *gui_ctx) {
  static const int x = 50;
  static const int y = 400;
  static const int w = 680;
  static const int h = 380 + 16;
  pair_size_t range = {0, 0};
  size_t poi = 0;
#define HEX_00_0F_STR                                                          \
  "    00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F"
  if (gui_ctx->state.show_CPU_RAM) {
    if (nk_begin(gui_ctx->nk_ctx, "CPU" HEX_00_0F_STR, nk_rect(x, y, w, h),
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE)) {
      GUI_cpu_ram_view(gui_ctx, gui_ctx->state.nes);
    }
    nk_end(gui_ctx->nk_ctx);
  }

  if (gui_ctx->state.show_NAM_RAM) {
    if (nk_begin(gui_ctx->nk_ctx, "NAM" HEX_00_0F_STR, nk_rect(x, y, w, h),
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE)) {
      GUI_memory_view(gui_ctx, gui_ctx->state.nes->ppu.name_table, 2048, range,
                      poi);
    }
    nk_end(gui_ctx->nk_ctx);
  }

  if (gui_ctx->state.show_OAM_RAM) {
    if (nk_begin(gui_ctx->nk_ctx, "OAM" HEX_00_0F_STR, nk_rect(x, y, w, h),
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE)) {
      GUI_memory_view(gui_ctx, gui_ctx->state.nes->ppu.OAM, 64 * 4, range, poi);
    }
    nk_end(gui_ctx->nk_ctx);
  }

  if (gui_ctx->state.show_OAS_RAM) {
    if (nk_begin(gui_ctx->nk_ctx, "OAS" HEX_00_0F_STR, nk_rect(x, y, w, h),
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE)) {
      GUI_memory_view(gui_ctx, gui_ctx->state.nes->ppu.sprite_scan_line_OA,
                      8 * 4, range, poi);
    }
    nk_end(gui_ctx->nk_ctx);
  }

  if (gui_ctx->state.show_MAP_RAM) {
    if (gui_ctx->state.nes->rom->mapper_id == 1) {
      if (nk_begin(gui_ctx->nk_ctx, "001" HEX_00_0F_STR, nk_rect(x, y, w, h),
                   NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                       NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE)) {
        GUI_memory_view(
            gui_ctx,
            ((Mapper001 *)(gui_ctx->state.nes->rom->mapper.state))->RAM,
            32 * 1024, range, poi);
      }
      nk_end(gui_ctx->nk_ctx);
    } else {
    }
  }
}

void GUI_asm_txt(GUI_context *gui_ctx, Disassembly6502 *dasm, NES_BUS *nes) {
  struct nk_context *ctx = gui_ctx->nk_ctx;
  // len 56
  // 0000   ISB  $FFFF,X  ;ABS_XW     (0xFF) (unofficial)ISB
  const int cw = 8; // ctx->style.font->width+1;
  const int h = ctx->style.font->height + 3;
  if (nk_begin(ctx, "asm", nk_rect(0, gui_ctx->win_height / 2 - 60, 600, 470),
               NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {

    CPU_state state = nes->cpu.state;
    uint32_t mapped_adr;
    uint8_t dat;
    /*uint8_t map_flag =*/nes->rom->mapper.cpu_read(
        nes->rom->mapper.state, state.PC, &mapped_adr, &dat);

    static const int len = 12 + 56;
    char line[len];
    uint32_t l_adrs[10];
    // get constant number of lines before PC
    for (int lines = 0, i = -1; lines < 10; --i) {
      uint32_t adr = (mapped_adr + i);
      if (adr > dasm->size) {
        adr = dasm->size - 1;
      }
      l_adrs[lines] = adr;
      lines++;
    }
    // at PC
    nk_layout_row_static(ctx, h, cw * len + 20, 1);
    for (int i = 9; i >= 0; --i) {
      snprintf(line, len, "%04X<-%04X  %s", l_adrs[i], state.PC + (i - 10),
               dasm->lines[l_adrs[i]]);
      nk_label(ctx, line, NK_TEXT_LEFT);
    }
    // get constant number of lines after PC
    snprintf(line, len, "%04X<-%04X  %s", mapped_adr, state.PC,
             dasm->lines[mapped_adr]);
    nk_label_colored(ctx, line, NK_TEXT_LEFT,
                     RGBAu32_to_nk_color(STYLE_YELLOW));
    for (int lines = 0, i = 1; lines < 10; ++i) {
      uint32_t adr = (mapped_adr + i);
      if (adr > dasm->size)
        adr = 0;
      snprintf(line, len, "%04X<-%04X  %s", adr, state.PC + i,
               dasm->lines[adr]);
      nk_label(ctx, line, NK_TEXT_LEFT);
      lines++;
    }
  }
  nk_end(ctx);
}

nk_bool GUI_color_txt_button(GUI_context *ctx, uint32_t rgba, const char *txt,
                             nk_flags txt_align) {
  static struct nk_style_button color_txt_btn;
  color_txt_btn.rounding = 0;
  color_txt_btn.normal.data.color = RGBAu32_to_nk_color(rgba);
  color_txt_btn.hover.data.color = RGBAu32_to_nk_color(rgba);
  color_txt_btn.active.data.color = RGBAu32_to_nk_color(rgba);
  // color_txt_btn.text_background = RGBAu32_to_nk_color(0x000000aa);
  color_txt_btn.text_hover = RGBAu32_to_nk_color(0xffffffff - rgba + 0xff);
  color_txt_btn.text_normal = RGBAu32_to_nk_color(0xffffffff - rgba + 0xff);
  color_txt_btn.text_active = RGBAu32_to_nk_color(0xffffffff - rgba + 0xff);

  color_txt_btn.text_alignment = txt_align;
  return nk_button_text_styled(ctx->nk_ctx, &color_txt_btn, txt, 2);
}

// clang-format off
#include "PPU.h"
uint16_t GUI_palette_table(GUI_context *ctx){
      uint16_t ret=0xff00;
      const int blen=32;
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x0],"00",NK_TEXT_ALIGN_LEFT) ){ret=0x00;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x1],"01",NK_TEXT_ALIGN_LEFT) ){ret=0x01;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x2],"02",NK_TEXT_ALIGN_LEFT) ){ret=0x02;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x3],"03",NK_TEXT_ALIGN_LEFT) ){ret=0x03;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x4],"04",NK_TEXT_ALIGN_LEFT) ){ret=0x04;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x5],"05",NK_TEXT_ALIGN_LEFT) ){ret=0x05;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x6],"06",NK_TEXT_ALIGN_LEFT) ){ret=0x06;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x7],"07",NK_TEXT_ALIGN_LEFT) ){ret=0x07;}
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x10],"10",NK_TEXT_ALIGN_LEFT) ){ret=0x10;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x11],"11",NK_TEXT_ALIGN_LEFT) ){ret=0x11;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x12],"12",NK_TEXT_ALIGN_LEFT) ){ret=0x12;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x13],"13",NK_TEXT_ALIGN_LEFT) ){ret=0x13;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x14],"14",NK_TEXT_ALIGN_LEFT) ){ret=0x14;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x15],"15",NK_TEXT_ALIGN_LEFT) ){ret=0x15;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x16],"16",NK_TEXT_ALIGN_LEFT) ){ret=0x16;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x17],"17",NK_TEXT_ALIGN_LEFT) ){ret=0x17;}
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x20],"20",NK_TEXT_ALIGN_LEFT)){ret=0x20;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x21],"21",NK_TEXT_ALIGN_LEFT)){ret=0x21;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x22],"22",NK_TEXT_ALIGN_LEFT)){ret=0x22;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x23],"23",NK_TEXT_ALIGN_LEFT)){ret=0x23;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x24],"24",NK_TEXT_ALIGN_LEFT)){ret=0x24;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x25],"25",NK_TEXT_ALIGN_LEFT)){ret=0x25;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x26],"26",NK_TEXT_ALIGN_LEFT)){ret=0x26;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x27],"27",NK_TEXT_ALIGN_LEFT)){ret=0x27;}
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x30],"30",NK_TEXT_ALIGN_LEFT)){ret=0x30;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x31],"31",NK_TEXT_ALIGN_LEFT)){ret=0x31;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x32],"32",NK_TEXT_ALIGN_LEFT)){ret=0x32;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x33],"33",NK_TEXT_ALIGN_LEFT)){ret=0x33;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x34],"34",NK_TEXT_ALIGN_LEFT)){ret=0x34;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x35],"35",NK_TEXT_ALIGN_LEFT)){ret=0x35;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x36],"36",NK_TEXT_ALIGN_LEFT)){ret=0x36;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x37],"37",NK_TEXT_ALIGN_LEFT)){ret=0x37;}
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x8],"08",NK_TEXT_ALIGN_LEFT)){ret=0x08;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x9],"09",NK_TEXT_ALIGN_LEFT)){ret=0x09;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0xa],"0A",NK_TEXT_ALIGN_LEFT)){ret=0x0A;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0xb],"0B",NK_TEXT_ALIGN_LEFT)){ret=0x0B;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0xc],"0C",NK_TEXT_ALIGN_LEFT)){ret=0x0C;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0xd],"0D",NK_TEXT_ALIGN_LEFT)){ret=0x0D;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0xe],"0E",NK_TEXT_ALIGN_LEFT)){ret=0x0E;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0xf],"0F",NK_TEXT_ALIGN_LEFT)){ret=0x0F;}
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x18],"18",NK_TEXT_ALIGN_LEFT)){ret=0x18;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x19],"19",NK_TEXT_ALIGN_LEFT)){ret=0x19;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x1a],"1a",NK_TEXT_ALIGN_LEFT)){ret=0x1a;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x1b],"1b",NK_TEXT_ALIGN_LEFT)){ret=0x1b;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x1c],"1c",NK_TEXT_ALIGN_LEFT)){ret=0x1c;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x1d],"1d",NK_TEXT_ALIGN_LEFT)){ret=0x1d;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x1e],"1e",NK_TEXT_ALIGN_LEFT)){ret=0x1e;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x1f],"1f",NK_TEXT_ALIGN_LEFT)){ret=0x1f;}
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x28],"28",NK_TEXT_ALIGN_LEFT)){ret=0x28;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x29],"29",NK_TEXT_ALIGN_LEFT)){ret=0x29;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x2a],"2a",NK_TEXT_ALIGN_LEFT)){ret=0x2a;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x2b],"2b",NK_TEXT_ALIGN_LEFT)){ret=0x2b;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x2c],"2c",NK_TEXT_ALIGN_LEFT)){ret=0x2c;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x2d],"2d",NK_TEXT_ALIGN_LEFT)){ret=0x2d;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x2e],"2e",NK_TEXT_ALIGN_LEFT)){ret=0x2e;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x2f],"2f",NK_TEXT_ALIGN_LEFT)){ret=0x2f;}
      nk_layout_row_static(ctx->nk_ctx,blen,blen,8);
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x38],"38",NK_TEXT_ALIGN_LEFT)){ret=0x38;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x39],"39",NK_TEXT_ALIGN_LEFT)){ret=0x39;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x3a],"3a",NK_TEXT_ALIGN_LEFT)){ret=0x3a;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x3b],"3b",NK_TEXT_ALIGN_LEFT)){ret=0x3b;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x3c],"3c",NK_TEXT_ALIGN_LEFT)){ret=0x3c;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x3d],"3d",NK_TEXT_ALIGN_LEFT)){ret=0x3d;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x3e],"3e",NK_TEXT_ALIGN_LEFT)){ret=0x3e;}
      if(GUI_color_txt_button(ctx, PPU_PALETTE_RGBA[0][0x3f],"3f",NK_TEXT_ALIGN_LEFT)){ret=0x3f;}
      return ret;
}

uint16_t GUI_memory_view(GUI_context *gctx, uint8_t *memory, size_t nbytes, pair_size_t interesting_range, size_t interesting_pos) {
  struct nk_context *ctx = gctx->nk_ctx;
  struct nk_rect tmp;
  static char str[3];
  static int len_s;
  static uint8_t popup_edit = 0;
  static size_t edit = 0;
  static struct nk_rect b;

  char *mem_hex = malloc(3 * nbytes);
  for (size_t i = 0; i < nbytes; ++i) {
    snprintf(&mem_hex[i * 2], 3, "%02X", memory[i]);
  }

  //(int)ctx->style.font->width; 
  int cw = 7;//character width of default font
  int ch = 13;//character height of default font

  const int w=cw*3;// one byte 0x00 cell size
  
  const uint32_t def_color = 0x353535ff;
  const uint32_t range_color = 0x404048ff;
  const uint32_t pos_color = 0x694940ff;
  size_t row = 0;
  struct nk_rect lb;
  lb.h=ch+5;
  char str_row[20];
  for (size_t i = 0; i < nbytes; ++i) {
    if (i % 16 == 0) {
      if(i!=0){
        lb.x+=w*2;lb.w=cw*20;nk_layout_space_push(ctx,lb);
        nk_label(ctx, str_row, NK_TEXT_ALIGN_LEFT);
        nk_layout_space_end(ctx);
      }
      
      nk_layout_space_begin(ctx,NK_STATIC,16,16 +2);
      lb.x=0;lb.y=0;lb.w=4*cw;
      nk_layout_space_push(ctx,lb);
      lb.w=cw*4;lb.x=cw*2;
      
      char crow[5];
      snprintf(crow, 5, "%04lX", row);
      nk_label(ctx, crow, NK_TEXT_ALIGN_CENTERED);


        snprintf(str_row,20,"%c%c%c%c" "%c%c%c%c" "%c%c%c%c" "%c%c%c%c",
bta(memory[row   ]), bta(memory[row+ 1]), bta(memory[row+ 2]), bta(memory[row+ 3]),
bta(memory[row+ 4]), bta(memory[row+ 5]), bta(memory[row+ 6]), bta(memory[row+ 7]),
bta(memory[row+ 8]), bta(memory[row+ 9]), bta(memory[row+10]), bta(memory[row+11]),
bta(memory[row+12]), bta(memory[row+13]), bta(memory[row+14]), bta(memory[row+15])
        );
      row += 0x10;
    }

    lb.x+=cw*4;nk_layout_space_push(ctx,lb);
    tmp = nk_widget_bounds(ctx);
    
    uint32_t color = def_color;
    if ( (interesting_range.y !=0 && interesting_range.y>interesting_range.x) &&   
      (i >= interesting_range.x && i <= interesting_range.y))
      color = range_color;
    if (interesting_pos!=0 && i == interesting_pos)
      color = pos_color;
    if (GUI_color_txt_button(gctx, color, &mem_hex[i * 2],
                             NK_TEXT_ALIGN_CENTERED)) {
      edit = i;
      popup_edit = 1;
      b = tmp;
    }

  }
  lb.x+=w*2;lb.w=cw*20;nk_layout_space_push(ctx,lb);
  nk_label(ctx, str_row, NK_TEXT_ALIGN_LEFT);
  nk_layout_space_end(ctx);
  
  if (popup_edit) {
    nk_layout_space_begin(ctx, NK_STATIC, 32, 1);
    struct nk_rect b2 = nk_layout_space_rect_to_local(ctx, b);
    b2.x -= 3;
    b2.y -= 3;
    b2.h += 3;
    b2.w += 3;
    nk_layout_space_push(ctx, b2);
    nk_flags active = nk_edit_string(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER,
                                     str, &len_s, 3, nk_filter_hex);
    if (active & NK_EDIT_COMMITED) {
      memory[edit] = strtoul(str, NULL, 16);
      popup_edit = 0;
    }
    nk_layout_space_end(ctx);
  }
  return 0;
}
// clang-format on

uint16_t GUI_cpu_ram_view(GUI_context *gctx, NES_BUS *nes) {
  pair_size_t range;
  range.x = 0x100;
  range.y = 0x1ff;
  GUI_memory_view(gctx, nes->ram, CPU_RAM_SIZE, range,
                  0x100 + nes->cpu.state.SP);
  return 0;
}

void GUI_palette_view(GUI_context *gctx, NES_BUS *nes) {
  const int pal_h = 18;
  nk_layout_row_dynamic(gctx->nk_ctx, pal_h, 8);
  struct nk_rect bounds;
  bounds = nk_widget_bounds(gctx->nk_ctx);
  uint16_t pal_i;

#define PALETTE_RAM_BUTTON(palette, color_idx)                                 \
  bounds = nk_widget_bounds(gctx->nk_ctx);                                     \
  nk_button_color(gctx->nk_ctx,                                                \
                  RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(          \
                      &nes->ppu, palette, color_idx)));                        \
  if (nk_contextual_begin(gctx->nk_ctx, 0, nk_vec2(300, 300), bounds)) {       \
    pal_i = GUI_palette_table(gctx);                                           \
    if (pal_i < 0x40) {                                                        \
      nes->ppu.palette_ram[4 * palette + color_idx] = pal_i;                   \
    }                                                                          \
    nk_contextual_end(gctx->nk_ctx);                                           \
  }

#define PALETTE_RAM_BUTTONS(palette)                                           \
  if (nk_group_begin(gctx->nk_ctx, "Group",                                    \
                     NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {             \
    nk_layout_row_dynamic(gctx->nk_ctx, pal_h, 4);                             \
    PALETTE_RAM_BUTTON(palette, 0);                                            \
    PALETTE_RAM_BUTTON(palette, 1);                                            \
    PALETTE_RAM_BUTTON(palette, 2);                                            \
    PALETTE_RAM_BUTTON(palette, 3);                                            \
    nk_group_end(gctx->nk_ctx);                                                \
  }

  PALETTE_RAM_BUTTONS(0);
  PALETTE_RAM_BUTTONS(1);
  PALETTE_RAM_BUTTONS(2);
  PALETTE_RAM_BUTTONS(3);

  PALETTE_RAM_BUTTONS(4);
  PALETTE_RAM_BUTTONS(5);
  PALETTE_RAM_BUTTONS(6);
  PALETTE_RAM_BUTTONS(7);
}

struct nk_image GUI_image_rgba(GUI_context *gctx, uint32_t *img_data,
                               uint32_t w, uint32_t h) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8, img_data);
  struct nk_image img;
  img.handle.id = tex;
  img.h = 0;
  img.w = 0; // for auto size
  return img;
}

void GUI_image_refresh(GUI_context *ctx, struct nk_image *img,
                       uint32_t *img_data, uint32_t w, uint32_t h) {
  glBindTexture(GL_TEXTURE_2D, img->handle.id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
                  GL_UNSIGNED_INT_8_8_8_8, img_data);
}

void GUI_ppu_state(GUI_context *gctx, const PPU *ppu) {
  struct nk_context *ctx = gctx->nk_ctx;

  if (nk_begin(ctx, "ppu", nk_rect(gctx->win_width - 900, 30, 290, 150),
               NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
    char str[16];

    nk_layout_row_dynamic(ctx, 16, 2);
    nk_label(ctx, "Cycle:", NK_TEXT_ALIGN_LEFT);
    snprintf(str, 16, "%u ", ppu->cycle);
    nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);

    nk_layout_row_dynamic(ctx, 16, 2);
    nk_label(ctx, "Scanline:", NK_TEXT_ALIGN_LEFT);
    snprintf(str, 16, "%d ", ppu->scan_line);
    nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);

    nk_layout_row_dynamic(ctx, 16, 2);
    nk_label(ctx, "Frame:", NK_TEXT_ALIGN_LEFT);
    snprintf(str, 16, "%u ", ppu->frame_count);
    nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);

    nk_layout_row_dynamic(ctx, 16, 2);
    nk_label(ctx, "addr:", NK_TEXT_ALIGN_LEFT);
    snprintf(str, 16, "%04X ", ppu->state.vram_adr.reg);
    nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);

    nk_layout_row_dynamic(ctx, 16, 2);
    nk_label(ctx, "data buf:", NK_TEXT_ALIGN_LEFT);
    snprintf(str, 16, "%02X ", ppu->state.data_buf);
    nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);

    nk_layout_row_dynamic(ctx, 16, 2);
    nk_label(ctx, "X Scroll:", NK_TEXT_ALIGN_LEFT);
    snprintf(str, 16, "%02X ", ppu->state.x_scroll);
    nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);

    // status
    // control
    // mask
    nk_layout_row_dynamic(ctx, 16 * 6, 1);
    if (nk_group_begin(gctx->nk_ctx, "Group1",
                       NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(ctx, 16, 1);
      nk_label(ctx, "STATUS:", NK_TEXT_ALIGN_LEFT);
      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Vertical blank:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.status.VERTICAL_BLANK ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Sprite overflow:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.status.SPRITE_OVERFLOW ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Sprite zero hit:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.status.SPRITE_ZERO_HIT ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);
      nk_group_end(ctx);
    }

    nk_layout_row_dynamic(ctx, 16 * 12, 1);
    if (nk_group_begin(gctx->nk_ctx, "Group1",
                       NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(ctx, 16, 1);
      nk_label(ctx, "CONTROLL:", NK_TEXT_ALIGN_LEFT);
      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Increment mode:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.INCREMENT_MODE ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "NMI enable:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.ENABLE_NMI ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Nametable X:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.NAMETABLE_X ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Nametable X:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.NAMETABLE_Y ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Pattern background:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.PATTERN_BACKGROUND ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Pattern sprite:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.PATTERN_SPRITE ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Sprite size:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.SPRITE_SIZE ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Slave mode:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.control.SLAVE_MODE ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_group_end(ctx);
    }

    nk_layout_row_dynamic(ctx, 16 * 12, 1);
    if (nk_group_begin(gctx->nk_ctx, "Group1",
                       NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(ctx, 16, 1);
      nk_label(ctx, "MASK:", NK_TEXT_ALIGN_LEFT);
      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Render background:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.RENDER_BACKGROUND ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Render background L:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.RENDER_BACKGROUND_LEFT ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Render sprites:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.RENDER_SPRITES ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Render sprites L:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.RENDER_SPRITES_LEFT ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Enhance red:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.ENHANCE_RED ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Enhance green:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.ENHANCE_GREEN ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Enhance blue:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.ENHANCE_BLUE ? "1" : "0",
               NK_TEXT_ALIGN_RIGHT);

      nk_layout_row_dynamic(ctx, 16, 2);
      nk_label(ctx, "Greyscale:", NK_TEXT_ALIGN_LEFT);
      nk_label(ctx, ppu->state.mask.GRAYSCALE ? "1" : "0", NK_TEXT_ALIGN_RIGHT);
      nk_group_end(ctx);
    }
  }
  nk_end(ctx);
}

void GUI_apu_dB(GUI_context *gctx, APU *apu) {
  struct nk_context *ctx = gctx->nk_ctx;
  static float gains[6];

  char str[32];
  if (nk_begin(ctx, "gain", nk_rect(0, 180, 350, 245),
               NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE | NK_WINDOW_BORDER |
                   NK_WINDOW_MOVABLE)) {

    nk_layout_row_static(ctx, 30, 200, 2);
    nk_slider_float(ctx, -60.0f, &gains[5], 0.0f, 0.1);
    snprintf(str, 32, "%5.1f dB master", gains[5]);
    nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

    nk_layout_row_static(ctx, 30, 200, 2);
    nk_slider_float(ctx, -60.0f, &gains[0], 0.0f, 0.1);
    snprintf(str, 32, "%5.1f dB square 1", gains[0]);
    nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

    nk_layout_row_static(ctx, 30, 200, 2);
    nk_slider_float(ctx, -60.0f, &gains[1], 0.0f, 0.1);
    snprintf(str, 32, "%5.1f dB square 2", gains[1]);
    nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

    nk_layout_row_static(ctx, 30, 200, 2);
    nk_slider_float(ctx, -60.0f, &gains[2], 0.0f, 0.1);
    snprintf(str, 32, "%5.1f dB triangle", gains[2]);
    nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

    nk_layout_row_static(ctx, 30, 200, 2);
    nk_slider_float(ctx, -60.0f, &gains[3], 0.0f, 0.1);
    snprintf(str, 32, "%5.1f dB noise", gains[3]);
    nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

    nk_layout_row_static(ctx, 30, 200, 2);
    nk_slider_float(ctx, -60.0f, &gains[4], 0.0f, 0.1);
    snprintf(str, 32, "%5.1f dB dmc", gains[4]);
    nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

    APU_set_dB_master(apu, gains[5]);
    APU_set_dB_sqr1(apu, gains[0]);
    APU_set_dB_sqr2(apu, gains[1]);
    APU_set_dB_tr(apu, gains[2]);
    APU_set_dB_noise(apu, gains[3]);
    APU_set_dB_dmc(apu, gains[4]);

    nk_end(ctx);
  }
}

nk_bool GUI_color_round_button(struct nk_context *ctx, uint32_t rgba,uint32_t rgba_active,
                               float rounding) {
  static struct nk_style_button color_round_btn;
  color_round_btn.rounding = rounding;
  color_round_btn.normal.data.color = RGBAu32_to_nk_color(rgba);
  color_round_btn.hover.data.color = RGBAu32_to_nk_color(rgba);
  color_round_btn.active.data.color = RGBAu32_to_nk_color(rgba_active);

  return nk_button_text_styled(ctx, &color_round_btn, "", 0);
}

void GUI_NES_controller(GUI_context *gctx, uint8_t *c_state, uint8_t i) {
  struct nk_context *ctx = gctx->nk_ctx;

 nk_bool b_right = (*c_state)&(1<<0);//Right
 nk_bool b_left  = (*c_state)&(1<<1);//Left
 nk_bool b_down  = (*c_state)&(1<<2);//Down
 nk_bool b_up    = (*c_state)&(1<<3);//Up
 nk_bool b_start = (*c_state)&(1<<4);//Start
 nk_bool b_select= (*c_state)&(1<<5);//Select
 nk_bool b_B     = (*c_state)&(1<<6);//B
 nk_bool b_A     = (*c_state)&(1<<7);//A

  if (nk_begin(ctx, i?"ctrl 1":"ctrl 0", nk_rect(0, 180, 300, 128),
               NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE | NK_WINDOW_BORDER |
                   NK_WINDOW_MOVABLE)) {
struct nk_rect lb;
const uint32_t b = 0x2f2f2f00;// brighter when pressed
// clang-format off
nk_layout_space_begin(ctx,NK_STATIC,16,16);
    lb.h=90;lb.w=290;lb.x=0;lb.y=0;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_DARK_GREY,STYLE_DARK_GREY, 5);
// meh hack grey quad
    lb.h=25;lb.w=100;lb.x=90;lb.y=40;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_LIGHT_GREY,STYLE_LIGHT_GREY, 5);
    

    lb.h=30;lb.w=30;lb.x=240;lb.y=32;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_RED+(b_A?b:0) ,STYLE_RED + b, 14.5);
    lb.h=30;lb.w=30;lb.x=240;lb.y=32+30;
    nk_layout_space_push(ctx,lb);
    nk_label_colored(ctx,"A",NK_TEXT_ALIGN_RIGHT,RGBAu32_to_nk_color(STYLE_RED));

    lb.h=30;lb.w=30;lb.x=200;lb.y=32;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_RED+(b_B?b:0),STYLE_RED + b, 14.5);
    lb.h=30;lb.w=30;lb.x=200;lb.y=32+30;
    nk_layout_space_push(ctx,lb);
    nk_label_colored(ctx,"B",NK_TEXT_ALIGN_RIGHT,RGBAu32_to_nk_color(STYLE_RED));

    lb.h=10;lb.w=30;lb.x=150;lb.y=32+15;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_DARK_GREY+(b_start?b:0),STYLE_GREY, 3);
    lb.h=30;lb.w=30;lb.x=150-3;lb.y=20;
    nk_layout_space_push(ctx,lb);
    nk_label_colored(ctx,"start",NK_TEXT_ALIGN_RIGHT,RGBAu32_to_nk_color(STYLE_RED));

    lb.h=10;lb.w=30;lb.x=100;lb.y=32+15;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_DARK_GREY+(b_select?b:0),STYLE_GREY, 3);
    lb.h=30;lb.w=30;lb.x=100-5;lb.y=20;
    nk_layout_space_push(ctx,lb);
    nk_label_colored(ctx,"select",NK_TEXT_ALIGN_RIGHT,RGBAu32_to_nk_color(STYLE_RED));

    // right
    lb.h=20;lb.w=20;lb.x=50;lb.y=32+5;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_GREY+(b_right?b:0),STYLE_GREY + b, 1);
    //center
    lb.h=20;lb.w=20;lb.x=30;lb.y=32+5;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_GREY,STYLE_GREY, 1);
    //left
    lb.h=20;lb.w=20;lb.x=10;lb.y=32+5;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_GREY+(b_left?b:0),STYLE_GREY+ b, 1);
    // up
    lb.h=20;lb.w=20;lb.x=30;lb.y=12+5;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_GREY+(b_up?b:0),STYLE_GREY+ b, 1);
    //down
    lb.h=20;lb.w=20;lb.x=30;lb.y=52+5;
    nk_layout_space_push(ctx,lb);
    GUI_color_round_button(ctx, STYLE_GREY+(b_down?b:0),STYLE_GREY+ b, 1);
    
nk_layout_space_end(ctx);
// clang-format on
    nk_end(ctx);
  }
}