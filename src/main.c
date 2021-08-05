#include "../src/CPU_6502.h"
#include "../src/ROM.h"

#include <stdio.h>

#include <SDL2/SDL.h>

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

  uint8_t palette[4] = {0x05,0x0a,0x0c,0x11};
  ROM_dump_CHR_to_BMP(&rom, palette);

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    LOG_ERROR("SDL INIT ERROR: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Window *window =
      SDL_CreateWindow("NES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       630, 387, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    LOG_ERROR("SDL_CreateWindow ERROR: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    LOG_ERROR("SDL_CreateRenderer ERROR: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  uint8_t close = 0;
  while (!close) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: close = 1; break;
      default: break;
      }
    }
    SDL_Delay(1000 / 60);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}