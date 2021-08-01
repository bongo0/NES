#ifndef CPU_6502_H
#define CPU_6502_H
#include <stdint.h>
#include <stdio.h>

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
#define CPU_RAM_SIZE 2048
#define CPU_VIDEO_RAM_SIZE 0x4000

typedef struct cpu_state_ {
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
} CPU_state;

// maybe make this opaque type
typedef struct cpu_ {
  CPU_state state;

  uint8_t ram[CPU_RAM_SIZE];
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
  IMPLICIT,
  NONE,
  INDIRECT,
  ACCUMULATE,
  // what are these???
  M__AbsYW,
  M__AbsXW,
  M__IndYW,

} CPU_addr_mode;

typedef void *(*opcode_func_t)(CPU_6502 *, CPU_addr_mode);

CPU_6502 *CPU_init(CPU_6502 *cpu);
CPU_state CPU_get_state(CPU_6502 *CPU);

void CPU_print_state(CPU_6502 *cpu, FILE *fd);

uint8_t CPU_get_status_flag(CPU_6502 *cpu, uint8_t flag);

void CPU_load_to_memory(CPU_6502 *cpu, uint8_t *data, uint16_t size);

// things that probably does not need to be public
void CPU_exec(CPU_6502 *CPU);
void CPU_exec_instruction(CPU_6502 *CPU, uint8_t op_code);
opcode_func_t CPU_op_table[256];
CPU_addr_mode CPU_addr_mode_table[256];

char *CPU_op_names[256];

typedef struct op_cycles_t {
  uint8_t cycles : 4;
  uint8_t cross : 4;
} CPU_op_cycles_t;
CPU_op_cycles_t CPU_op_cycles_table[256];

#endif // CPU_6502_H