#include "APU.h"
#include "timers.h"

float dB_to_gain(float dB) { return pow(10.0, dB / 20.0); }

float gain_to_dB(float gain) {
  if (gain <= 0)
    return -INFINITY;
  return 20.0 * log10(gain);
}

//#############################################################
// DIVIDER
//#############################################################

static inline void APU_divider_set(APU_divider *div, uint8_t val) {
  div->clock = val;
  div->period = val;
}

static inline uint8_t APU_divider_tick(APU_divider *div) {
  if (div->clock == 0) {
    div->clock = div->period;
    return 1; // tick
  } else {
    div->clock--;
    return 0; // no tick
  }
}

// reset and dont output a tick
static inline void APU_divider_reload(APU_divider *div) {
  div->clock = div->period;
}

//#############################################################
// ENVELOPE
//#############################################################

void APU_envelope_init(APU_envelope *env, uint8_t reg) { // OK
  // $4000 / $4004	DDLC VVVV	Duty (D), envelope loop / length counter
  // halt (L), constant volume (C), volume/envelope (V)
  env->gain = reg & 0x0F;
  env->constant = (reg & 0x10) == 0x10;
  env->loop = (reg & 0x20) == 0x20; //??
}

void APU_envelope_tick(APU_envelope *env) { // OK
  if (env->start == 0) {
    env->divider--;
    if (env->divider < 0) {
      env->divider = env->gain;
      if (env->decay_gain > 0) {
        env->decay_gain--;
      } else if (env->loop) { //??
        env->decay_gain = 15;
      }
    }
  } else {
    env->start = 0;
    env->decay_gain = 15;
    env->divider = env->gain;
  }
}

uint8_t APU_envelope_get_gain(APU_envelope *env) { // OK
  if (env->constant)
    return env->gain;
  else
    return env->decay_gain;
}

void APU_envelope_reset(APU_envelope *env, uint8_t soft_reset) { // OK
  env->constant = 0;
  env->gain = 0;
  env->envelope_counter = 0;
  env->start = 0;
  env->divider = 0;
  env->decay_gain = 0;
}

//#############################################################
// LENGTH COUNTER
//#############################################################

void APU_length_counter_tick(APU_length_counter *lc) { // OK
  if (!lc->halt && lc->length_counter > 0) {
    lc->length_counter--;
  }
}

void APU_length_counter_set_enabled(APU_length_counter *lc,
                                    uint8_t enabled) { // OK
  if (!enabled) {
    lc->length_counter = 0;
  }
  lc->enabled = enabled;
}

void APU_length_counter_init(APU_length_counter *lc) { // OK
  lc->enabled = 0;
  lc->halt = 0;
  lc->new_halt = 0;
  lc->length_counter = 0;
  lc->prev_length_counter = 0;
  lc->reload_length_counter = 0;
}

void APU_length_counter_reset(APU_length_counter *lc,
                              uint8_t soft_reset) { // OK

  lc->enabled = 0;

  lc->halt = 0;
  lc->length_counter = 0;
  lc->new_halt = 0;
  lc->reload_length_counter = 0;
  lc->prev_length_counter = 0;
}

void APU_length_counter_reset_triangle(APU_length_counter *lc,
                                       uint8_t soft_reset) { // OK
  if (soft_reset) {
    lc->enabled = 0;
  } else {
    lc->enabled = 0;

    lc->halt = 0;
    lc->length_counter = 0;
    lc->new_halt = 0;
    lc->reload_length_counter = 0;
    lc->prev_length_counter = 0;
  }
}

// set apu needs_to_run if lc.enabled
void APU_length_counter_load(APU_length_counter *lc, uint8_t idx) { // OK
  if (lc->enabled) {
    lc->prev_length_counter = lc->length_counter;
    lc->reload_length_counter = Length_counter_lookup[idx];
  }
}

void APU_length_counter_reload(APU_length_counter *lc) { // OK
  if (lc->reload_length_counter) {
    if (lc->length_counter == lc->prev_length_counter) {
      lc->length_counter = lc->reload_length_counter;
    }
    lc->reload_length_counter = 0;
  }
  lc->halt = lc->new_halt;
}

