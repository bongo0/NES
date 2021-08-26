#include "disassembler.h"

static const char Instruction_info_table[256][28] = {
    "Break                      ",
    "Inclusive OR with A        ",
    "(unofficial)Halt           ",
    "(unofficial)SLO            ",
    "No Operation               ",
    "Inclusive OR with A        ",
    "Arithmetic reg Shift Left  " /*Memory*/,
    "(unofficial)SLO            ",
    "Push Status to stack       ",
    "Inclusive OR with A        ",
    "Arithmetic reg Shift Left  " /*Acc*/,
    "(unofficial)AAC            ",
    "No Operation               ",
    "Inclusive OR with A        ",
    "Arithmetic reg Shift Left  " /*Memory*/,
    "(unofficial)SLO            ", /* 0*/
    "Branch if Positive         ",
    "Inclusive OR with A        ",
    "(unofficial)Halt           ",
    "(unofficial)SLO            ",
    "No Operation               ",
    "Inclusive OR with A        ",
    "Arithmetic reg Shift Left  " /*Memory*/,
    "(unofficial)SLO            ",
    "Clear Carry flag           ",
    "Inclusive OR with A        ",
    "No Operation               ",
    "(unofficial)SLO            ",
    "No Operation               ",
    "Inclusive OR with A        ",
    "Arithmetic reg Shift Left  " /*Memory*/,
    "(unofficial)SLO            ", // 1
    "Jump to Subroutine         ",
    "Bitwise AND with A register",
    "(unofficial)Halt           ",
    "(unofficial)RLA            ",
    "Bit Test                   ",
    "Bitwise AND with A register",
    "Rotate Left                " /*Memory*/,
    "(unofficial)RLA            ",
    "Pop stack to Status        ",
    "Bitwise AND with A register",
    "Rotate Left                " /*Acc*/,
    "(unofficial)AAC            ",
    "Bit Test                   ",
    "Bitwise AND with A register",
    "Rotate Left                " /*Memory*/,
    "(unofficial)RLA            ", // 2*/
    "Branch if Negative set     ",
    "Bitwise AND with A register",
    "(unofficial)Halt           ",
    "(unofficial)RLA            ",
    "No Operation               ",
    "Bitwise AND with A register",
    "Rotate Left                " /*Memory*/,
    "(unofficial)RLA            ",
    "Set Carry Flag             ",
    "Bitwise AND with A register",
    "No Operation               ",
    "(unofficial)RLA            ",
    "No Operation               ",
    "Bitwise AND with A register",
    "Rotate Left                " /*Memory*/,
    "(unofficial)RLA            ", // 3*/
    "Return from Interrupt      ",
    "Exclusive OR with A        ",
    "(unofficial)Halt           ",
    "(unofficial)SRE            ",
    "No Operation               ",
    "Exclusive OR with A        ",
    "Logical Shift Right        " /*Memory*/,
    "(unofficial)SRE            ",
    "Push Accumulator to stack  ",
    "Exclusive OR with A        ",
    "Logical Shift Right        " /*Acc*/,
    "(unofficial)ASR            ",
    "Jump to address            ",
    "Exclusive OR with A        ",
    "Logical Shift Right        " /*Memory*/,
    "(unofficial)SRE            ", // 4*/
    "Branch if Overflow Clear   ",
    "Exclusive OR with A        ",
    "(unofficial)Halt           ",
    "(unofficial)SRE            ",
    "No Operation               ",
    "Exclusive OR with A        ",
    "Logical Shift Right        " /*Memory*/,
    "(unofficial)SRE            ",
    "Clear Interupt Disable     ",
    "Exclusive OR with A        ",
    "No Operation               ",
    "(unofficial)SRE            ",
    "No Operation               ",
    "Exclusive OR with A        ",
    "Logical Shift Right        " /*Memory*/,
    "(unofficial)SRE            ", // 5*/
    "Return from Subroutine     ",
    "Add with Carry to A        ",
    "(unofficial)Halt           ",
    "(unofficial)RRA            ",
    "No Operation               ",
    "Add with Carry to A        ",
    "Rotate Right               " /*Memory*/,
    "(unofficial)RRA            ",
    "Pop stack to Accumulator   ",
    "Add with Carry to A        ",
    "Rotate Right               " /*Acc*/,
    "(unofficial)ARR            ",
    "Jump to address            " /*Ind*/,
    "Add with Carry to A        ",
    "Rotate Right               " /*Memory*/,
    "(unofficial)RRA            ", // 6*/
    "Branch if Overflow Set     ",
    "Add with Carry to A        ",
    "(unofficial)Halt           ",
    "(unofficial)RRA            ",
    "No Operation               ",
    "Add with Carry to A        ",
    "Rotate Right               "   /*Memory*/,
    "(unofficial)RRA            ",
    "Set Interupt Disable       ",
    "Add with Carry to A        ",
    "No Operation               ",
    "(unofficial)RRA            ",
    "No Operation               ",
    "Add with Carry to A        ",
    "Rotate Right               " /*Memory*/,
    "(unofficial)RRA            ", // 7*/
    "No Operation               ",
    "Store Accumulator to       ",
    "No Operation               ",
    "(unofficial)SAX            ",
    "Store Y register           ",
    "Store Accumulator to       ",
    "Store X register           ",
    "(unofficial)SAX            ",
    "Decrement Y register       ",
    "No Operation               ",
    "Transfer X to A            ",
    "(unofficial)UNK            ",
    "Store Y register           ",
    "Store Accumulator to       ",
    "Store X register           ",
    "(unofficial)SAX            ", // 8*/
    "Branch if Carry Clear      ",
    "Store Accumulator to       ",
    "(unofficial)Halt           ",
    "(unofficial)AXA            ",
    "Store Y register           ",
    "Store Accumulator to       ",
    "Store X register           ",
    "(unofficial)SAX            ",
    "Transfer Y to A            ",
    "Store Accumulator to       ",
    "Transfer X to SP           ",
    "(unofficial)TAS            ",
    "(unofficial)SYA            ",
    "Store Accumulator to       ",
    "(unofficial)SXA            ",
    "(unofficial)AXA            ", // 9*/
    "Load to Y register         ",
    "Load to Accumulator        ",
    "Load to X register         ",
    "(unofficial)LAX            ",
    "Load to Y register         ",
    "Load to Accumulator        ",
    "Load to X register         ",
    "(unofficial)LAX            ",
    "Transfer A to Y            ",
    "Load to Accumulator        ",
    "Transfer A to X            ",
    "(unofficial)ATX            ",
    "Load to Y register         ",
    "Load to Accumulator        ",
    "Load to X register         ",
    "(unofficial)LAX            ", // A*/
    "Branch if Carry Set        ",
    "Load to Accumulator        ",
    "(unofficial)Halt           ",
    "(unofficial)LAX            ",
    "Load to Y register         ",
    "Load to Accumulator        ",
    "Load to X register         ",
    "(unofficial)LAX            ",
    "(unofficial)CLV            ",
    "Load to Accumulator        ",
    "(unofficial)TSX            ",
    "(unofficial)LAS            ",
    "Load to Y register         ",
    "Load to Accumulator        ",
    "Load to X register         ",
    "(unofficial)LAX            ", // B*/
    "Compare with Y register    ",
    "Compare with Accumulator   " /*CPA*/,
    "No Operation               ",
    "(unofficial)DCP            ",
    "Compare with Y register    ",
    "Compare with Accumulator   ",/*CPA*/
    "Decrement memory           ",
    "(unofficial)DCP            ",
    "Increment Y register       ",
    "Compare with Accumulator   " /*CPA*/,
    "Decrement X register       ",
    "(unofficial)AXS            ",
    "Compare with Y register    ",
    "Compare with Accumulator   " /*CPA*/,
    "Decrement memory           ",
    "(unofficial)DCP            ", // C*/
    "Branch if Not Equal        ",
    "Compare with Accumulator   " /*CPA*/,
    "(unofficial)Halt           ",
    "(unofficial)DCP            ",
    "No Operation               ",
    "Compare with Accumulator   ",/*CPA*/ 
    "Decrement memory           ",
    "(unofficial)DCP            ",
    "Clear Decimal Mode         ",
    "Compare with Accumulator   " /*CPA*/,
    "No Operation               ",
    "(unofficial)DCP            ",
    "No Operation               ",
    "Compare with Accumulator   " /*CPA*/,
    "Decrement memory           ",
    "(unofficial)DCP            ", // D*/
    "Compare with X register    ",
    "Substract with Carry from A",
    "No Operation               ",
    "(unofficial)ISB            ",
    "Compare with X register    ",
    "Substract with Carry from A",
    "Increment memory           ",
    "(unofficial)ISB            ",
    "Increment X register       ",
    "Substract with Carry from A",
    "No Operation               ",
    "Substract with Carry from A",
    "Compare with X register    ",
    "Substract with Carry from A",
    "Increment memory           ",
    "(unofficial)ISB            ", // E*/
    "Branch if Equal            ",
    "Substract with Carry from A",
    "(unofficial)Halt           ",
    "(unofficial)ISB            ",
    "No Operation               ",
    "Substract with Carry from A",
    "Increment memory           ",
    "(unofficial)ISB            ",
    "Set Decimal flag           ",
    "Substract with Carry from A",
    "No Operation               ",
    "(unofficial)ISB            ",
    "No Operation               ",
    "Substract with Carry from A",
    "Increment memory           ",
    "(unofficial)ISB            " // F*/
};

