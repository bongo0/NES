#include "mappers.h"
#include <stdio.h>
// ########################################################
//    MAPPER 000
// ########################################################

uint8_t Mapper000_cpu_read(void *mapper_state, uint16_t adr,
                           uint16_t *mapped_adr, uint8_t *data) {
  // if  PRG_size is 16KB
  //     CPU address Bus          PRG ROM
  //     0x8000 -> 0xBFFF: Map    0x0000 -> 0x3FFF
  //     0xC000 -> 0xFFFF: Mirror 0x0000 -> 0x3FFF
  // else if 32KB
  //     CPU address Bus          PRG ROM
  //     0x8000 -> 0xFFFF: Map    0x0000 -> 0x7FFF
  Mapper000 *mapper = (Mapper000 *)mapper_state;
  if (adr >= 0x8000) {
    (*mapped_adr) = adr & (mapper->PRG_size > (1 << 14) ? 0x7FFF : 0x3FFF);
    return MAP_TRUE;
  }
  return MAP_FALSE;
}

uint8_t Mapper000_cpu_write(void *mapper_state, uint16_t adr,
                            uint16_t *mapped_adr, uint8_t *data) {
  Mapper000 *mapper = (Mapper000 *)mapper_state;
  if (adr >= 0x8000) {
    (*mapped_adr) = adr & (mapper->PRG_size > (1 << 14) ? 0x7FFF : 0x3FFF);
    return MAP_TRUE;
  }
  return MAP_FALSE;
}

uint8_t Mapper000_ppu_read(void *mapper_state, uint16_t adr,
                           uint16_t *mapped_adr, uint8_t *data) {
  // no mapping needed
  if (adr <= 0x1fff) {
    (*mapped_adr) = adr;
    return MAP_TRUE;
  }
  return MAP_FALSE;
}

uint8_t Mapper000_ppu_write(void *mapper_state, uint16_t adr,
                            uint16_t *mapped_adr, uint8_t *data) {
  if (adr <= 0x1fff) {
    Mapper000 *mapper = (Mapper000 *)mapper_state;
    if (mapper->CHR_size == 0) {
      // Using CHR RAM
      (*mapped_adr) = adr;
      return MAP_TRUE;
    }
  }
  return MAP_FALSE;
}
NES_MIRRORING_TYPE Mapper000_mirror(void *mapper_state){
  Mapper000 *state = (Mapper000 *)mapper_state;
  return state->mirror_mode;
}
void Mapper000_reset(void *mapper_state) {}

// ########################################################
//    MAPPER 001 aka MMC1
// ########################################################
uint8_t Mapper001_cpu_read(void *mapper_state, uint16_t adr,
                           uint16_t *mapped_adr, uint8_t *data) {
  Mapper001 *state = (Mapper001 *)mapper_state;
  if (adr >= 0x6000 && adr <= 0x7fff) {
    // reading from ROMs RAM
    (*data) = state->RAM[adr & 0x1fff];
    return MAP_TRUE | MAP_RAM;
  }

  if (adr >= 0x8000) {
    if (state->control_register & 0x8) { // 16K
      if (adr >= 0x8000 && adr <= 0xbfff) {
        (*mapped_adr) = state->PRG_bank_select16low * 0x4000 + (adr & 0x3fff);
        return MAP_TRUE;
      }
      if (adr >= 0xC000) {
        (*mapped_adr) = state->PRG_bank_select16high * 0x4000 + (adr & 0x3FFF);
        return MAP_TRUE;
      }
    } else {
      // 32K
      (*mapped_adr) = state->PRG_bank_select32 * 0x8000 + (adr & 0x7fff);
      return MAP_TRUE;
    }
  }

  return MAP_FALSE;
}