//#############################################################
// SWEEP
//#############################################################

// void APU_sweep_init(APU_sweep *sw, uint8_t reg) {
//  // EPPP NSSS sweep
//  sw->enabled = (reg & 0x80) == 0x80;
//  sw->negate = (reg & 0x08) == 0x08;
//
//  APU_divider_set(&sw->divider, ((reg & 0x70) >> 4) /*+ 1 */);
//  sw->shift_count = (reg & 0x07);
//
//  sw->reload = 1;
//}

//#############################################################
// SQUARE CHANNEL
//#############################################################

void APU_square_channel_update_sweep_target(APU_square_channel *sq) { // OK
  uint16_t shift = sq->real_period >> sq->sweep_shift;
  if (sq->sweep_negate) {
    sq->sweep_target_period = sq->real_period - shift;
    if (sq->sweep_negate_mode == SQR_CHANNEL_1) {
      sq->sweep_target_period--;
    }
  } else {
    sq->sweep_target_period = sq->real_period + shift;
  }
}

void APU_square_channel_sweep_init(APU_square_channel *sq, uint8_t reg) { // OK
  // EPPP NSSS sweep
  sq->sweep_enable = (reg & 0x80) == 0x80;
  sq->sweep_negate = (reg & 0x08) == 0x08;

  sq->sweep_period = ((reg & 0x70) >> 4) + 1;
  sq->sweep_shift = (reg & 0x07);

  APU_square_channel_update_sweep_target(sq);
  sq->sweep_reload = 1;
}

void APU_square_channel_reset(APU_square_channel *sq,
                              uint8_t soft_reset) { // OK
  // timer reset
  sq->timer = 0;
  sq->period = 0;
  sq->last_output = 0;
  sq->prev_cycle = 0;
  sq->real_period = 0;

  // length counter reset
  APU_length_counter_reset(&sq->lc, soft_reset);
  // envelope reset
  APU_envelope_reset(&sq->envelope, soft_reset);

  sq->duty_cycle = 0;
  sq->duty_pos = 0;

  sq->sweep_enable = 0;
  sq->sweep_period = 0;
  sq->sweep_period = 0;
  sq->sweep_negate_mode = 0;
  sq->sweep_shift = 0;
  sq->sweep_reload = 0;
  sq->sweep_target_period = 0;

  APU_square_channel_update_sweep_target(sq);
}

void APU_square_channel_set_period(APU_square_channel *sq,
                                   uint16_t period) { // OK
  sq->real_period = period;
  sq->period = (sq->real_period * 2) + 1;

  APU_square_channel_update_sweep_target(sq);
}

void APU_square_channel_duty(APU_square_channel *sq) { // OK
  sq->duty_pos = (sq->duty_pos - 1) & 0x07;
}

void APU_square_channel_sweep_tick(APU_square_channel *sq) { // OK

  // tick sweeper
  sq->sweep_divider--;
  if (sq->sweep_divider == 0) {
    if (sq->sweep_shift > 0 && sq->sweep_enable && sq->real_period >= 8 &&
        sq->sweep_target_period <= 0x7ff) {
      APU_square_channel_set_period(sq, sq->sweep_target_period);
    }
    sq->sweep_divider = sq->sweep_period;
  }
  if (sq->sweep_reload) {
    sq->sweep_divider = sq->sweep_period;
    sq->sweep_reload = 0;
  }
}

int8_t APU_square_channel_get_output(APU_square_channel *sq) { // OK

  if (sq->real_period < 8 ||
      (!sq->sweep_negate && sq->sweep_target_period > 0x7FF)) // muted
    return 0;
  else {
    uint8_t gain = 0;
    if (sq->lc.length_counter > 0) {
      gain = APU_envelope_get_gain(&sq->envelope);
    }
    return Square_duty_lookup[sq->duty_cycle][sq->duty_pos] * gain;
  }
}

