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

  //uint8_t pal[] = {1,2,3,4};
  //ROM_dump_CHR_to_BMP(&rom,pal);

  char *dis_asm = NULL;
  size_t asm_len = CPU_disassemble(ROM_get_PRG_p(&rom), rom.PRG_size, &dis_asm);
  //printf("%s %lu\n", dis_asm, asm_len);
  GUI_context gui_ctx;
  GUI_init(&gui_ctx);

  int close = 0;
  while (!close) {
    close = GUI_process_events(&gui_ctx);
    GUI_render_begin(&gui_ctx);

    Menu_bar(&gui_ctx);
    example(&gui_ctx);
    asm_txt(&gui_ctx, dis_asm, asm_len);

    GUI_render_end(&gui_ctx);
    SDL_Delay(1000 / 60);
  }

  GUI_quit(&gui_ctx);

  return EXIT_SUCCESS;
}