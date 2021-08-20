#ifndef APU_H
#define APU_H
#include <stdint.h>
// NES is mono
typedef float APUsample;

typedef struct {
  uint8_t period;
  uint8_t clock;
} APU_divider;

inline void APU_divider_set(APU_divider *div, uint8_t val);
inline uint8_t APU_divider_tick(APU_divider *div); // return 1 if tick, 0 if not
inline void
APU_divider_reload(APU_divider *div); // reset and dont output a tick

typedef struct {
  APUsample val;
} APU_sequencer;

typedef struct {
  APUsample val;
} APU_timer;

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

/* typedef struct {
  uint32_t counter;
  uint8_t enabled;
} APU_sequencer;
void APU_sequencer_reload(APU_sequencer *seq);
void APU_sequencer_tick(APU_sequencer *seq); */





static const uint8_t Length_counter_lookup[16 * 2] = {
    10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};

typedef struct {
  uint8_t enabled;
  uint8_t halt;
  uint8_t new_halt;

  uint8_t length_counter;
  uint8_t prev_length_counter;
  uint8_t reload_length_counter;

} APU_length_counter;

void APU_length_counter_tick(APU_length_counter *lc);
void APU_length_counter_set_enabled(APU_length_counter *lc, uint8_t enabled);
void APU_length_counter_reset(APU_length_counter *lc);
void APU_length_counter_reload(APU_length_counter *lc);

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

static const uint8_t Square_duty_lookup[4][8] =  {
//  Output waveform               Duty
	{0, 1, 0, 0, 0, 0, 0, 0},  //  0    (12.5%)
	{0, 1, 1, 0, 0, 0, 0, 0},  //  1    (25%)
	{0, 1, 1, 1, 1, 0, 0, 0},  //  2    (50%)
	{1, 0, 0, 1, 1, 1, 1, 1},};//  3    (25% negated)

#define SQR_CHANNEL_1 1
typedef struct {
  APU_envelope envelope;
  APU_length_counter lc;

// sweep
  APU_sweep sweep;
  uint8_t sweep_negate_mode;
  uint32_t sweep_target_period;
  uint16_t real_period;
  
  // controlls sound frequency
  // The triangle channel's timer is clocked on every CPU cycle, but the pulse, noise, and DMC timers are clocked only on every second CPU cycle and thus produce only even periods.
  APU_divider timer;// ticked by cpu

  uint16_t period;//??

  uint8_t duty_cycle;
  uint8_t duty_pos;
} APU_square_channel;

//void APU_square_channel_reset(APU_square_channel *sq);
void APU_square_channel_update_sweep(APU_square_channel *sq);
void APU_square_channel_set_period(APU_square_channel *sq, uint16_t period);
void APU_square_channel_tick(APU_square_channel *sq);
uint8_t APU_square_channel_get_output(APU_square_channel *sq);
void APU_square_channel_tick_envelope(APU_square_channel *sq);
void APU_square_channel_tick_len_counter(APU_square_channel *sq);





typedef struct {
  APU_square_channel sqr1,sqr2;
  


  uint32_t previous_cycle;
  uint32_t current_cycle;
} APU;


void APU_init(APU *apu);
void APU_reset(APU *apu);
void APU_frame_counter_tick(APU *apu, uint8_t half_frame);
void APU_end_frame(APU * apu);




#endif // APU_H