int8_t APU_square_channel_tick(APU_square_channel *sq) { // OK
  APU_square_channel_duty(sq);

  // TODO: output the sample
  int8_t sample = APU_square_channel_get_output(sq);
  int16_t delta = (int8_t)sample - (int8_t)sq->last_output;
  if (delta != 0) {
    blip_add_delta(sq->out, sq->prev_cycle, delta * 1000);
  }

#ifdef DEBUG_AUDIO
  if (sample != 0)
    sq->start_rec = 1; // first nonzero sample starts rec
  if (sq->start_rec) {
    //  fprintf(sq->sqr_f, "%d\n", sample);
  }
#endif

  sq->last_output = sample;
  return sample;
}

void APU_square_channel_run(APU_square_channel *sq,
                            uint32_t target_cycle) { // OK
  int32_t cycles_to_run = target_cycle - sq->prev_cycle;
  while (cycles_to_run > sq->timer) {
    cycles_to_run -= sq->timer + 1;
    sq->prev_cycle += sq->timer + 1;
    APU_square_channel_tick(sq);
    sq->timer = sq->period;
  }
  sq->timer -= cycles_to_run;
  sq->prev_cycle = target_cycle;
}

//#############################################################
// TRIANGLE CHANNEL
//#############################################################

void APU_triangle_channel_reset(APU_triangle_channel *tr, uint8_t soft_reset) {
  // timer reset
  tr->timer = 0;
  tr->period = 0;
  tr->last_output = 0;
  tr->prev_cycle = 0;
  tr->real_period = 0;

  // length counter reset
  APU_length_counter_reset(&tr->lc, soft_reset);

  tr->linear_counter = 0;
  tr->linear_counter_reload = 0;
  tr->linear_counter_reload_flag = 0;
  tr->linear_counter_reload_flag = 0;
  tr->sequence_pos = 0;
}

void APU_triangle_channel_tick_linear_counter(APU_triangle_channel *tr) {
  if (tr->linear_counter_reload_flag) {
    tr->linear_counter = tr->linear_counter_reload;
  } else if (tr->linear_counter > 0) {
    tr->linear_counter--;
  }
  if (!tr->linear_control_flag) {
    tr->linear_counter_reload_flag = 0;
  }
}

void APU_triangle_channel_tick(APU_triangle_channel *tr) {
  uint8_t sample = 0;
  if (tr->lc.length_counter > 0 && tr->linear_counter > 0) {
    // period??
    tr->sequence_pos = (tr->sequence_pos + 1) & 0x1f;
    sample = triangle_sequence[tr->sequence_pos];
  }

  int16_t delta = (int8_t)sample - (int8_t)tr->last_output;
  if (delta != 0) {
    blip_add_delta(tr->out, tr->prev_cycle, delta * 1000);
  }
  tr->last_output = sample;
#ifdef DEBUG_AUDIO
  if (sample != 0)
    tr->start_rec = 1; // first nonzero sample starts rec
  if (tr->start_rec) {
    // fprintf(tr->sqr_f, "%d\n", sample);
  }
#endif
}

void APU_triangle_channel_run(APU_triangle_channel *tr,
                              uint32_t target_cycle) { // OK
  int32_t cycles_to_run = target_cycle - tr->prev_cycle;
  while (cycles_to_run > tr->timer) {
    cycles_to_run -= tr->timer + 1;
    tr->prev_cycle += tr->timer + 1;
    APU_triangle_channel_tick(tr);
    tr->timer = tr->period;
  }
  tr->timer -= cycles_to_run;
  tr->prev_cycle = target_cycle;
}

//#############################################################
// NOISE CHANNEL
//#############################################################

//#############################################################
// Delta Modulation Channel DMC
//#############################################################

