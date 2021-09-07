// clang-format off
#include "../src/BUS.h"
#include "../src/CPU_6502.h"
#include "../src/ROM.h"
#include "GUI.h"
#include "../deps/Nuklear/example.h" // must be after GUI.h
#include "disassembler.h"
#include "audio.h"
#include <stdio.h>
#include <time.h>
#include <sched.h>
// clang-format on

void example(GUI_context *gui_ctx) { overview(gui_ctx->nk_ctx); }

#define TRACE_LOG_MAX_LINES 100000
// quite spaghetti - mamma mia
struct trace_func_data {
  uint16_t last_PC;
  uint32_t line_repeat;
  int log_num;
  int line_num;
  char log_file[128];
  char *rom_path;
  FILE *trace_log_f;
  NES_BUS *nes;
  GUI_context *gui_ctx;
  Disassembly6502 *dis_asm;
};
void trace_func(void *data) {
  if (data == NULL)
    return;
  struct trace_func_data *d = (struct trace_func_data *)data;
  if (d->line_num > TRACE_LOG_MAX_LINES)
    d->gui_ctx->state.write_tracelog = 0;
  NES_BUS *nes = d->nes;
  // TRACE LOG
  if (d->gui_ctx->state.write_tracelog) {
    if (d->trace_log_f == NULL) {
      snprintf(d->log_file, 128, "%s.%d.log", d->rom_path, d->log_num);
      d->trace_log_f = fopen(d->log_file, "w");
      d->log_num++;
    }

    if (d->last_PC != d->nes->cpu.state.PC) {
      if (d->line_repeat != 0) {
        fprintf(d->trace_log_f, "^ * %u times\n", d->line_repeat);
      }
      d->line_repeat = 0;
      uint32_t mapped_adr =
          nes->cpu.state.PC; // set to PC if mapper does not map it
      uint8_t dat;
      /*uint8_t map_flag =*/nes->rom->mapper.cpu_read(
          nes->rom->mapper.state, nes->cpu.state.PC, &mapped_adr, &dat);
      fprintf(d->trace_log_f,
              "%04X  %s    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%3u SL:%d\n",
              nes->cpu.state.PC, d->dis_asm->lines[mapped_adr],
              nes->cpu.state.A, nes->cpu.state.X, nes->cpu.state.Y,
              nes->cpu.state.P, nes->cpu.state.SP, nes->ppu.cycle,
              nes->ppu.scan_line);
      d->line_num++;
    } else {
      d->line_repeat++;
    }
    d->last_PC = d->nes->cpu.state.PC;

  } else {
    if (d->trace_log_f != NULL) {
      fclose(d->trace_log_f);
      d->trace_log_f = NULL;
      d->line_num = 0;
    }
  }
}

