#ifndef CPU_6502_H
#define CPU_6502_H
#include <stdint.h>
#include <stdio.h>

typedef struct cpu_ CPU_6502;

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

opcode_func_t op_table[256];        // probably does not need to be bublic
CPU_addr_mode addr_mode_table[256]; // probably does not need to be bublic

#endif // CPU_6502_H