//#############################################################
// APU
//#############################################################
// REGISTERS
// $4000-$4003   Sqr1     Timer, length counter, envelope, sweep
// $4004-$4007   Sqr2     Timer, length counter, envelope, sweep
// $4008-$400B   Triangle Timer, length counter, linear counter
// $400C-$400F   Noise    Timer, length counter, envelope, linear feedback shift
// register $4010-$4013	DMC	Timer, memory reader, sample buffer, output unit
// $4015  All Channel enable and length counter status
// $4017  All Frame counter
void APU_cpu_write(APU *apu, uint16_t adr, uint8_t data,
                   uint8_t between_apu_cycle) { // OK
  APU_run(apu);
  switch (adr) {
  // Square ch 1
  case 0x4000: // OK
    // DDLC NNNN
    apu->sqr1.duty_cycle = (data & 0xc0) >> 6;
    // init length counter
    apu->sqr1.lc.new_halt = (data & 0x20) == 0x20;
    apu->needs_to_run = 1;
    APU_envelope_init(&apu->sqr1.envelope, data);
    break;
  case 0x4001: // OK
    APU_square_channel_sweep_init(&apu->sqr1, data);
    break;
  case 0x4002: // OK
    APU_square_channel_set_period(&apu->sqr1,
                                  (apu->sqr1.real_period & 0x0700) | data);
    break;
  case 0x4003: // OK
    APU_length_counter_load(&apu->sqr1.lc, data >> 3);
    if (apu->sqr1.lc.enabled)
      apu->needs_to_run = 1;
    APU_square_channel_set_period(&apu->sqr1, (apu->sqr1.real_period & 0xff) |
                                                  ((data & 0x07) << 8));
    apu->sqr1.duty_pos = 0;       // reset duty pos
    apu->sqr1.envelope.start = 1; // reset envelope
    break;

  // Square ch 2 (Should be the same as above just with sqr1 -> sgr2 )
  case 0x4004: // OK
    apu->sqr2.duty_cycle = (data & 0xc0) >> 6;
    // init length counter
    apu->sqr2.lc.new_halt = (data & 0x20) == 0x20;
    apu->needs_to_run = 1;
    APU_envelope_init(&apu->sqr2.envelope, data);
    break;
  case 0x4005: // OK
    APU_square_channel_sweep_init(&apu->sqr2, data);
    break;
  case 0x4006: // OK
    APU_square_channel_set_period(&apu->sqr2,
                                  (apu->sqr2.real_period & 0x0700) | data);
    break;
  case 0x4007: // OK
    APU_length_counter_load(&apu->sqr2.lc, data >> 3);
    if (apu->sqr2.lc.enabled)
      apu->needs_to_run = 1;
    APU_square_channel_set_period(&apu->sqr2, (apu->sqr2.real_period & 0xff) |
                                                  ((data & 0x07) << 8));
    apu->sqr2.duty_pos = 0;       // reset duty pos
    apu->sqr2.envelope.start = 1; // reset envelope
    break;

  // Triangle ch
  case 0x4008:
    apu->tr.linear_control_flag = (data & 0x80) == 0x80;
    apu->tr.linear_counter_reload = data & 0x7f;
    // init length counter
    apu->sqr1.lc.new_halt = (data & 0x20) == 0x20;
    apu->needs_to_run = 1;
    break;
  case 0x400A:
    apu->tr.period &= ~0x00ff;
    apu->tr.period |= data;
    break;
  case 0x400B:
    APU_length_counter_load(&apu->tr.lc, data >> 3);
    apu->tr.period &= ~0x00ff;
    apu->tr.period |= (data & 0x07) << 8;
    apu->tr.linear_counter_reload_flag = 1;
    break;

  // Noise ch
  case 0x400C: break;
  case 0x400E: break;
  case 0x400F: break;

  // DMC ch
  case 0x4010: break;
  case 0x4011: break;
  case 0x4012: break;
  case 0x4013: break;

  // Control reg
  case 0x4015:             // OK
    apu->DMC_IRQ_flag = 0; // clear DMC interupt flag
    APU_length_counter_set_enabled(&apu->sqr1.lc, (data & 0x01) == 0x01);
    APU_length_counter_set_enabled(&apu->sqr2.lc, (data & 0x02) == 0x02);
    apu->tr.lc.enabled = (data & 0x04) == 0x04;
    // TODO
    // apu->noise.lc.enabled    = (data & 0x08) == 0x08;
    // apu->DMC.lc.enabled      = (data & 0x10) == 0x10;

    break;

  // Frame counter reg
  case 0x4017: // OK
    //  $4017	MI--.----	Set mode and interrupt (write)
    // Bit 7	M--- ----	Sequencer mode: 0 selects 4-step sequence, 1
    // selects 5-step sequence
    // Bit 6	-I-- ----	Interrupt inhibit flag. If set, the frame
    // interrupt flag is cleared, otherwise it is unaffected. Side effects
    // After 3 or 4 CPU clock cycles*, the timer is reset. If the mode flag is
    // set, then both "quarter frame" and "half frame" signals are also
    // generated.
    //* If the write occurs during an APU cycle, the effects occur 3 CPU cycles
    // after the $4017 write cycle, and if the write occurs between APU cycles,
    // the effects occurs 4 CPU cycles after the write cycle.
    apu->sequence_mode_delayed_val = ((data & 0x80) == 0x80) ? 1 : 0;
    if (between_apu_cycle) {
      apu->write_delay_0x4017 = 4;
    } else {
      apu->write_delay_0x4017 = 3;
    }
    apu->IRQ_inhibit_flag = (data & 0x40) == 0x40;
    if (apu->IRQ_inhibit_flag)
      apu->Frame_counter_IRQ_flag = 0;
    break;

  default: break;
  }
}

