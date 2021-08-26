#ifndef APU_H
#define APU_H
#include <SDL2/SDL.h>
#include <stdint.h>

#define DEBUG_AUDIO

// NES is mono
typedef float APUsample;

typedef struct {
  uint16_t period;
  uint16_t clock;
} APU_divider;

// void APU_divider_set(APU_divider *div, uint8_t val);
// uint8_t APU_divider_tick(APU_divider *div); // return 1 if tick, 0 if not
// void APU_divider_reload(APU_divider *div);

typedef struct {
  APUsample val;
} APU_sequencer;

typedef struct {
  APUsample val;
} APU_timer;

//#############################################################
// ENVELOPE
//#############################################################
// very simple hardware envelope on the NES
// can generate:
// a saw envelope, i.e. just a decreasing line with looping
// or constant volume
typedef struct {
  // flags
  uint8_t loop;
  uint8_t start;
  uint8_t constant;

  uint8_t gain;
  uint8_t decay_gain;
  APU_divider divider;

  
} APU_envelope;

void APU_envelope_tick(APU_envelope *env);
uint8_t APU_envelope_get_gain(APU_envelope *env);
void APU_envelope_init(APU_envelope *env, uint8_t reg);
// void APU_envelope_reset(APU_envelope *env); // just set env->start = TRUE

/* typedef struct {
  uint32_t counter;
  uint8_t enabled;
} APU_sequencer;
void APU_sequencer_reload(APU_sequencer *seq);
void APU_sequencer_tick(APU_sequencer *seq); */

//#############################################################
// LENGTH COUNTER
//#############################################################
static const uint8_t Length_counter_lookup[16 * 2] = {
    10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};

typedef struct {
  uint8_t enabled;//=0
  uint8_t halt;
  uint8_t new_halt;

  uint8_t length_counter;
  uint8_t prev_length_counter;
  uint8_t reload_length_counter;

} APU_length_counter;

void APU_length_counter_tick(APU_length_counter *lc);
void APU_length_counter_set_enabled(APU_length_counter *lc, uint8_t enabled);
void APU_length_counter_reset(APU_length_counter *lc);
void APU_length_counter_load(APU_length_counter *lc, uint8_t idx);
void APU_length_counter_reload(APU_length_counter *lc);
//#############################################################
// SWEEP
//#############################################################
typedef struct {

  uint8_t enabled;
  uint8_t reload;
  uint8_t negate; //  0: add to period, sweeping toward lower frequencies 1:
                  //  subtract from period, sweeping toward higher frequencies
  uint8_t negate_mode; // for different behavior of square 1/2

  uint8_t shift_count;
  APU_divider divider;
} APU_sweep;
void APU_sweep_init(APU_sweep *sw, uint8_t reg);

//#############################################################
// SQUARE CHANNEL
//#############################################################
static const uint8_t Square_duty_lookup[4][8] = {
    //  Output waveform               Duty
    {0, 0, 0, 0, 0, 0, 0, 1}, //  0    (12.5%)
    {0, 0, 0, 0, 0, 0, 1, 1}, //  1    (25%)
    {0, 0, 0, 0, 1, 1, 1, 1}, //  2    (50%)
    {1, 1, 1, 1, 1, 1, 0, 0},
}; //  3    (25% negated)

#define SQR_CHANNEL_1 1
typedef struct {
  APU_envelope envelope;
  APU_length_counter lc;

  // sweep
  APU_sweep sweep;
  uint8_t sweep_negate_mode; // = 0
  uint32_t sweep_target_period;// = 0

  // controlls sound frequency
  // The triangle channel's timer is clocked on every CPU cycle, but the pulse,
  // noise, and DMC timers are clocked only on every second CPU cycle and thus
  // produce only even periods.
//  APU_divider timer; // ticked by cpu


  uint8_t duty_cycle;// = 0
  uint8_t duty_pos;// = 0

  // timer stuff
  uint32_t prev_cycle;//init = 0
  uint8_t last_output;// = 0
  uint16_t timer; // = 0
  uint16_t real_period; // = 0
  uint16_t period;// = 0

} APU_square_channel;

