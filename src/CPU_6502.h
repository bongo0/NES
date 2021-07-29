#ifndef CPU_6502_H
#define CPU_6502_H
#include <stdint.h>
#include <stdio.h>

typedef struct cpu_6502 CPU_6502;

uint8_t fetchInstruction();

void executeInstruction(uint8_t opc);

enum CPU_ADDRESSING_MODE{
    IMMIDIATE,
    ZEROPAGE,
    ZEROPAGEX,
    ZEROPAGEY,
    ABSOLUTE,
    ABSOLUTEX,
    ABSOLUTEY,
    INDIRECTX,
    INDIRECTY,
    RELATIVE
};


#endif //CPU_6502_H