uint8_t APU_cpu_read(APU *apu, uint16_t adr) { // OK
  // if(adr == 0x4015) only ever called with adr 0x4015
  APU_run(apu);
  // Status reg
  uint8_t status = 0;
  status |= apu->sqr1.lc.length_counter > 0 ? 0x01 : 0x00;
  status |= apu->sqr2.lc.length_counter > 0 ? 0x02 : 0x00;
  status |= apu->tr.lc.length_counter > 0 ? 0x04 : 0x00;
  // TODO
  // status |= apu->noise.lc.length_counter>0  ? 0x08 : 0x00;
  // status |= apu->DMC.lc.length_counter>0  ? 0x10 : 0x00;
  status |= apu->Frame_counter_IRQ_flag ? 0x40 : 0x00;
  status |= apu->DMC_IRQ_flag ? 0x80 : 0x00;

  apu->Frame_counter_IRQ_flag = 0;

  return status;
}

void APU_init(APU *apu) {
#ifdef DEBUG_AUDIO
  apu->sqr1.sqr_f = fopen("sqr1_ch.txt", "w");
  apu->sqr1.start_rec = 0;
  apu->sqr2.sqr_f = fopen("sqr2_ch.txt", "w");
  apu->sqr2.start_rec = 0;
  apu->tr.sqr_f = fopen("tr_ch.txt", "w");
  apu->tr.start_rec = 0;
#endif

  apu->sqr1.out = blip_new(AUDIO_FRAME_SIZE);
  apu->sqr2.out = blip_new(AUDIO_FRAME_SIZE);
  apu->tr.out = blip_new(AUDIO_FRAME_SIZE);
  blip_set_rates(apu->sqr1.out, 1789773, 44100);
  blip_set_rates(apu->sqr2.out, 1789773, 44100);
  blip_set_rates(apu->tr.out, 1789773, 44100);
  apu->sqr1.out_buf = malloc(AUDIO_FRAME_SIZE * sizeof(int16_t));
  apu->sqr2.out_buf = malloc(AUDIO_FRAME_SIZE * sizeof(int16_t));
  apu->tr.out_buf = malloc(AUDIO_FRAME_SIZE * sizeof(int16_t));

  apu->out_buf = malloc(AUDIO_FRAME_SIZE * sizeof(int16_t));

  apu->sqr1.sweep_negate_mode = SQR_CHANNEL_1;
  apu->sqr2.sweep_negate_mode = SQR_CHANNEL_1 + 1;

  apu->sequence_mode = 0;
  apu->current_step = 0;
  apu->write_delay_0x4017 = 0;

#define FINAL_AUDIO_BUF_SIZE 2 << 14
  // apu->final_buf = malloc(FINAL_AUDIO_BUF_SIZE);
  // memset(apu->final_buf,0,FINAL_AUDIO_BUF_SIZE);
  // apu->r_buf=ring_buffer_new(FINAL_AUDIO_BUF_SIZE);
  apu->r_buf = ringbuf_new(FINAL_AUDIO_BUF_SIZE);
  // apu->r_buf=vr_buf_new(3);
  // printf("vr_buf size: %lu\n",apu->r_buf->size);
  apu->fill_audio_buffer = &APU_fill_audio_buffer;
  apu->out_read_pos = 0;
  apu->out_write_pos = 0;
  apu->output_gains[0] = 1.0;
  apu->output_gains[1] = 1.0;
  apu->output_gains[2] = 1.0;
  apu->output_gains[3] = 1.0;
  apu->output_gains[4] = 1.0;
  apu->master_gain = 1.0;
}

