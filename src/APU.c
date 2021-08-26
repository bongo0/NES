#include "APU.h"

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

void APU_length_counter_load(APU_length_counter *lc, uint8_t idx) {
  if (lc->enabled) {
    lc->prev_length_counter = lc->length_counter;
    lc->reload_length_counter = Length_counter_lookup[idx];
  }
}

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
  // EPPP NSSS sweep
  sw->enabled = (reg & 0x80) == 0x80;
  sw->negate = (reg & 0x08) == 0x08;

  APU_divider_set(&sw->divider, ((reg & 0x70) >> 4) + 1);
  sw->shift_count = (reg & 0x07);

  sw->reload = 1;
}

//#############################################################
// SQUARE CHANNEL
//#############################################################

void APU_square_channel_reset(APU_square_channel *sq){
  //TODO
  sq->duty_cycle=0;
  sq->duty_pos=0;

  sq->real_period=0;

  sq->sweep.enabled=0;

  sq->sweep_negate_mode=0;
  
  //APU_divider_set(&sq->sweep,0);
  //APU_sweep_init(&sq->sweep,0);
  sq->sweep_target_period=0;
}

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
  sq->duty_pos = (sq->duty_pos - 1) & 0x07;// test
  if (sq->real_period < 8 ||
      (!sq->sweep.negate && sq->sweep_target_period > 0x7FF)) // muted
    return 0;
  else{
    uint8_t gain=0;
    if(sq->lc.length_counter>0){
        gain = APU_envelope_get_gain(&sq->envelope);
    }
    return Square_duty_lookup[sq->duty_cycle][sq->duty_pos] * sq->envelope.gain;
  
  }
}

void APU_square_channel_tick_envelope(APU_square_channel *sq) {
  APU_envelope_tick(&sq->envelope);
}

void APU_square_channel_tick_len_counter(APU_square_channel *sq) {
  APU_length_counter_tick(&sq->lc);
}

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
void APU_cpu_write(APU *apu, uint16_t adr, uint8_t data, uint8_t during_apu_cycle) {
  switch (adr) {
  // Square ch 1
  case 0x4000:
    // DDLC NNNN
    apu->sqr1.duty_cycle = data >> 6;
    apu->sqr1.lc.new_halt = (data & 0x20) == 0x20;
    apu->sqr1.envelope.constant = (data & 0x10) == 0x10;
    apu->sqr1.envelope.gain = data & 0x0F;
    break;
  case 0x4001: APU_sweep_init(&apu->sqr1.sweep, data); break;
  case 0x4002:
    APU_square_channel_set_period(&apu->sqr1,
                                  (apu->sqr1.real_period & 0x0700) | data);
    break;
  case 0x4003:
    APU_length_counter_load(&apu->sqr1.lc, data >> 3);
    apu->sqr1.duty_pos = 0;       // reset duty pos
    apu->sqr1.envelope.start = 1; // reset envelope
    break;

  // Square ch 2 (Should be the same as above just with sqr1 -> sgr2 )
  case 0x4004:
    apu->sqr2.duty_cycle = data >> 6;
    apu->sqr2.lc.new_halt = (data & 0x20) == 0x20;
    apu->sqr2.envelope.constant = (data & 0x10) == 0x10;
    apu->sqr2.envelope.gain = data & 0x0F;
    break;
  case 0x4005: APU_sweep_init(&apu->sqr1.sweep, data); break;
  case 0x4006:
    APU_square_channel_set_period(&apu->sqr2,
                                  (apu->sqr2.real_period & 0x0700) | data);
    break;
  case 0x4007:
    APU_length_counter_load(&apu->sqr2.lc, data >> 3);
    apu->sqr2.duty_pos = 0;       // reset duty pos
    apu->sqr2.envelope.start = 1; // reset envelope
    break;

  // Triangle ch
  case 0x4008: break;
  case 0x400A: break;
  case 0x400B: break;

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
  case 0x4015:
    apu->DMC_IRQ_flag = 0; // clear DMC interupt flag
    apu->sqr1.lc.enabled = (data & 0x01) == 0x01;
    apu->sqr2.lc.enabled = (data & 0x02) == 0x02;
    // TODO
    // apu->triangle.lc.enabled = (data & 0x04) == 0x04;
    // apu->noise.lc.enabled    = (data & 0x08) == 0x08;
    // apu->DMC.lc.enabled      = (data & 0x10) == 0x10;

    break;

  // Frame counter reg
  case 0x4017:
    //  $4017	MI--.----	Set mode and interrupt (write)
    // Bit 7	M--- ----	Sequencer mode: 0 selects 4-step sequence, 1 selects
    // 5-step sequence
    // Bit 6	-I-- ----	Interrupt inhibit flag. If set, the frame interrupt flag
    // is cleared, otherwise it is unaffected. Side effects	After 3 or 4 CPU
    // clock cycles*, the timer is reset. If the mode flag is set, then both
    // "quarter frame" and "half frame" signals are also generated.
    //* If the write occurs during an APU cycle, the effects occur 3 CPU cycles
    //after the $4017 write cycle, and if the write occurs between APU cycles,
    //the effects occurs 4 CPU cycles after the write cycle.
    apu->sequence_mode_delayed_val = ((data & 0x80) == 0x80) ? 1 : 0;
    apu->new_write=1;
    if(during_apu_cycle){
      apu->write_delay_0x4017 = 3;
    }else{
      apu->write_delay_0x4017 = 4;
    }
    apu->IRQ_inhibit_flag = (data & 0x40) == 0x40;
    if(apu->IRQ_inhibit_flag)apu->Frame_counter_IRQ_flag=0;
    break;

  default: break;
  }
}

