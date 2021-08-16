#include "../src/BUS.h"
#include "../src/CPU_6502.h"
#include "../src/ROM.h"
#include "GUI.h"
#include "../deps/Nuklear/example.h" // must be after GUI.h
#include <stdio.h>

#define RGBA_to_nk_color(col, in)                                              \
  col.a = (in & 0xFF);                                                         \
  col.b = ((in >> 8) & 0xFF);                                                  \
  col.g = ((in >> 16) & 0xFF);                                                 \
  col.r = (nk_byte)((in >> 24) & 0xFF);

struct nk_color RGBAu32_to_nk_color(uint32_t in){
  struct nk_color col;
  col.a = (in & 0xFF);                                                         
  col.b = ((in >> 8) & 0xFF);                                                  
  col.g = ((in >> 16) & 0xFF);                                                 
  col.r = (nk_byte)((in >> 24) & 0xFF);
  return col;
}

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
  BUS_init(&nes,&rom);

  char **dis_asm = NULL;
  size_t asm_len =
      CPU_disassemble_arr(ROM_get_PRG_p(&rom), rom.PRG_size, &dis_asm);
  // printf("%s %lu\n", dis_asm, asm_len);
  GUI_context gui_ctx;
  GUI_init(&gui_ctx);

  for (int i = 0; i < 2 * 128 * 128; ++i) {
    // if(i<800) nes.ppu.pattern_table[i] =0xff0000ff;
    // else nes.ppu.pattern_table[i]=0xffffffff;
    // nes.ppu.pattern_table[i] =0xff0000ff;
    nes.ppu.pattern_table[i] = 0x000000ff;
  }
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128 * 2, 128, 0, GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8, nes.ppu.pattern_table);
  struct nk_image img;
  img.h = 0;
  img.w = 0;
  img.handle.id = tex;
  //  printf("h:%d w:%d id:%d\n", img.h, img.w, img.handle.id);

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
          BUS_tick(&nes);BUS_tick(&nes);BUS_tick(&nes);BUS_tick(&nes);
          while (nes.cpu.state.cycle_count > 0) BUS_tick(&nes);
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
      for (int i = 0; i < 300; ++i) {
        BUS_tick(&nes);
        while (nes.cpu.state.cycle_count > 0) BUS_tick(&nes);
      }
    }
    // nes.ppu.pattern_table[test++%(128*2*128)] = 0xffffffff;
    PPU_load_pattern_table(&nes.ppu, 0);
    PPU_load_pattern_table(&nes.ppu, 1);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16 * 8 * 2, 16 * 8, GL_RGBA,
                    GL_UNSIGNED_INT_8_8_8_8, nes.ppu.pattern_table);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16 * 8 * 2, 16 * 8, 0, GL_RGBA,
    //           GL_UNSIGNED_INT_8_8_8_8, nes.ppu.pattern_table);

    GUI_render_begin(&gui_ctx);

    Menu_bar(&gui_ctx);
    example(&gui_ctx);
    cpu_state(&gui_ctx, nes.cpu.state);
    asm_txt(&gui_ctx, dis_asm, rom.PRG_size,
            nes.cpu.state); // this is broken --

    // draw_image(0,0,16*8*2,16*8,tex,0);
    if (nk_begin(gui_ctx.nk_ctx, "pattern table",
                 nk_rect(0, 0, 128 * 2 * 2, 128 * 2 + 64 + 30),
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE | NK_WINDOW_BORDER |
                     NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {

      float wh = nk_window_get_height(gui_ctx.nk_ctx);
      nk_layout_row_dynamic(gui_ctx.nk_ctx, wh - 64 - 30, 1);
      //      float r[2]={128*2,128};

      // nk_layout_row_dynamic(gui_ctx.nk_ctx,0,1);
      //      struct nk_rect rect = nk_layout_space_bounds(gui_ctx.nk_ctx);
      //      nk_layout_space_begin(gui_ctx.nk_ctx,NK_DYNAMIC,wh,INT_MAX);
      //      printf("x:%f y:%f h:%f w:%f    wid_h:%f
      //      win_h:%f\n",rect.x,rect.y,rect.h,rect.w,h,wh);
      //      nk_layout_space_push(gui_ctx.nk_ctx,nk_rect(0, 0, 1, 2));

      nk_image(gui_ctx.nk_ctx, img);
      //      nk_layout_space_end(gui_ctx.nk_ctx);
      int pal_h = 25;
      struct nk_color col;
      // clang-format off
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,8);
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,0,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,0,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,0,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,0,3)));
      nk_group_end(gui_ctx.nk_ctx);
        }
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,1,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,1,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,1,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,1,3)));
      nk_group_end(gui_ctx.nk_ctx);
        }
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,2,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,2,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,2,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,2,3)));

      nk_group_end(gui_ctx.nk_ctx);
        }
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,3,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,3,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,3,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,3,3)));

      nk_group_end(gui_ctx.nk_ctx);
        }
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,4,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,4,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,4,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,4,3)));

      nk_group_end(gui_ctx.nk_ctx);
        }
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,5,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,5,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,5,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,5,3)));

      nk_group_end(gui_ctx.nk_ctx);
        }
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,6,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,6,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,6,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,6,3)));

      nk_group_end(gui_ctx.nk_ctx);
        }
      if (nk_group_begin(gui_ctx.nk_ctx, "Group", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      nk_layout_row_dynamic(gui_ctx.nk_ctx,pal_h,4);
      
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,7,0)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,7,1)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,7,2)));
      nk_button_color(gui_ctx.nk_ctx, RGBAu32_to_nk_color(PPU_get_color_from_palette_ram(&nes.ppu,7,3)));

      nk_group_end(gui_ctx.nk_ctx);
        }

    }
    nk_end(gui_ctx.nk_ctx);
    // clang-format on
    /*     if (nk_begin(gui_ctx.nk_ctx, "Demo", nk_rect(50, 50, 230, 250),
                       NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE
       | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) { enum { EASY, HARD }; static
       int op = EASY; static int property = 20;

            nk_layout_row_static(gui_ctx.nk_ctx, 30, 80, 1);
            if (nk_button_label(gui_ctx.nk_ctx, "button"))
              printf("button pressed!\n");
            nk_layout_row_dynamic(gui_ctx.nk_ctx, 30, 2);
            if (nk_option_label(gui_ctx.nk_ctx, "easy", op == EASY))
              op = EASY;
            if (nk_option_label(gui_ctx.nk_ctx, "hard", op == HARD))
              op = HARD;
            nk_layout_row_dynamic(gui_ctx.nk_ctx, 22, 1);
            nk_property_int(gui_ctx.nk_ctx, "Compression:", 0, &property, 100,
       10, 1);

            nk_layout_row_dynamic(gui_ctx.nk_ctx, 20, 1);
            nk_label(gui_ctx.nk_ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(gui_ctx.nk_ctx, 25, 1);
            if (nk_combo_begin_color(gui_ctx.nk_ctx, nk_rgb_cf(gui_ctx.bg),
                                     nk_vec2(nk_widget_width(gui_ctx.nk_ctx),
       400))) { nk_layout_row_dynamic(gui_ctx.nk_ctx, 120, 1); gui_ctx.bg =
       nk_color_picker(gui_ctx.nk_ctx, gui_ctx.bg, NK_RGBA);
              nk_layout_row_dynamic(gui_ctx.nk_ctx, 25, 1);
              gui_ctx.bg.r = nk_propertyf(gui_ctx.nk_ctx, "#R:", 0,
       gui_ctx.bg.r, 1.0f, 0.01f, 0.005f); gui_ctx.bg.g =
       nk_propertyf(gui_ctx.nk_ctx, "#G:", 0, gui_ctx.bg.g, 1.0f, 0.01f,
       0.005f); gui_ctx.bg.b = nk_propertyf(gui_ctx.nk_ctx, "#B:", 0,
       gui_ctx.bg.b, 1.0f, 0.01f, 0.005f); gui_ctx.bg.a =
       nk_propertyf(gui_ctx.nk_ctx, "#A:", 0, gui_ctx.bg.a, 1.0f, 0.01f,
       0.005f); nk_combo_end(gui_ctx.nk_ctx);
            }
            nk_text(gui_ctx.nk_ctx, "hello dude", 11, NK_TEXT_ALIGN_LEFT);
          }
          nk_end(gui_ctx.nk_ctx); */

    GUI_render_end(&gui_ctx);

    // SDL_Delay(1000 / 60);
  }

  GUI_quit(&gui_ctx);

  return EXIT_SUCCESS;
}