void APU_frame_counter_reset(APU *apu, uint8_t soft_reset) {
  apu->previous_frame_cycle = 0;
  apu->current_step = 0;
  if (!soft_reset)
    apu->sequence_mode = 0;

  apu->sequence_mode_delayed_val = apu->sequence_mode ? 0x80 : 0x00;
  apu->write_delay_0x4017 = 3;
  apu->IRQ_inhibit_flag = 0;

  apu->block_frame_counter_tick = 0;
}

static inline void APU_frame_counter_tick__(APU *apu,
                                            uint8_t frame_type) { // OK
  // if (frame_type == SKIP_FRAME)
  //  return;

  APU_envelope_tick(&apu->sqr1.envelope);
  APU_envelope_tick(&apu->sqr2.envelope);
  APU_triangle_channel_tick_linear_counter(&apu->tr);
  // NOISE channel envelope tick

  if (frame_type == HALF_FRAME) {
    APU_length_counter_tick(&apu->sqr1.lc);
    APU_length_counter_tick(&apu->sqr2.lc);
    APU_length_counter_tick(&apu->tr.lc);
    // TODO
    // noise tick len counter

    APU_square_channel_sweep_tick(&apu->sqr1);
    APU_square_channel_sweep_tick(&apu->sqr2);
  }
}

// This is ticked every CPU tick
// APU runs every 2 CPU cycles, This function handles it
uint32_t APU_frame_counter_run(APU *apu, int32_t *cycles_to_run) { // OK
  uint32_t cycles_ran;
  uint8_t mode = apu->sequence_mode;
  uint8_t step = apu->current_step;

  if (apu->previous_frame_cycle + (*cycles_to_run) >=
      Step_cycles_ntsc_table[mode][step]) {

    if (Frame_type_table[mode][step] != SKIP_FRAME &&
        !apu->block_frame_counter_tick) {
      APU_frame_counter_tick__(apu, Frame_type_table[mode][step]);
      apu->block_frame_counter_tick = 2;
    }

    if (apu->current_step >= 3 && apu->sequence_mode == STEP_4_SEQ) {
      if (apu->IRQ_inhibit_flag == 0) {
        apu->Frame_counter_IRQ_flag = 1;
      }
    }

    // if(Step_cycles_XXXX_table[mode][step]<apu->previous_frame_cycle){cycles_ran=0;}else
    cycles_ran = Step_cycles_ntsc_table[mode][step] - apu->previous_frame_cycle;

    (*cycles_to_run) -= cycles_ran;

    apu->current_step++;
    if (apu->current_step == 6) {
      apu->current_step = 0;
      apu->previous_frame_cycle = 0;
    } else {
      apu->previous_frame_cycle += cycles_ran;
    }

  } else {
    cycles_ran = *cycles_to_run;
    *cycles_to_run = 0;
    apu->previous_frame_cycle += cycles_ran;
  }

  if (apu->write_delay_0x4017 > 0) {
    apu->write_delay_0x4017--;
    if (apu->write_delay_0x4017 == 0) {
      apu->sequence_mode = apu->sequence_mode_delayed_val;
      apu->current_step = 0;
      apu->previous_frame_cycle = 0;

      if (apu->sequence_mode == STEP_5_SEQ && !apu->block_frame_counter_tick) {
        //"Writing to $4017 with bit 7 set will immediately generate a clock for
        // both the quarter frame and the half frame units, regardless of what
        // the sequencer is doing."
        APU_frame_counter_tick__(apu, HALF_FRAME);
        apu->block_frame_counter_tick = 2;
      }
    }
  }
  if (apu->block_frame_counter_tick > 0)
    apu->block_frame_counter_tick--;

  return cycles_ran;
}

