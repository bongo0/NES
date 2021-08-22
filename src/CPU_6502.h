#ifndef CPU_6502_H
#define CPU_6502_H

#include "ROM.h"

//#include "BUS.h"
typedef struct bus NES_BUS;

#include <stdint.h>
#include <stdio.h>

//#define NESTEST_LOG_COMP

#ifdef NESTEST_LOG_COMP 
FILE *mycpu_log;
#endif
// processor status register
// Status register flags
//    7  bit  0
//    ---- ----
//    NVss DIZC
//    |||| ||||
//    |||| |||+- Carry
//    |||| ||+-- Zero
//    |||| |+--- Interrupt Disable
//    |||| +---- Decimal
//    ||++------ No CPU effect, see: the B flag
//    |+-------- Overflow
//    +--------- Negative
#define CPU_STATUS_CARRY (1 << 0)
#define CPU_STATUS_ZERO (1 << 1)
#define CPU_STATUS_INTERUPT_DISABLE (1 << 2)
#define CPU_STATUS_DECIMAL (1 << 3)
#define CPU_STATUS_BREAK (1 << 4)
#define CPU_STATUS_RESERVED (1 << 5)
#define CPU_STATUS_OVERFLOW (1 << 6)
#define CPU_STATUS_NEGATIVE (1 << 7)

#define CPU_MEM_ADDR_SIZE 0x10000

#define CPU_VIDEO_RAM_SIZE 0x4000

#define CPU_NMI_VECTOR 0xFFFA
#define CPU_RESET_VECTOR 0xFFFC
#define CPU_IRQ_VECTOR 0xFFFE

#define CPU_CLOCK_NTSC 1789773
#define CPU_CLOCK_PAL 1662607
#define CPU_CLOCK_DENDY 1773448

typedef struct {
  // CPU registers                     = at power-up
  uint8_t A;   // accumulator          = 0
  uint8_t X;   // X register           = 0
  uint8_t Y;   // Y register           = 0
  uint16_t PC; // program counter      = 0
  uint8_t SP;  // stack pointer        = 0xFD
  uint8_t P;   // processor status reg = 0x34

  uint64_t cycle_count;
  uint16_t operand;
  uint8_t page_cross; // flag for when page is crossed
  uint8_t branch_taken;

//#ifdef NESTEST_LOG_COMP
  uint8_t raw_operand_ind; // for nestest.log testing
  uint8_t raw_operand_abs_low; // for nestest.log testing
  uint8_t raw_operand_abs_high; // for nestest.log testing
  uint8_t raw_operand_zp; // for nestest.log testing
//#endif
  uint64_t cycles_accumulated;
  uint8_t op;
  uint8_t last_op;

  uint8_t need_nmi;
  uint8_t last_need_nmi;
} CPU_state;

// maybe make this opaque type
typedef struct {
  CPU_state state;
  CPU_state previous_state;// for logging
  //uint8_t ram[CPU_RAM_SIZE];
  NES_BUS *bus; // pointer to the connected bus
} CPU_6502;

typedef enum {
  IMMIDIATE,
  ZEROPAGE,
  ZEROPAGE_X,
  ZEROPAGE_Y,
  ABSOLUTE,
  ABSOLUTE_X,
  ABSOLUTE_Y,
  INDIRECT_X,
  INDIRECT_Y,
  RELATIVE,
  IMPLICIT,
  NONE,
  INDIRECT,
  ACCUMULATE,
  // what are these???
  AbsYW,
  AbsXW,
  IndYW,

} CPU_addr_mode;

// stack location ($0100-$01FF)
#define STACK_LOCATION 0x100

typedef void *(*opcode_func_t)(CPU_6502 *, CPU_addr_mode);

CPU_6502 *CPU_init(CPU_6502 *cpu, NES_BUS *bus);
void CPU_load_rom(CPU_6502 *cpu, NES_ROM *rom);
void CPU_reset(CPU_6502 *cpu);
void CPU_print_state_(CPU_6502 *cpu, FILE *fd);
void CPU_log_state_simple(CPU_6502 *cpu, FILE *fd, uint16_t last_pc,
                          uint8_t last_op);



// things that probably does not need to be public
uint8_t CPU_tick(CPU_6502 *CPU);
void CPU_exec_instruction(CPU_6502 *CPU, uint8_t op_code);
void CPU_load_to_memory(CPU_6502 *cpu, uint8_t *data, uint16_t size, uint16_t adr);
uint8_t CPU_get_status_flag(CPU_6502 *cpu, uint8_t flag);
CPU_state CPU_get_state(CPU_6502 *CPU);
void CPU_NMI(CPU_6502 *cpu);
opcode_func_t CPU_op_table[256];
CPU_addr_mode CPU_addr_mode_table[256];

char *CPU_op_names[256];

typedef struct {
  uint8_t cycles : 4;
  uint8_t cross : 4;
} CPU_op_cycles_t;
CPU_op_cycles_t CPU_op_cycles_table[256];

size_t CPU_disassemble(uint8_t *data, uint16_t size, char **out);
size_t CPU_disassemble_arr(uint8_t *data, uint32_t size, char ***out);
#endif // CPU_6502_H