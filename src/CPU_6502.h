#ifndef CPU_6502_H
#define CPU_6502_H
#include <stdint.h>
#include <stdio.h>

typedef struct cpu_state_ {
  // CPU registers                     = at power-up
  uint8_t A;   // accumulator          = 0
  uint8_t X;   // X register           = 0
  uint8_t Y;   // Y register           = 0
  uint16_t PC; // program counter      = 0
  uint8_t SP;  // stack pointer        = 0xFD
  uint8_t P;   // processor status reg = 0x34
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
  uint64_t cycle_count;
} CPU_state;

// maybe make this opaque type
typedef struct cpu_ {
  CPU_state state;
  
  uint8_t ram[2048];
} CPU_6502;


typedef enum addr {
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
  IMPLIED,
  NONE,
  // what are these???
  INDIRECT,
  M__Acc,
  M__AbsYW,
  M__AbsXW,
  M__IndYW,

} CPU_addr_mode;

typedef void *(*opcode_func_t)(CPU_6502 *, CPU_addr_mode);

CPU_6502* CPU_init(CPU_6502 *cpu);
CPU_state CPU_get_state(CPU_6502 *CPU);



// things that probably does not need to be public
void CPU_exec_instruction(CPU_6502 *CPU, uint8_t op_code);
opcode_func_t CPU_op_table[256];
CPU_addr_mode CPU_addr_mode_table[256];

#endif // CPU_6502_H