int main(int argc, char **argv) {

  /*   struct sched_param sched;
    sched.sched_priority=99;
    int priority_max = sched_get_priority_max(SCHED_FIFO);
    LOG("SCHED_FIFO priority max: %d\n",priority_max);
    if(sched_setscheduler(0, SCHED_FIFO, &sched)!=0){
      LOG_ERROR("Could not set realtime scheduler\n");
    } */

  char *rom_path;
  if (argc > 1) {
    rom_path = argv[1];
  } else {
    printf("input: [rom path]\n");
    exit(1);
  }

  // testing
  Audio_context a_ctx;
  //-------

  NES_ROM rom;
  ROM_load_from_disc(rom_path, &rom);

  NES_BUS nes;
  BUS_init(&nes, &rom);

  Audio_init(&a_ctx, 44100, 1 << 12, &nes.apu);

  Disassembly6502 dis_asm;
  Disassemble(rom.PRG_p, rom.PRG_size, &dis_asm);

  GUI_context gui_ctx;
  GUI_init(&gui_ctx, &nes);

  // trace log stuff
  struct trace_func_data d;
  d.last_PC = 0;
  d.line_num = 0;
  d.log_num = 0;
  d.line_repeat = 0;
  d.trace_log_f = NULL;
  d.nes = &nes;
  d.gui_ctx = &gui_ctx;
  d.dis_asm = &dis_asm;
  d.rom_path = rom_path;
  nes.trace_data = &d;
  nes.trace_log = &trace_func;

  struct nk_image pattern_table_img =
      GUI_image_rgba(&gui_ctx, nes.ppu.pattern_table_img, 128 * 2, 128);
  struct nk_image nes_screen =
      GUI_image_rgba(&gui_ctx, nes.ppu.screen, 256, 240);
  int run = 0;
  int close = 0;
  int last_scanl = 0;
  int audio = 0;
  double dTime = 0;
  double dt_avr = 0;
  size_t frame_count = 0;
  while (!close) {
    uint64_t t_start = SDL_GetPerformanceCounter();

    SDL_Event event;

    nk_input_begin(gui_ctx.nk_ctx);
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: close = 1; break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_SPACE:
          BUS_tick(&nes);
          BUS_tick(&nes);
          BUS_tick(&nes);
          BUS_tick(&nes);
          while (nes.cpu.state.cycle_count > 0) BUS_tick(&nes);
          break;
        case SDLK_MINUS:
          while (last_scanl == nes.ppu.scan_line) {
            BUS_tick(&nes);
          }
          last_scanl = nes.ppu.scan_line;
          break;
        case SDLK_r: run = !run; break;
        case SDLK_m: audio = !audio; break;
        case SDLK_F1: BUS_reset(&nes); break;
        default: break;
        }
        break;
      default: close = 0; break;
      }
      nk_sdl_handle_event(&event);
    }
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);
    // clang-format off
    // Controller #1
    nes.controller[0]=0;
    if(keyboard_state[SDL_SCANCODE_D]){           nes.controller[0]|=(1<<0);}//Right
    if(keyboard_state[SDL_SCANCODE_A]){           nes.controller[0]|=(1<<1);}//Left
    if(keyboard_state[SDL_SCANCODE_S]){           nes.controller[0]|=(1<<2);}//Down
    if(keyboard_state[SDL_SCANCODE_W]){           nes.controller[0]|=(1<<3);}//Up
    if(keyboard_state[SDL_SCANCODE_H]){           nes.controller[0]|=(1<<4);}//Start
    if(keyboard_state[SDL_SCANCODE_J]){           nes.controller[0]|=(1<<5);}//Select
    if(keyboard_state[SDL_SCANCODE_K]){ nes.controller[0]|=(1<<6);}//B
    if(keyboard_state[SDL_SCANCODE_L]){nes.controller[0]|=(1<<7);}//A
    // clang-format on
    nk_input_end(gui_ctx.nk_ctx);

    if (run) {
      while (!nes.ppu.frame_complete) {
        BUS_tick(&nes);
      }
      nes.ppu.frame_complete = 0;
    }

    if (audio) {
      SDL_PauseAudioDevice(a_ctx.dev_id, 1);
    } else {
      SDL_PauseAudioDevice(a_ctx.dev_id, 0);
    }

    if (gui_ctx.state.show_Pattern_table) {
      PPU_load_pattern_table(&nes.ppu, 0);
      PPU_load_pattern_table(&nes.ppu, 1);
    }

    GUI_render_begin(&gui_ctx);

    GUI_menu_bar(&gui_ctx);