uint8_t APU_needs_to_run(APU *apu, uint32_t current_cycle) { // OK
  // TODO if(DMC needs to run || apu->needs_to_run){apu->needs_to_run=0; return
  // 1;}
  if (apu->needs_to_run) {
    apu->needs_to_run = 0;
    return 1;
  }
  uint32_t to_run = current_cycle - apu->prev_cycle;

  uint8_t frame_counter_run =
      apu->write_delay_0x4017 > 0 || apu->block_frame_counter_tick > 0 ||
      (apu->previous_frame_cycle + (int32_t)to_run >=
       Step_cycles_ntsc_table[apu->sequence_mode][apu->current_step] - 1);

  return frame_counter_run; // || DMC_IRQ_pending(apu->DMC, to_run);
}

void APU_run(APU *apu) { // OK
  int32_t cycles_to_run = apu->cycle - apu->prev_cycle;
  while (cycles_to_run > 0) {
    apu->prev_cycle += APU_frame_counter_run(apu, &cycles_to_run);

    APU_length_counter_reload(&apu->sqr1.lc);
    APU_length_counter_reload(&apu->sqr2.lc);
    APU_length_counter_reload(&apu->tr.lc);
    // TODO + noise, triangle, dmc

    APU_square_channel_run(&apu->sqr1, apu->prev_cycle);
    APU_square_channel_run(&apu->sqr2, apu->prev_cycle);
    APU_triangle_channel_run(&apu->tr, apu->prev_cycle);
    // + noise,triangle,dmc
  }
}

// CLOCK_INIT(into)

void APU_tick(APU *apu) { // OK

  apu->cycle++;
  if (apu->cycle == AUDIO_FRAME_SIZE - 1) {
    APU_run(apu);
    // end frame
    apu->sqr1.prev_cycle = 0;
    apu->sqr2.prev_cycle = 0;
    apu->tr.prev_cycle = 0;
    // TODO noise dmc

    blip_end_frame(apu->sqr1.out, apu->cycle);
    size_t sample_count = blip_read_samples(apu->sqr1.out, apu->sqr1.out_buf,
                                            AUDIO_FRAME_SIZE, 0);

    blip_end_frame(apu->sqr2.out, apu->cycle);
    sample_count = blip_read_samples(apu->sqr2.out, apu->sqr2.out_buf,
                                     AUDIO_FRAME_SIZE, 0);

    blip_end_frame(apu->tr.out, apu->cycle);
    sample_count =
        blip_read_samples(apu->tr.out, apu->tr.out_buf, AUDIO_FRAME_SIZE, 0);

    for (size_t i = 0; i < sample_count; ++i) {
      apu->out_buf[i] = (int16_t)(
          apu->master_gain * ((apu->sqr1.out_buf[i] * apu->output_gains[0]) +
                              (apu->sqr2.out_buf[i] * apu->output_gains[1]) +
                              (apu->tr.out_buf[i] * apu->output_gains[2])));
    }

    // CLOCK_START(into);
    // ring_buffer_memcpy_to(apu->r_buf, apu->out_buf,2*sample_count);
    ringbuf_memcpy_into(apu->r_buf, apu->out_buf, 2 * sample_count);
    // vr_buf_memcpy_to(apu->r_buf, apu->out_buf,2*sample_count);
    // CLOCK_END(into);
    // printf("free: %lu / %lu\n",apu->r_buf->free_bytes,FINAL_AUDIO_BUF_SIZE);

    apu->cycle = 0;
    apu->prev_cycle = 0;

  } else if (APU_needs_to_run(apu, apu->cycle)) {
    APU_run(apu);
  }
}

