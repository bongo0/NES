#include "../src/CPU_6502.h"
#include "../src/ROM.h"

#include <stdio.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "../deps/Nuklear/conf_nuklear.h"
#include "../deps/Nuklear/example.h"
#define WIN_WIDTH 1200
#define WIN_HEIGHT 800


int main(int argc, char **argv) {

  //  char *rom_path;
  //  if (argc > 1) {
  //    rom_path = argv[1];
  //  } else {
  //    printf("input: [rom path]\n");
  //    exit(1);
  //  }
  //  NES_ROM rom;
  //  ROM_load_from_disc(rom_path, &rom);
  //  uint8_t palette[4] = {0x05,0x0a,0x0c,0x11};
  //  ROM_dump_CHR_to_BMP(&rom, palette);
  int win_width, win_height;
  SDL_Window *window;
  SDL_GLContext gl_ctx;

  struct nk_context *ctx;
  struct nk_colorf bg;

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    LOG_ERROR("SDL INIT ERROR: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  window = SDL_CreateWindow("NES", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                                SDL_WINDOW_ALLOW_HIGHDPI);
  if (window == NULL) {
    LOG_ERROR("SDL_CreateWindow ERROR: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  gl_ctx = SDL_GL_CreateContext(window);

  // SDL_Renderer *renderer = SDL_CreateRenderer(
  //    window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  // if (renderer == NULL) {
  //  LOG_ERROR("SDL_CreateRenderer ERROR: %s\n", SDL_GetError());
  //  SDL_DestroyWindow(window);
  //  SDL_Quit();
  //  return EXIT_FAILURE;
  //}

  // OpenGL setup
  glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
  glewExperimental = 1;
  if (glewInit() != GLEW_OK) {
    LOG_ERROR("Failed to setup GLEW\n");
    return EXIT_FAILURE;
  }

  ctx = nk_sdl_init(window);

  {
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
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

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    uint8_t close = 0;
    while (!close) {

      SDL_Event event;
      nk_input_begin(ctx);
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: close = 1; break;
        default: nk_sdl_handle_event(&event); break;
        }
      }
      nk_input_end(ctx);

      // ########gui test
      if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
                   NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                       NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
        enum { EASY, HARD };
        static int op = EASY;
        static int property = 20;

        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button"))
          printf("button pressed!\n");
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY))
          op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD))
          op = HARD;
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "background:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, nk_rgb_cf(bg),
                                 nk_vec2(nk_widget_width(ctx), 400))) {
          nk_layout_row_dynamic(ctx, 120, 1);
          bg = nk_color_picker(ctx, bg, NK_RGBA);
          nk_layout_row_dynamic(ctx, 25, 1);
          bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
          bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
          bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
          bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
          nk_combo_end(ctx);
        }
        nk_text(ctx, "hello dude", 11, NK_TEXT_ALIGN_LEFT);
      }
      nk_end(ctx);
      // ########
      overview(ctx);

      SDL_GetWindowSize(window, &win_width, &win_height);
      glViewport(0, 0, win_width, win_height);
      glClear(GL_COLOR_BUFFER_BIT);
      glClearColor(bg.r, bg.g, bg.b, bg.a);
      /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
       * with blending, scissor, face culling, depth test and viewport and
       * defaults everything back into a default state.
       * Make sure to either a.) save and restore or b.) reset your own state
       * after rendering the UI. */
      nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
      SDL_GL_SwapWindow(window);

      SDL_Delay(1000 / 60);
    }

    // SDL_DestroyRenderer(renderer);
    nk_sdl_shutdown();
    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}