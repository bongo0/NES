#include "CPU_6502.h"

#include <stdint.h>

typedef struct cpu_6502{
// CPU registers                                = at power-up
    uint8_t A;          // accumulator          = 0
    uint8_t X;          // X register           = 0
    uint8_t Y;          // Y register           = 0
    uint16_t PC;        // program counter      = 0
    uint8_t SP;         // stack pointer        = 0xFD
    uint8_t P;          // processor status reg = 0x34
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
} CPU_6502;

