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


void asm_txt(GUI_context *gui_ctx, char *text, size_t text_len) {
  struct nk_context *ctx = gui_ctx->nk_ctx;
  if (nk_begin(ctx, "asm",
               nk_rect(gui_ctx->win_width - 230, 30, 230, gui_ctx->win_height-30),
                 NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE )) {

    nk_layout_row_static(ctx, gui_ctx->win_height-80, 200, 1);
    nk_flags active;
    int len2;
    active =
        nk_edit_string(ctx, NK_EDIT_BOX, text, &len2, 50*(16+5+1)+1, nk_filter_ascii);
  }
  nk_end(ctx);
}