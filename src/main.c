#include "../src/BUS.h"
#include "../src/CPU_6502.h"
#include "../src/ROM.h"
#include "GUI.h"
#include "../deps/Nuklear/example.h" // must be after GUI.h
#include <stdio.h>

void example(GUI_context *gui_ctx) { overview(gui_ctx->nk_ctx); }

int main(int argc, char **argv) {

  char *rom_path;
  if (argc > 1) {
    rom_path = argv[1];
  } else {
    printf("input: [rom path]\n");
    exit(1);
  }
  NES_ROM rom;
  ROM_load_from_disc(rom_path, &rom);

  NES_BUS nes;
  BUS_init(&nes, &rom);
  // printf("   rom %p\n",&rom);
  char **dis_asm = NULL;

  CPU_disassemble_arr(ROM_get_PRG_p(&rom), rom.PRG_size, &dis_asm);
  GUI_context gui_ctx;
  GUI_init(&gui_ctx);

  struct nk_image pattern_table_img =
      GUI_image_rgba(&gui_ctx, nes.ppu.pattern_table_img, 128 * 2, 128);
  struct nk_image nes_screen =
      GUI_image_rgba(&gui_ctx, nes.ppu.screen, 256, 240);
  int run = 0;
  int close = 0;
  while (!close) {

    SDL_Event event;

    nk_input_begin(gui_ctx.nk_ctx);
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: close = 1; break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_SPACE:
          // printf("space pressed\n");
          BUS_tick(&nes);
          BUS_tick(&nes);
          BUS_tick(&nes);
          BUS_tick(&nes);
          while (nes.cpu.state.cycle_count > 0) BUS_tick(&nes);
          break;
        case SDLK_r: run = !run; break;
        case SDLK_f:
          while (!nes.ppu.frame_complete) {
            BUS_tick(&nes);
          }
          nes.ppu.frame_complete = 0;
          break;
        }
      default:
        nk_sdl_handle_event(&event);
        close = 0;
        break;
      }
    }
    nk_input_end(gui_ctx.nk_ctx);
    if (run) {
      for (int i = 0; i < 3000; ++i) {
        // if(nes.cpu.state.PC==0x8ef3){run=0;break;}
        // if(nes.cpu.state.PC==0x8ebb){run=0;break;}
        BUS_tick(&nes);
        while (nes.cpu.state.cycle_count > 0) BUS_tick(&nes);
      }
    }

    PPU_load_pattern_table(&nes.ppu, 0);
    PPU_load_pattern_table(&nes.ppu, 1);

    GUI_render_begin(&gui_ctx);

    GUI_menu_bar(&gui_ctx);
    example(&gui_ctx);
    GUI_cpu_state(&gui_ctx, &nes);
    GUI_ppu_state(&gui_ctx, &nes.ppu);
    GUI_asm_txt(&gui_ctx, dis_asm, rom.PRG_size,
                nes.cpu.state); // this is broken -- needs mappers

    if (nk_begin(gui_ctx.nk_ctx, "pattern table",
                 nk_rect(0, 0, 128 * 2 * 2, 128 * 2 + 64 + 30),
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE | NK_WINDOW_BORDER |
                     NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
      float wh = nk_window_get_height(gui_ctx.nk_ctx);
      nk_layout_row_dynamic(gui_ctx.nk_ctx, wh - 64 - 30, 1);

      GUI_image_refresh(&gui_ctx, &pattern_table_img, nes.ppu.pattern_table_img,
                        128 * 2, 128);
      nk_image(gui_ctx.nk_ctx, pattern_table_img);

      GUI_palette_view(&gui_ctx, &nes);
    }
    nk_end(gui_ctx.nk_ctx);

    if (nk_begin(gui_ctx.nk_ctx,
                 "ram  00  01  02   03  04  05   06  07  08   09  0A  0B   0C  "
                 "0D  0E  0F",
                 nk_rect(50, 400, 530, 380 + 16),
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE)) {
      GUI_cpu_ram_view(&gui_ctx, &nes);
    }
    nk_end(gui_ctx.nk_ctx);


    if (nk_begin(gui_ctx.nk_ctx,
                 "nam  00  01  02   03  04  05   06  07  08   09  0A  0B   0C  "
                 "0D  0E  0F",
                 nk_rect(50, 400, 530, 380 + 16),
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE)) {
      GUI_memory_view(&gui_ctx, nes.ppu.name_table,2048);
    }
    nk_end(gui_ctx.nk_ctx);

    if (nk_begin(gui_ctx.nk_ctx, "screen", nk_rect(0, 0, 256 * 2, 240 * 2),
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE | NK_WINDOW_BORDER |
                     NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
      float wh = nk_window_get_height(gui_ctx.nk_ctx);
      nk_layout_row_dynamic(gui_ctx.nk_ctx, wh, 1);
      GUI_image_refresh(&gui_ctx, &nes_screen, nes.ppu.screen, 256, 240);
      nk_image(gui_ctx.nk_ctx, nes_screen);
    }
    nk_end(gui_ctx.nk_ctx);
    GUI_render_end(&gui_ctx);

    // SDL_Delay(1000 / 60);
  }

  GUI_quit(&gui_ctx);

  return EXIT_SUCCESS;
}
