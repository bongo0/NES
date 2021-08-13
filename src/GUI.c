#include "GUI.h"
#include "logger.h"

void GUI_init(GUI_context *ctx) {

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
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

    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/

    nk_sdl_font_stash_end();
  
  /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &roboto->handle);*/}
    ctx->bg.r = 0.10f;
    ctx->bg.g = 0.18f;
    ctx->bg.b = 0.24f;
    ctx->bg.a = 1.0f;
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

void Menu_bar(GUI_context *gui_ctx) {
  struct nk_context *ctx = gui_ctx->nk_ctx;
  if (nk_begin(ctx, "Menu", nk_rect(0, 0, gui_ctx->win_width, 30),
               NK_WINDOW_NO_SCROLLBAR)) {
    nk_menubar_begin(ctx);
    nk_layout_row_begin(ctx, NK_STATIC, 25, 5);
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

    if (nk_menu_begin_label(ctx, "VIEW", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      if (nk_menu_item_label(ctx, "Debug", NK_TEXT_LEFT)) {
      }
      if (nk_menu_item_label(ctx, "stuff etc", NK_TEXT_LEFT)) {
      }
      nk_menu_end(ctx);
    }

    if (nk_menu_begin_label(ctx, "OPTIONS", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 25, 1);
      if (nk_menu_item_label(ctx, "Controls", NK_TEXT_LEFT)) {
      }
      if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT)) {
      }
      nk_menu_end(ctx);
    }

    nk_layout_row_end(ctx);
    nk_menubar_end(ctx);
  }
  nk_end(ctx);
}

void cpu_state(GUI_context *gui_ctx, const CPU_state state) {
  struct nk_context *ctx = gui_ctx->nk_ctx;
  if (nk_begin(ctx, "cpu", nk_rect(gui_ctx->win_width - 230, 30, 230, 100),
               NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE)) {

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
  }
  nk_end(ctx);
}

void asm_txt(GUI_context *gui_ctx, const char **text, uint16_t size,
             const CPU_state state) {
  struct nk_context *ctx = gui_ctx->nk_ctx;
  if (nk_begin(ctx, "asm",
               nk_rect(gui_ctx->win_width - 230, 130, 230,
                       gui_ctx->win_height - 130),
               NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE)) {

    // nk_layout_row_static(ctx, gui_ctx->win_height , 200, 1);
    nk_layout_row_static(ctx, 10, 220, 1);
    int len2;
    // nk_flags active = nk_edit_string(ctx, NK_EDIT_BOX, text, &len2,
    //                                 50 * (16 + 5 + 1) + 1, nk_filter_ascii);
    int l_pcs[10];
    for (int lines = 0, i = -1; lines < 10; --i) {
      int pc = (state.PC + i) % size;
      if (pc < 0 || !text[pc]) {
        continue;
      }
      // nk_label(ctx, text[pc], NK_TEXT_LEFT);
      l_pcs[lines] = pc;
      lines++;
    }
    for (int i = 9; i >= 0; --i) {
      nk_label(ctx, text[l_pcs[i]], NK_TEXT_LEFT);
    }
    if (text[(state.PC + 0) % size])
      nk_label_colored(ctx, text[(state.PC + 0) % size], NK_TEXT_LEFT,
                       nk_rgb(255, 255, 0));
    for (int lines = 0, i = 1; lines < 10; ++i) {
      int pc = (state.PC + i) % size;
      if (pc < 0 || !text[pc]) {
        continue;
      }
      nk_label(ctx, text[pc], NK_TEXT_LEFT);
      lines++;
    }
  }
  nk_end(ctx);
}

void draw_image(int x, int y, int width, int height, GLuint texture_id,
                uint8_t blendFlag) {
  // Bind Texture
  glBindTexture(GL_TEXTURE_2D, texture_id);

  if (blendFlag) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  GLfloat Vertices[] = {(float)x,
                        (float)y,
                        0,
                        (float)x + width,
                        (float)y,
                        0,
                        (float)x + (float)width,
                        (float)y + (float)height,
                        0,
                        (float)x,
                        (float)y + (float)height,
                        0};
  GLfloat TexCoord[] = {
      0, 0, 1, 0, 1, 1, 0, 1,
  };
  GLubyte indices[] = {
      0, 1, 2,  // first triangle (bottom left - top left - top right)
      0, 2, 3}; // second triangle (bottom left - top right - bottom right)

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, Vertices);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, 0, TexCoord);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  if (blendFlag)
    glDisable(GL_BLEND);
}