#include "APU.h"

//#############################################################
// DIVIDER
//#############################################################

void APU_divider_set(APU_divider *div, uint8_t val) {
  div->clock = val;
  div->period = val;
}

uint8_t APU_divider_tick(APU_divider *div) {
  if (div->clock == 0) {
    div->clock = div->period;
    return 1; // tick
  } else {
    div->clock--;
    return 0; // no tick
  }
}

void APU_divider_reload(APU_divider *div) { div->clock = div->period; }

//#############################################################
// ENVELOPE
//#############################################################

void APU_envelope_init(APU_envelope *env, uint8_t reg) {
  // $4000 / $4004	DDLC VVVV	Duty (D), envelope loop / length counter
  // halt (L), constant volume (C), volume/envelope (V)
  env->gain = reg & 0x0F;
  env->constant = (reg & 0x10) == 0x10;
  env->start = 1; // maybe not right??
}

void APU_envelope_tick(APU_envelope *env) {
  if (env->start == 0) {
    if (APU_divider_tick(&env->divider)) {
      APU_divider_set(&env->divider, env->gain);
      if (env->decay_gain > 0) {
        env->decay_gain--;
      } else if (env->loop) {
        env->decay_gain = 15;
      }
    }
  } else {
    env->start = 0;
    env->decay_gain = 15;
    APU_divider_set(&env->divider, env->gain);
  }
}

uint8_t APU_envelope_get_gain(APU_envelope *env) {
  if (env->constant)
    return env->gain;
  else
    return env->decay_gain;
}


//#############################################################
// LENGTH COUNTER
//#############################################################

void APU_length_counter_tick(APU_length_counter *lc) {
  if (lc->halt || lc->length_counter == 0) {

  } else {
    lc->length_counter--;
  }
}

void APU_length_counter_set_enabled(APU_length_counter *lc, uint8_t enabled) {
  if (!enabled) {
    lc->enabled = 0;
    lc->length_counter = 0;
  } else {
    lc->enabled = 1;
  }
}

void APU_length_counter_init(APU_length_counter *lc) {
  lc->enabled = 0;
  lc->halt = 0;
  lc->new_halt = 0;
  lc->length_counter = 0;
  lc->prev_length_counter = 0;
  lc->reload_length_counter = 0;
}

void APU_length_counter_reset(APU_length_counter *lc) { lc->enabled = 0; }

void APU_length_counter_reload(APU_length_counter *lc) {
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
void APU_sweep_init(APU_sweep *sw, uint8_t reg) {
  sw->enabled = (reg & 0x80) == 0x80;
  sw->negate = (reg & 0x08) == 0x08;

  APU_divider_set(&sw->divider, ((reg & 0x70) >> 4) + 1);
  sw->shift_count = (reg & 0x07);

  sw->reload = 1;
}


//#############################################################
// SQUARE CHANNEL
//#############################################################

void APU_square_channel_update_sweep(APU_square_channel *sq) {
  uint16_t res = sq->real_period >> sq->sweep.shift_count;
  if (sq->sweep.negate) {
    sq->sweep_target_period = sq->real_period - res;
    if (sq->sweep_negate_mode == SQR_CHANNEL_1) {
      sq->sweep_target_period--;
    }
  } else {
    sq->sweep_target_period = sq->real_period + res;
  }
}

void APU_square_channel_set_period(APU_square_channel *sq, uint16_t period) {
  sq->real_period = period;
  sq->period = sq->real_period * 2 + 1;

  APU_square_channel_update_sweep(sq);
}

void APU_square_channel_duty(APU_square_channel *sq) {
  sq->duty_pos = (sq->duty_pos - 1) & 0x07;
}

void APU_square_channel_sweep_tick(APU_square_channel *sq) {

  // tick sweeper
  if (APU_divider_tick(&sq->sweep.divider)) {
    if (sq->sweep.shift_count > 0 && sq->sweep.enabled &&
        sq->real_period >= 8 && sq->sweep_target_period <= 0x7fff) {
      APU_square_channel_set_period(sq, sq->sweep_target_period);
    }
  }
  if (sq->sweep.reload) {
    APU_divider_reload(&(sq->sweep.divider));
    sq->sweep.reload = 0;
  }
}

uint8_t APU_square_channel_get_output(APU_square_channel *sq) {
  if (sq->real_period < 8 ||
      (!sq->sweep.negate && sq->sweep_target_period > 0x7FF)) // muted
    return 0;
  else
    return Square_duty_lookup[sq->duty_cycle][sq->duty_pos] * sq->envelope.gain;
}

void APU_square_channel_tick_envelope(APU_square_channel *sq){

}

void APU_square_channel_tick_len_counter(APU_square_channel *sq){

}







//#############################################################
// APU
//#############################################################

void APU_init(APU *apu){
    apu->sqr1.sweep_negate_mode=SQR_CHANNEL_1;
    apu->sqr2.sweep_negate_mode=SQR_CHANNEL_1+1;



}

void APU_reset(APU *apu){

}

void APU_frame_counter_tick(APU *apu, uint8_t half_frame){
  APU_square_channel_tick_envelope(&apu->sqr1);
  APU_square_channel_tick_envelope(&apu->sqr2);

  if(half_frame){
      APU_square_channel_tick_len_counter(&apu->sqr1);
      APU_square_channel_tick_len_counter(&apu->sqr2);
      
      
      APU_square_channel_sweep_tick(&apu->sqr1);
      APU_square_channel_sweep_tick(&apu->sqr2);

  }
}

void APU_end_frame(APU * apu){

}