uint8_t Mapper001_cpu_write(void *mapper_state, uint16_t adr,
                            uint16_t *mapped_adr, uint8_t *data) {
  Mapper001 *state = (Mapper001 *)mapper_state;
  if (adr >= 0x6000 && adr <= 0x7FFF) {

    state->RAM[adr & 0x1FFF] = (*data);

    return MAP_TRUE | MAP_RAM;
  }

  if (adr >= 0x8000) {
    if ((*data) & 0x80) {
      // MSB is set, so reset serial loading
      state->load_register = 0x00;
      state->load_register_count = 0;
      state->control_register = state->control_register | 0x0C;
    } else {
      // Load data in serially into load register
      // It arrives LSB first, so implant this at
      // bit 5. After 5 writes, the register is ready
      state->load_register >>= 1;
      state->load_register |= ((*data) & 0x01) << 4;
      state->load_register_count++;

      if (state->load_register_count == 5) {
        // Get Mapper Target Register, by examining
        // bits 13 & 14 of the address
        uint8_t target_register = (adr >> 13) & 0x03;

        if (target_register == 0) // 0x8000 - 0x9FFF
        {
          // Set Control Register
          state->control_register = state->load_register & 0x1F;

          switch (state->control_register & 0x03) {
          case 0: state->mirror_mode = MIRROR_ONESCREEN_LOW; break;
          case 1: state->mirror_mode = MIRROR_ONESCREEN_HIGH; break;
          case 2: state->mirror_mode = MIRROR_VERTICAL; break;
          case 3: state->mirror_mode = MIRROR_HORIZONTAL; break;
          }
        } else if (target_register == 1) // 0xA000 - 0xBFFF
        {
          // Set CHR Bank Lo
          if (state->control_register & 0x10) {
            // 4K CHR Bank at PPU 0x0000
            state->CHR_bank_select_4low = state->load_register & 0x1F;
          } else {
            // 8K CHR Bank at PPU 0x0000
            state->CHR_bank_select8 = state->load_register & 0x1E;
          }
        } else if (target_register == 2) // 0xC000 - 0xDFFF
        {
          // Set CHR Bank Hi
          if (state->control_register & 0x10) {
            // 4K CHR Bank at PPU 0x1000
            state->CHR_bank_select_4high = state->load_register & 0x1F;
          }
        } else if (target_register == 3) // 0xE000 - 0xFFFF
        {
          // Configure PRG Banks
          uint8_t nPRGMode = (state->control_register >> 2) & 0x03;

          if (nPRGMode == 0 || nPRGMode == 1) {
            // Set 32K PRG Bank at CPU 0x8000
            state->PRG_bank_select32 = (state->load_register & 0x0E) >> 1;
          } else if (nPRGMode == 2) {
            // Fix 16KB PRG Bank at CPU 0x8000 to First Bank
            state->PRG_bank_select16low = 0;
            // Set 16KB PRG Bank at CPU 0xC000
            state->PRG_bank_select16high = state->load_register & 0x0F;
          } else if (nPRGMode == 3) {
            // Set 16KB PRG Bank at CPU 0x8000
            state->PRG_bank_select16low = state->load_register & 0x0F;
            // Fix 16KB PRG Bank at CPU 0xC000 to Last Bank
            state->PRG_bank_select16high = state->PRG_size / PRG_BANK_SIZE - 1;
          }
        }

        // 5 bits were written, and decoded, so
        // reset load register
        state->load_register = 0x00;
        state->load_register_count = 0;
      }
    }
  }

  return MAP_FALSE;
}

uint8_t Mapper001_ppu_read(void *mapper_state, uint16_t adr,
                           uint16_t *mapped_adr, uint8_t *data) {
  Mapper001 *state = (Mapper001 *)mapper_state;

  if (adr < 0x2000) {
    if (state->CHR_size == 0) {
      (*mapped_adr) = adr;
      return MAP_TRUE;
    } else {
      if (state->control_register & 0x10) {
        // 4K CHR Bank Mode
        if (adr <= 0x0FFF) {
          (*mapped_adr) = state->CHR_bank_select_4low * 0x1000 + (adr & 0x0FFF);
          return MAP_TRUE;
        }

        if (adr >= 0x1000 && adr <= 0x1FFF) {
          (*mapped_adr) =
              state->CHR_bank_select_4high * 0x1000 + (adr & 0x0FFF);
          return MAP_TRUE;
        }
      } else {
        // 8K CHR Bank Mode
        (*mapped_adr) = state->CHR_bank_select8 * 0x2000 + (adr & 0x1FFF);
        return MAP_TRUE;
      }
    }
  }

  return MAP_FALSE;
}

uint8_t Mapper001_ppu_write(void *mapper_state, uint16_t adr,
                            uint16_t *mapped_adr, uint8_t *data) {
  Mapper001 *state = (Mapper001 *)mapper_state;

if (adr < 0x2000)
	{
		if (state->CHR_size == 0)
		{
			(*mapped_adr) = adr;
			return MAP_TRUE;
		}

		return MAP_TRUE;
	}


  return MAP_FALSE;
}

NES_MIRRORING_TYPE Mapper001_mirror(void *mapper_state){
  Mapper001 *state = (Mapper001 *)mapper_state;
  return state->mirror_mode;
}

void Mapper001_reset(void *mapper_state) {
  Mapper001 *state = (Mapper001 *)mapper_state;
  state->control_register = 0x1C;
  state->load_register = 0;
  state->load_register_count = 0;

  state->CHR_bank_select_4low = 0;
  state->CHR_bank_select_4high = 0;
  state->CHR_bank_select8 = 0;

  state->PRG_bank_select32 = 0;
  state->PRG_bank_select16low = 0;
  state->PRG_bank_select16high = state->PRG_size / PRG_BANK_SIZE - 1;
}