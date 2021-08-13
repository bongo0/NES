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

  // uint8_t pal[] = {1,2,3,4};
  // ROM_dump_CHR_to_BMP(&rom,pal);
  NES_BUS nes;
  nes.rom = &rom;
  PPU_init(&nes.ppu, nes.rom);
  CPU_init(&nes.cpu, &nes);
  CPU_reset(&nes.cpu);
  char **dis_asm = NULL;
  size_t asm_len =
      CPU_disassemble_arr(ROM_get_PRG_p(&rom), rom.PRG_size, &dis_asm);
  // printf("%s %lu\n", dis_asm, asm_len);
  GUI_context gui_ctx;
  GUI_init(&gui_ctx);

  for (int i = 0; i < 2 * 128 * 128; ++i) {
    //if(i<800) nes.ppu.pattern_table[i] =0xff0000ff;
    //else nes.ppu.pattern_table[i]=0xffffffff;
    //nes.ppu.pattern_table[i] =0xff0000ff;
    nes.ppu.pattern_table[i]=0x000000ff;
  }
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128*2, 128, 0, GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8, nes.ppu.pattern_table);
  struct nk_image img;
  img.h = 0;
  img.w = 0;
  img.handle.id = tex;
  printf("h:%d w:%d id:%d\n", img.h, img.w, img.handle.id);

  int run = 0;
  int close = 0;
  int test = 0;
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
          CPU_tick(&nes.cpu);
          while (nes.cpu.state.cycle_count > 0) CPU_tick(&nes.cpu);
          break;
        case SDLK_r: run = !run;
        }
      default:
        nk_sdl_handle_event(&event);
        close = 0;
        break;
      }
    }
    nk_input_end(gui_ctx.nk_ctx);
    if (run) {
      for (int i = 0; i < 100; ++i) {
        CPU_tick(&nes.cpu);
        while (nes.cpu.state.cycle_count > 0) CPU_tick(&nes.cpu);
      }
    }
    //nes.ppu.pattern_table[test++%(128*2*128)] = 0xffffffff;
    PPU_load_pattern_table(&nes.ppu, 0);
    PPU_load_pattern_table(&nes.ppu, 1);
    
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D,0, 0, 0, 16 * 8 * 2, 16 * 8, GL_RGBA,
                    GL_UNSIGNED_INT_8_8_8_8, nes.ppu.pattern_table);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16 * 8 * 2, 16 * 8, 0, GL_RGBA,
    //           GL_UNSIGNED_INT_8_8_8_8, nes.ppu.pattern_table);

    GUI_render_begin(&gui_ctx);

    Menu_bar(&gui_ctx);
    example(&gui_ctx);
    cpu_state(&gui_ctx, nes.cpu.state);
    asm_txt(&gui_ctx, dis_asm, rom.PRG_size,
            nes.cpu.state); // this is broken --

    // draw_image(0,0,16*8*2,16*8,tex,0);
    if (nk_begin(gui_ctx.nk_ctx, "pattern table",
                 nk_rect(0, 0, 128*2*2, 128*2),
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE |
                     NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)) {
      //nk_layout_row_dynamic(gui_ctx.nk_ctx, 128*2, 1);
      nk_layout_row_dynamic(gui_ctx.nk_ctx,128*2,1);
      nk_image(gui_ctx.nk_ctx, img);
    }
    nk_end(gui_ctx.nk_ctx);

    GUI_render_end(&gui_ctx);

    // SDL_Delay(1000 / 60);
  }

  GUI_quit(&gui_ctx);

  return EXIT_SUCCESS;
}