void    APU_square_channel_reset(APU_square_channel *sq);
void    APU_square_channel_update_sweep(APU_square_channel *sq);// UpdateTargetPeriod
void    APU_square_channel_set_period(APU_square_channel *sq, uint16_t period);
void    APU_square_channel_tick(APU_square_channel *sq);
uint8_t APU_square_channel_get_output(APU_square_channel *sq);

//#############################################################
// FRAME COUNTER STUFF
//#############################################################
//Mode 0: 4-Step Sequence (bit 7 of $4017 clear)
//Step   |APU cycles|Envelopes & triangle's linear counter (Quarter frame)|	Length counters & sweep units (Half frame)|	Frame interrupt flag|
//1      |3728.5    |Clock|     |                                 |
//2      |7456.5    |Clock|Clock|                                 |
//3      |11185.5   |Clock|     |                                 |
//4      |14914     |     |     |Set if interrupt inhibit is clear|
//5      |14914.5   |Clock|Clock|Set if interrupt inhibit is clear|
//6      |0 (14915) |     |     |Set if interrupt inhibit is clear|

//Mode 1: 5-Step Sequence (bit 7 of $4017 set)
//Step  |APU cycles|Envelopes & triangle's linear counter (Quarter frame)|	Length counters & sweep units (Half frame)|
//1     |3728.5    |Clock|     |
//2     |7456.5    |Clock|Clock|
//3     |11185.5   |Clock|     |
//4     |14914.5   |     |     |
//5     |18640.5   |Clock|Clock|
//6     |0 (18641) |     |     |

#define QUARTER_FRAME 4
#define HALF_FRAME 2
#define SKIP_FRAME 0

//                       [sequence_mode][step]
static const uint8_t Frame_type_table[2][6] = {
    {QUARTER_FRAME, HALF_FRAME, QUARTER_FRAME, SKIP_FRAME, HALF_FRAME,
     SKIP_FRAME},
    {QUARTER_FRAME, HALF_FRAME, QUARTER_FRAME, SKIP_FRAME, HALF_FRAME,
     SKIP_FRAME}};

// unit= 2* APU cycles = CPU cycles
//                             [sequence_mode][step]  hmmm
static const uint32_t Step_cycles_ntsc_table[2][6] = {
    {7457, 14913, 22371, 29828, 29829, 29830},
    {7457, 14913, 22371, 29829, 37281, 37282}};

//#############################################################
// APU
//#############################################################
typedef struct {
  int8_t write_delay_0x4017;
  uint8_t sequence_mode_delayed_val;
  // Frame counter
  uint8_t sequence_mode; //  0 selects 4-step sequence, 1 selects 5-step sequence
  uint8_t Frame_counter_IRQ_flag;
  uint8_t IRQ_inhibit_flag;
  uint32_t cycle; // in "cpu" ticks i.e. real APU cycles * 2
  uint8_t sequence_step;
  
  uint8_t DMC_IRQ_flag;

  APU_square_channel sqr1, sqr2;

  int32_t previous_cycle;// = 0
  uint32_t current_step; // = 0
  uint32_t current_cycle;

  // Audio output stuff
  // audio device call back
  SDL_AudioCallback fill_audio_buffer;
  // the required sample output format
  SDL_AudioFormat format;

  uint8_t block_frame_counter_tick;//=0

#ifdef DEBUG_AUDIO
  FILE *sqr1_f;
  uint8_t start_sqr1_rec;
  FILE *sqr2_f;
  uint8_t start_sqr2_rec;
#endif

} APU;

#define STEP_4_SEQ 0
#define STEP_5_SEQ 1

void APU_cpu_write(APU *apu, uint16_t adr, uint8_t data, uint8_t during_apu_cycle);
uint8_t APU_cpu_read(APU *apu, uint16_t adr);
// void APU_square_channel_reset(APU_square_channel *sq);

void APU_fill_audio_buffer(void *apu, uint8_t *stream, int len);

void APU_init(APU *apu);
void APU_reset(APU *apu);
void APU_frame_counter_tick(APU *apu);
void APU_tick(APU *apu);
void APU_end_frame(APU *apu);

#endif // APU_H