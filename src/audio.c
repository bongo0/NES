#include "audio.h"
#include "logger.h"

void Audio_init(Audio_context *actx, uint32_t sample_rate, uint16_t buf_size,
                APU *apu) {
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
    LOG_ERROR("[AUDIO] Failed to initialize audio\n");
    return;
  }

  // spec that we want
  SDL_AudioSpec spec;
  SDL_memset(&spec, 0, sizeof(spec));
  spec.format = AUDIO_S16SYS; // signed 16 bit samples AUDIO_S16SYS
  spec.freq = sample_rate;
  spec.channels = 1;
  spec.samples = 2 * buf_size;
  spec.callback = apu->fill_audio_buffer;
  spec.userdata = apu;

  // get available devices
  int dev_count = SDL_GetNumAudioDevices(0);
  if (dev_count == -1)
    LOG_ERROR("[AUDIO] No audio devices found\n");
  for (int i = 0; i < dev_count; i++) {
    LOG("[AUDIO] Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
  }

  SDL_AudioSpec got_spec;
  SDL_memset(&got_spec, 0, sizeof(got_spec));
  actx->dev_id = SDL_OpenAudioDevice(NULL, 0, &spec, &actx->obtained_spec, 0);

  LOG("[AUDIO] Obtained audio spec:\n"
      "    format:  0x%04X\n"
      "    sr:      %d\n"
      "    channels:%d\n"
      "    samples: %d\n",
      actx->obtained_spec.format, actx->obtained_spec.freq, actx->obtained_spec.channels, actx->obtained_spec.samples);

  // tell APU what is the sample format
  apu->format = got_spec.format;
}



void Audio_quit() { SDL_CloseAudio(); }