// clang-format off
char * Dissassemble_at(size_t adr,uint8_t *data, size_t size, uint8_t verbose){

    char *out = malloc(DISASM_LINE_LEN);
    out[0]=0;
    uint8_t operand;
    uint8_t hi;

    if(adr>=size){
        snprintf(out,DISASM_LINE_LEN,"[DISASSEMBLER] Adr out of range adr:%lu>=%lu\n",adr, size);
        return out;
    }

    uint8_t op = data[adr];
    
    operand=data[1+adr];
    hi=data[2+adr];
    switch (CPU_addr_mode_table[op]) {//      w5     w7                w11         (2)       s 
case IMMIDIATE: snprintf(out,DISASM_LINE_LEN,"%s  "  "#$%02X   "    "  ;IMMIDIATE  (0x%02X) %s",CPU_op_names[op],   operand, op, Instruction_info_table[op]);break;
case ZEROPAGE:  snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X    "    "  ;ZEROPAGE   (0x%02X) %s",CPU_op_names[op],   operand, op, Instruction_info_table[op]);break;
case ZEROPAGE_X:snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X,X  "    "  ;ZEROPAGE_X (0x%02X) %s",CPU_op_names[op],   operand, op, Instruction_info_table[op]);break;
case ZEROPAGE_Y:snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X,Y    "  "  ;ZEROPAGE_Y (0x%02X) %s",CPU_op_names[op],   operand, op, Instruction_info_table[op]);break;
case ABSOLUTE:  snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X%02X  "  "  ;ABSOLUTE   (0x%02X) %s",CPU_op_names[op],hi,operand, op, Instruction_info_table[op]);break;
case ABSOLUTE_X:snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X%02X,X"  "  ;ABSOLUTE_X (0x%02X) %s",CPU_op_names[op],hi,operand, op, Instruction_info_table[op]);break;
case ABSOLUTE_Y:snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X%02X,Y"  "  ;ABSOLUTE_Y (0x%02X) %s",CPU_op_names[op],hi,operand, op, Instruction_info_table[op]);break;
case INDIRECT:  snprintf(out,DISASM_LINE_LEN,"%s  "  "($%02X%02X)"  "  ;INDIRECT   (0x%02X) %s",CPU_op_names[op],hi,operand, op, Instruction_info_table[op]);break;
case INDIRECT_X:snprintf(out,DISASM_LINE_LEN,"%s  "  "($%02X,X)"    "  ;INDIRECT_X (0x%02X) %s",CPU_op_names[op]   ,operand, op, Instruction_info_table[op]);break;
case INDIRECT_Y:snprintf(out,DISASM_LINE_LEN,"%s  "  "($%02X),Y"    "  ;INDIRECT_Y (0x%02X) %s",CPU_op_names[op]   ,operand, op, Instruction_info_table[op]);break;
case RELATIVE:  snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X    "    "  ;RELATIVE   (0x%02X) %s",CPU_op_names[op],   operand, op, Instruction_info_table[op]);break;
case IMPLICIT:  snprintf(out,DISASM_LINE_LEN,"%s  "  "       "      "  ;IMPLICIT   (0x%02X) %s",CPU_op_names[op],            op, Instruction_info_table[op]);break;
case ACCUMULATE:snprintf(out,DISASM_LINE_LEN,"%s  "  "       "      "  ;ACCUMULATE (0x%02X) %s",CPU_op_names[op],            op, Instruction_info_table[op]);break;
case AbsYW:     snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X%02X,Y"  "  ;ABS_YW     (0x%02X) %s",CPU_op_names[op],hi,operand, op, Instruction_info_table[op]);break;
case AbsXW:     snprintf(out,DISASM_LINE_LEN,"%s  "  "$%02X%02X,X"  "  ;ABS_XW     (0x%02X) %s",CPU_op_names[op],hi,operand, op, Instruction_info_table[op]);break;
case IndYW:     snprintf(out,DISASM_LINE_LEN,"%s  "  "($%02X),Y"    "  ;IND_YW     (0x%02X) %s",CPU_op_names[op]   ,operand, op, Instruction_info_table[op]);break;
case NONE:      snprintf(out,DISASM_LINE_LEN,"%s  "  "       "      "  ;NONE       (0x%02X) %s",CPU_op_names[op],            op, Instruction_info_table[op]);break;
      
      default:LOG_ERROR("[DISASSEMBLER] THIS CANNOT HAPPEN\n");break;
  }
  return out;
}


size_t Disassemble(uint8_t *data, size_t size, Disassembly6502 *out){
    out->lines = (char**)malloc( size*sizeof(char*));
    out->size=size;
    if (out->lines == NULL){
      out->size=0;
      return 0;
    }
    for(size_t i = 0; i<size;++i){
      (out->lines)[i] = Dissassemble_at(i, data, size, 1);
    }
  return size;
}

uint8_t Disassembly6502_dump_to_file(Disassembly6502 *d, char *file_name){
  FILE *asm_file;
  asm_file = fopen(file_name, "w");
  if (asm_file == NULL) {
    LOG_ERROR("[DISASSEMBLER] Disassembly6502: could not open %s for writing\n", file_name);
    return 0;
  }

  for (size_t i = 0; i < d->size; ++i) {
      fprintf(asm_file, "%04lX   %s\n", i, d->lines[i]);
  }
  fclose(asm_file);
  LOG("[DISASSEMBLER] Disassembly6502: dumped %s\n",file_name);
  return 1;
}

void Disassembly6502_free(Disassembly6502 *d){
  for(size_t i = 0;i<d->size;++i){
    free(d->lines[i]);
  }
}