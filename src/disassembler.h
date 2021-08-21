#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "CPU_6502.h"

#define DISASM_LINE_LEN 128

typedef struct {
  char **lines;
  size_t size;

  //TODO
  size_t *program_lines; // filtered indices for program lines
} Disassembly6502;

char * Dissassemble_at(size_t adr,uint8_t *data, size_t size, uint8_t verbose);
size_t Disassemble(uint8_t *data, size_t size, Disassembly6502 *output);

uint8_t Disassembly6502_dump_to_file(Disassembly6502 *d, char *file_name);

void Disassembly6502_free(Disassembly6502 *d);


#endif // DISASSEMBLER_H