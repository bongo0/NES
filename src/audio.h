#ifndef AUDIO_H
#define AUDIO_H

#include "APU.h"
#include <SDL2/SDL.h>

typedef struct{
  SDL_AudioSpec obtained_spec;
  
  // ID of the currently active device
  SDL_AudioDeviceID dev_id;
} Audio_context;

void Audio_init(Audio_context *actx, uint32_t sample_rate, uint16_t buf_size, APU *apu);



void Audio_quit();

#endif // AUDIO_H