/*
void APU_end_frame(APU *apu) {
  APU_run(apu);
  apu->sqr1.prev_cycle = 0;
  apu->sqr2.prev_cycle = 0;
  apu->tr.prev_cycle = 0;

  apu->cycle = 0;
  apu->prev_cycle = 0;
} */

void APU_reset(APU *apu, uint8_t soft_reset) {
  apu->cycle = 0;
  apu->prev_cycle = 0;

  APU_square_channel_reset(&apu->sqr1, soft_reset);
  APU_square_channel_reset(&apu->sqr2, soft_reset);
  APU_triangle_channel_reset(&apu->tr, soft_reset);

  APU_frame_counter_reset(apu, soft_reset);

  blip_clear(apu->sqr1.out);
  blip_clear(apu->sqr2.out);
}

// CLOCK_INIT(from)

void APU_fill_audio_buffer(void *apu_, uint8_t *stream, int len /*in bytes*/) {
  APU *apu = (APU *)apu_;

  // CLOCK_START(from);
  // ring_buffer_memcpy_n_from(apu->r_buf,stream, len);
  ringbuf_memcpy_from(stream, apu->r_buf, len);
  // vr_buf_memcpy_from(stream, apu->r_buf, len);
  // CLOCK_END(from);
}

void APU_free(APU *apu) {
  blip_delete(apu->sqr1.out);
  blip_delete(apu->sqr2.out);
  blip_delete(apu->tr.out);

  free(apu->sqr1.out_buf);
  free(apu->sqr2.out_buf);
  free(apu->tr.out_buf);

  free(apu->out_buf);

  // ring_buffer_free(apu->r_buf);
  ringbuf_free(&apu->r_buf);
  // vr_buf_free(apu->r_buf);

  // printf("\nfrom: avr: %lf  full: %lf   n:%lu\n",
  //  CLOCK_RESULT_AVERAGE_uS(from),
  //  CLOCK_RESULT_FULL_TIME_uS(from),
  //  CLOCK_RESULT_N_MEAS(from)
  //  );
  //
  // printf("\ninto: avr: %lf  full: %lf   n:%lu\n",
  //  CLOCK_RESULT_AVERAGE_uS(into),
  //  CLOCK_RESULT_FULL_TIME_uS(into),
  //  CLOCK_RESULT_N_MEAS(into)
  //  );
}

void APU_set_dB_master(APU *apu, float dB) {
  if (apu->master_dB == dB)
    return;
  apu->master_dB = dB;
  apu->master_gain = dB_to_gain(dB);
}
void APU_set_dB_sqr1(APU *apu, float dB) {
  if (apu->output_dBs[0] == dB)
    return;
  apu->output_dBs[0] = dB;
  apu->output_gains[0] = dB_to_gain(dB);
}
void APU_set_dB_sqr2(APU *apu, float dB) {
  if (apu->output_dBs[1] == dB)
    return;
  apu->output_dBs[1] = dB;
  apu->output_gains[1] = dB_to_gain(dB);
}
void APU_set_dB_tr(APU *apu, float dB) {
  if (apu->output_dBs[2] == dB)
    return;
  apu->output_dBs[2] = dB;
  apu->output_gains[2] = dB_to_gain(dB);
}
void APU_set_dB_noise(APU *apu, float dB) {
  if (apu->output_dBs[3] == dB)
    return;
  apu->output_dBs[3] = dB;
  apu->output_gains[3] = dB_to_gain(dB);
}
void APU_set_dB_dmc(APU *apu, float dB) {
  if (apu->output_dBs[4] == dB)
    return;
  apu->output_dBs[4] = dB;
  apu->output_gains[4] = dB_to_gain(dB);
}