uint8_t APU_cpu_read(APU *apu, uint16_t adr) {
  // if(adr == 0x4015) only ever called with adr 0x4015
  // Status reg
  uint8_t status = 0;
  status |= apu->sqr1.lc.length_counter > 0 ? 0x01 : 0x00;
  status |= apu->sqr2.lc.length_counter > 0 ? 0x02 : 0x00;
  // TODO
  // status |= apu->triangle.lc.length_counter>0  ? 0x04 : 0x00;
  // status |= apu->noise.lc.length_counter>0  ? 0x08 : 0x00;
  // status |= apu->DMC.lc.length_counter>0  ? 0x10 : 0x00;
  status |= apu->Frame_counter_IRQ_flag ? 0x40 : 0x00;
  status |= apu->DMC_IRQ_flag ? 0x80 : 0x00;

  return status;
}

void APU_init(APU *apu) {
#ifdef DEBUG_AUDIO
  apu->sqr1_f=fopen("sqr1_ch.txt","w");
  apu->start_sqr1_rec=0;
  apu->sqr2_f=fopen("sqr2_ch.txt","w");
  apu->start_sqr2_rec=0;
#endif

  apu->sqr1.sweep_negate_mode = SQR_CHANNEL_1;
  apu->sqr2.sweep_negate_mode = SQR_CHANNEL_1 + 1;

  apu->sequence_mode = 0;
  apu->sequence_step = 0;
  apu->write_delay_0x4017 = 0;
}

void APU_reset(APU *apu) {
  
}

static inline void APU_frame_counter_tick__(APU *apu, uint8_t frame_type) {
  if(frame_type==SKIP_FRAME)return;

  APU_square_channel_tick_envelope(&apu->sqr1);
  APU_square_channel_tick_envelope(&apu->sqr2);
  // TODO triangle tick_linear_counter
  // NOISE channel envelope tick

  if (frame_type==HALF_FRAME) {
    APU_square_channel_tick_len_counter(&apu->sqr1);
    APU_square_channel_tick_len_counter(&apu->sqr2);
    //TODO triangle tick len counter
    // noise tick len counter

    APU_square_channel_sweep_tick(&apu->sqr1);
    APU_square_channel_sweep_tick(&apu->sqr2);
  }
}

// This is ticked every CPU tick
// APU runs every 2 CPU cycles, This function handles it
void APU_frame_counter_tick(APU *apu) {

if(apu->cycle>=Step_cycles_ntsc_table[apu->sequence_mode][5])apu->cycle=0;

  if (apu->cycle == Step_cycles_ntsc_table[apu->sequence_mode][apu->sequence_step]) {
     APU_frame_counter_tick__(apu,Frame_type_table[apu->sequence_mode][apu->sequence_step]);
     if(apu->sequence_step>=3 && apu->sequence_mode == STEP_4_SEQ){
       if (apu->IRQ_inhibit_flag != 0) {
         apu->Frame_counter_IRQ_flag = 1;
       }
     }
     apu->sequence_step++;
     if(apu->sequence_step == 6){
       apu->sequence_step=0;
       apu->cycle = 0;
     }
  }

  if(apu->write_delay_0x4017>0){
    apu->write_delay_0x4017--;
    if(apu->write_delay_0x4017==0){
      apu->sequence_mode=apu->sequence_mode_delayed_val;
      apu->cycle=0;

      if(apu->sequence_mode==STEP_5_SEQ /*&& !apu->Block_frame_counter_tick*/){
        //"Writing to $4017 with bit 7 set will immediately generate a clock for both the quarter frame and the half frame units, regardless of what the sequencer is doing."
        APU_frame_counter_tick__(apu, HALF_FRAME);
        // TODO
        // apu->Block_frame_counter_tick = 2;
      }
    }
  }


//if(apu->cycle % (1789773/48000)==0){/*1.789773MHz NTSC*/
  uint8_t sqr1_sample = APU_square_channel_get_output(&apu->sqr1);
  uint8_t sqr2_sample = APU_square_channel_get_output(&apu->sqr2);
#ifdef DEBUG_AUDIO
if(sqr1_sample!=0)apu->start_sqr1_rec=1;// first nonzero sample starts rec
if(apu->start_sqr1_rec)fprintf(apu->sqr1_f,"%d\n",sqr1_sample);

if(sqr2_sample!=0)apu->start_sqr2_rec=1;// first nonzero sample starts rec
if(apu->start_sqr2_rec)fprintf(apu->sqr2_f,"%d\n",sqr2_sample);
#endif
//}

  apu->cycle++;
}

void APU_tick(APU *apu){
  APU_frame_counter_tick(apu);
}

void APU_end_frame(APU *apu) {}

void APU_fill_audio_buffer(void *apu_, uint8_t *stream, int len) {
  APU *apu = (APU *)apu_;
}