#if 1
    // example(&gui_ctx);
    if (gui_ctx.state.show_CPU_state)
      GUI_cpu_state(&gui_ctx, &nes);
    if (gui_ctx.state.show_PPU_state)
      GUI_ppu_state(&gui_ctx, &nes.ppu);
    if (gui_ctx.state.show_disasm)
      GUI_asm_txt(&gui_ctx, &dis_asm, &nes);

    GUI_apu_dB(&gui_ctx,&nes.apu);
    /* // TRACE LOG
    if (gui_ctx.state.write_tracelog && run) {
      if (trace_log_f == NULL) {
        snprintf(log_file, 128, "%s.%d.log", rom_path, log_num);
        trace_log_f = fopen(log_file, "w");
        log_num++;
      }
      uint32_t mapped_adr;
      uint8_t dat;
      uint8_t map_flag = nes.rom->mapper.cpu_read(
          nes.rom->mapper.state, nes.cpu.state.PC, &mapped_adr, &dat);
      fprintf(trace_log_f,
              "%04X  %s    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%3lu
    SL:%d\n", nes.cpu.state.PC, dis_asm.lines[mapped_adr], nes.cpu.state.A,
              nes.cpu.state.X, nes.cpu.state.Y, nes.cpu.state.P,
              nes.cpu.state.SP, nes.cpu.state.cycles_accumulated % 341,
              nes.ppu.scan_line);
    } else {
      if (trace_log_f != NULL) {
        fclose(trace_log_f);
        trace_log_f = NULL;
      }
    } */

    if (gui_ctx.state.show_Pattern_table) {
      if (nk_begin(gui_ctx.nk_ctx, "pattern table",
                   nk_rect(0, 180, 128 * 2 * 2, 128 * 2 + 64 + 30),
                   NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE | NK_WINDOW_BORDER |
                       NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
        float wh = nk_window_get_height(gui_ctx.nk_ctx);
        nk_layout_row_dynamic(gui_ctx.nk_ctx, wh - 64 - 30, 1);

        GUI_image_refresh(&gui_ctx, &pattern_table_img,
                          nes.ppu.pattern_table_img, 128 * 2, 128);
        nk_image(gui_ctx.nk_ctx, pattern_table_img);

        GUI_palette_view(&gui_ctx, &nes);
      }
      nk_end(gui_ctx.nk_ctx);
    }

    GUI_memory(&gui_ctx);

    if (nk_begin(gui_ctx.nk_ctx, "screen",
                 nk_rect(gui_ctx.win_width - 256 * 2, 0, 256 * 2, 256 * 2),
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE)) {

      struct nk_window *win = gui_ctx.nk_ctx->current;
      nk_layout_row_static(gui_ctx.nk_ctx, 13, 13, 2);
      if (nk_button_label(gui_ctx.nk_ctx, "_")) {
        // half the screen min
        int len = (gui_ctx.win_height < gui_ctx.win_width ? gui_ctx.win_height
                                                          : gui_ctx.win_width) /
                  2;
        win->bounds.h = len;
        win->bounds.w = len;
      }
      if (nk_button_symbol(gui_ctx.nk_ctx, NK_SYMBOL_RECT_OUTLINE)) {
        // fit nes_screen to screen
        int len = gui_ctx.win_height < gui_ctx.win_width ? gui_ctx.win_height
                                                         : gui_ctx.win_width;
        win->bounds.h = len;
        win->bounds.w = len;
        win->bounds.x = gui_ctx.win_width - len;
        win->bounds.y = 0;
      }

      float wh = nk_window_get_height(gui_ctx.nk_ctx);
      nk_layout_row_dynamic(gui_ctx.nk_ctx, wh - 50, 1);
      GUI_image_refresh(&gui_ctx, &nes_screen, nes.ppu.screen, 256, 240);
      nk_image(gui_ctx.nk_ctx, nes_screen);

      /* window movement by dragging */
      if ((win->flags & NK_WINDOW_MOVABLE) && !(win->flags & NK_WINDOW_ROM)) {
        int left_mouse_down;
        int left_mouse_clicked;
        int left_mouse_click_in_cursor;
        /* calculate draggable window space */
        struct nk_rect header;
        struct nk_input *in = &gui_ctx.nk_ctx->input;
        header.x = win->bounds.x;
        header.y = win->bounds.y;
        header.w = win->bounds.w;
        header.h = win->bounds.h > 40 ? win->bounds.h - 30 : win->bounds.h;
        left_mouse_down = in->mouse.buttons[NK_BUTTON_LEFT].down;
        left_mouse_clicked = (int)in->mouse.buttons[NK_BUTTON_LEFT].clicked;
        left_mouse_click_in_cursor = nk_input_has_mouse_click_down_in_rect(
            in, NK_BUTTON_LEFT, header, nk_true);
        if (left_mouse_down && left_mouse_click_in_cursor &&
            !left_mouse_clicked) {
          win->bounds.x = win->bounds.x + in->mouse.delta.x;
          win->bounds.y = win->bounds.y + in->mouse.delta.y;
          in->mouse.buttons[NK_BUTTON_LEFT].clicked_pos.x += in->mouse.delta.x;
          in->mouse.buttons[NK_BUTTON_LEFT].clicked_pos.y += in->mouse.delta.y;
          gui_ctx.nk_ctx->style.cursor_active =
              gui_ctx.nk_ctx->style.cursors[NK_CURSOR_MOVE];
        }
      }
    }
    nk_end(gui_ctx.nk_ctx);

#endif
    GUI_render_end(&gui_ctx);

    // SDL_Delay(1000 / 60);
    uint64_t t_end = SDL_GetPerformanceCounter();
    dTime = (t_end - t_start) / (double)SDL_GetPerformanceFrequency();
    if (dTime * 1000.0 > 16.67)
      printf("fps: %g  dt: %g  floor ms%u\n", 1.0 / dTime, dTime,
             (uint32_t)(dTime * 1000.0 - 16.7));
    dt_avr += dTime;
    frame_count++;

    // spin lock, not ideal but...
    while (dTime * 1000.0 < 16.66666666666666666) {
      struct timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = 1;
      nanosleep(&ts, NULL);

      t_end = SDL_GetPerformanceCounter();
      dTime = (t_end - t_start) / (double)SDL_GetPerformanceFrequency();
    }
    /* #define FRAME_TIME 0.01666666667
        if(dTime<FRAME_TIME){
          uint64_t t_sleep = SDL_GetPerformanceCounter();

          struct timespec ts;
          ts.tv_sec=0;
          ts.tv_nsec = (int64_t)(((FRAME_TIME-dTime)*10e8)/10.0);
          //printf("ts nsec: %ld\n",ts.tv_nsec);
          nanosleep(&ts,NULL);
          uint64_t t_woke = SDL_GetPerformanceCounter();
          double dSleep =
       (t_woke-t_sleep)/(double)SDL_GetPerformanceFrequency();
          while(dTime+dSleep<FRAME_TIME){
            t_woke = SDL_GetPerformanceCounter();
            dSleep = (t_woke-t_sleep)/(double)SDL_GetPerformanceFrequency();
          }

        } */
  }
  printf("avr game loop ms: %g\n", ((dt_avr) / (double)frame_count));

  GUI_quit(&gui_ctx);
  Audio_quit();
  ROM_free(&rom);
  BUS_free(&nes);
  return EXIT_SUCCESS;
}
