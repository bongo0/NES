#include "CPU_6502.h"

#include <stdint.h>

/*
design hmm..

    cpu_process_op( cpu, op_code ){

        op_table[op_code](cpu, op_addr_mode_table[op_code]);

    }
*/

CPU_6502 *CPU_init(CPU_6502 *cpu) {
  // CPU registers                     = at power-up
  cpu->state.A = 0;     // accumulator          = 0
  cpu->state.X = 0;     // X register           = 0
  cpu->state.Y = 0;     // Y register           = 0
  cpu->state.PC = 0;    // program counter      = 0
  cpu->state.SP = 0xFD; // stack pointer        = 0xFD
  cpu->state.P = 0x34;  // processor status reg = 0x34

  return cpu;
};

uint8_t CPU_get_status_flag(CPU_6502 *cpu, uint8_t flag){return (cpu->state.P & flag);};

void CPU_set_status_carry(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_CARRY; };
void CPU_set_status_zero(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_ZERO; };
void CPU_set_status_interupt_disable(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_INTERUPT_DISABLE; };
void CPU_set_status_decimal(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_DECIMAL; };
void CPU_set_status_break(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_BREAK; };
void CPU_set_status_reserved(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_RESERVED; };
void CPU_set_status_overflow(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_OVERFLOW; };
void CPU_set_status_negative(CPU_6502 *cpu) { cpu->state.P |= CPU_STATUS_NEGATIVE; };

void CPU_clear_status_flags(CPU_6502 *cpu, uint8_t flags) { cpu->state.P &= ~flags; };

CPU_state CPU_get_state(CPU_6502 *CPU) { return CPU->state; };

uint8_t CPU_read_memory(CPU_6502 *CPU, uint16_t adr) {
  return CPU->ram[adr];
};

uint8_t CPU_get_op(CPU_6502 *CPU) {
  return CPU_read_memory(CPU, CPU->state.PC);
};

void CPU_exec(CPU_6502 *CPU) {

  uint8_t op = CPU_get_op(CPU);

  CPU_exec_instruction(CPU, op);

  // cycles
};

void CPU_exec_instruction(CPU_6502 *CPU, uint8_t op_code) {
  char *tmp = CPU_op_table[op_code](CPU, CPU_addr_mode_table[op_code]);
  printf("0x%02x == %s\n", op_code, tmp);
};



uint16_t CPU_get_operand(CPU_6502 *cpu, uint8_t op_code) { return 0; };

//#v	Immediate	Uses the 8-bit operand itself as the value for the
// operation, rather than fetching a value from a memory address.
uint16_t CPU_get_immidiate(CPU_6502 *cpu) {
  return CPU_read_memory(cpu, ++cpu->state.PC);
};

// Implicit	Instructions like RTS or CLC have no address operand, the
// destination of results are implied.
uint16_t CPU_get_implicit(CPU_6502 *cpu, uint16_t addr) { return 0; };

// d	Zero page	Fetches the value from an 8-bit address on the zero
// page.
uint16_t CPU_get_zeropage(CPU_6502 *cpu, uint16_t addr) { return 0; };

// d,x	Zero page indexed	val = PEEK((arg + X) % 256)	4
uint16_t CPU_get_zeropage_idx_X(CPU_6502 *cpu, uint16_t addr) { return 0; };

// d,y	Zero page indexed	val = PEEK((arg + Y) % 256)	4
uint16_t CPU_get_zeropage_idx_Y(CPU_6502 *cpu, uint16_t addr) { return 0; };

//(a)	Indirect	The JMP instruction has a special indirect addressing
// mode that can jump to the address stored in a 16-bit pointer anywhere in
// memory.
uint16_t CPU_get_indirect(CPU_6502 *cpu, uint16_t addr) { return 0; };

//(d,x)	Indexed indirect	val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X
//+ 1) % 256) * 256)	6
uint16_t CPU_get_indirect_idx_X(CPU_6502 *cpu, uint16_t addr) { return 0; };

//(d),y	Indirect indexed	val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256)
//* 256 + Y)	5+ (+ means add a cycle for write instructions or for page
// wrapping on read instructions)
uint16_t CPU_get_indirect_idx_Y(CPU_6502 *cpu, uint16_t addr) { return 0; };

// a	Absolute	Fetches the value from a 16-bit address anywhere in
// memory.
uint16_t CPU_get_absolute(CPU_6502 *cpu, uint16_t addr) { return 0; };

// a,x	Absolute indexed	val = PEEK(arg + X)	4+ (+ means add a cycle
// for write instructions or for page wrapping on read instructions)
uint16_t CPU_get_absolute_idx_X(CPU_6502 *cpu, uint16_t addr) { return 0; };

// a,y	Absolute indexed	val = PEEK(arg + Y)	4+ (+ means add a cycle
// for write instructions or for page wrapping on read instructions)
uint16_t CPU_get_absolute_idx_Y(CPU_6502 *cpu, uint16_t addr) { return 0; };

// A	Accumulator	Many instructions can operate on the accumulator, e.g.
// LSR A. Some assemblers will treat no operand as an implicit A where
// applicable.
uint16_t CPU_get_accumulate(CPU_6502 *cpu, uint16_t addr) { return 0; };

// label	Relative	Branch instructions (e.g. BEQ, BCS) have a
// relative addressing mode that specifies an 8-bit signed offset relative to
// the current PC.
uint16_t CPU_get_relative(CPU_6502 *cpu, uint16_t addr) { return 0; };

// void* (CPU_6502 *cpu, CPU_addr_mode mode){return "";};
void *BRK(CPU_6502 *cpu, CPU_addr_mode mode) { return "BRK"; };
void *BPL(CPU_6502 *cpu, CPU_addr_mode mode) { return "BPL"; };
void *JSR(CPU_6502 *cpu, CPU_addr_mode mode) { return "JSR"; };
void *BMI(CPU_6502 *cpu, CPU_addr_mode mode) { return "BMI"; };
void *RTI(CPU_6502 *cpu, CPU_addr_mode mode) { return "RTI"; };
void *BVC(CPU_6502 *cpu, CPU_addr_mode mode) { return "BVC"; };
void *RTS(CPU_6502 *cpu, CPU_addr_mode mode) { return "RTS"; };
void *BVS(CPU_6502 *cpu, CPU_addr_mode mode) { return "BVS"; };
void *NOP(CPU_6502 *cpu, CPU_addr_mode mode) { return "NOP"; };
void *BCC(CPU_6502 *cpu, CPU_addr_mode mode) { return "BCC"; };
void *LDY(CPU_6502 *cpu, CPU_addr_mode mode) { return "LDY"; };
void *BCS(CPU_6502 *cpu, CPU_addr_mode mode) { return "BCS"; };
void *CPY(CPU_6502 *cpu, CPU_addr_mode mode) { return "CPY"; };
void *BNE(CPU_6502 *cpu, CPU_addr_mode mode) { return "BNE"; };
void *CPX(CPU_6502 *cpu, CPU_addr_mode mode) { return "CPX"; };
void *BEQ(CPU_6502 *cpu, CPU_addr_mode mode) { return "BEQ"; };
void *ORA(CPU_6502 *cpu, CPU_addr_mode mode) { return "ORA"; };
void *AND(CPU_6502 *cpu, CPU_addr_mode mode) { return "AND"; };
void *EOR(CPU_6502 *cpu, CPU_addr_mode mode) { return "EOR"; };
void *ADC(CPU_6502 *cpu, CPU_addr_mode mode) { return "ADC"; };
void *STA(CPU_6502 *cpu, CPU_addr_mode mode) { return "STA"; };
void *LDA(CPU_6502 *cpu, CPU_addr_mode mode) { return "LDA"; };
void *CPA(CPU_6502 *cpu, CPU_addr_mode mode) { return "CPA"; };
void *SBC(CPU_6502 *cpu, CPU_addr_mode mode) { return "SBC"; };
void *HLT(CPU_6502 *cpu, CPU_addr_mode mode) { return "HLT"; };

void *LDX(CPU_6502 *cpu, CPU_addr_mode mode) {
  if (mode == IMMIDIATE) {
    uint16_t operand = CPU_get_immidiate(cpu);

    CPU_clear_status_flags(cpu, CPU_STATUS_ZERO | CPU_STATUS_NEGATIVE); 
    if (operand == 0) {
      CPU_set_status_zero(cpu);
    }
    else if (operand & 0x80) {
      CPU_set_status_negative(cpu);
    }

    cpu->state.X = operand;
  }
  return "LDX";
};

void *SLO(CPU_6502 *cpu, CPU_addr_mode mode) { return "SLO"; };
void *RLA(CPU_6502 *cpu, CPU_addr_mode mode) { return "RLA"; };
void *SRE(CPU_6502 *cpu, CPU_addr_mode mode) { return "SRE"; };
void *RRA(CPU_6502 *cpu, CPU_addr_mode mode) { return "RRA"; };
void *SAX(CPU_6502 *cpu, CPU_addr_mode mode) { return "SAX"; };
void *AXA(CPU_6502 *cpu, CPU_addr_mode mode) { return "AXA"; };
void *LAX(CPU_6502 *cpu, CPU_addr_mode mode) { return "LAX"; };
void *DCP(CPU_6502 *cpu, CPU_addr_mode mode) { return "DCP"; };
void *ISB(CPU_6502 *cpu, CPU_addr_mode mode) { return "ISB"; };
void *BIT(CPU_6502 *cpu, CPU_addr_mode mode) { return "BIT"; };
void *STY(CPU_6502 *cpu, CPU_addr_mode mode) { return "STY"; };
void *ASL_Memory(CPU_6502 *cpu, CPU_addr_mode mode) { return "ASL_Memory"; };
void *ROL_Memory(CPU_6502 *cpu, CPU_addr_mode mode) { return "ROL_Memory"; };
void *LSR_Memory(CPU_6502 *cpu, CPU_addr_mode mode) { return "LSR_Memory"; };
void *ROR_Memory(CPU_6502 *cpu, CPU_addr_mode mode) { return "ROR_Memory"; };
void *STX(CPU_6502 *cpu, CPU_addr_mode mode) { return "STX"; };
void *DEC(CPU_6502 *cpu, CPU_addr_mode mode) { return "DEC"; };
void *INC(CPU_6502 *cpu, CPU_addr_mode mode) { return "INC"; };
void *PHP(CPU_6502 *cpu, CPU_addr_mode mode) { return "PHP"; };
void *CLC(CPU_6502 *cpu, CPU_addr_mode mode) { return "CLC"; };
void *PLP(CPU_6502 *cpu, CPU_addr_mode mode) { return "PLP"; };
void *SEC(CPU_6502 *cpu, CPU_addr_mode mode) { return "SEC"; };
void *PHA(CPU_6502 *cpu, CPU_addr_mode mode) { return "PHA"; };
void *CLI(CPU_6502 *cpu, CPU_addr_mode mode) { return "CLI"; };
void *PLA(CPU_6502 *cpu, CPU_addr_mode mode) { return "PLA"; };
void *SEI(CPU_6502 *cpu, CPU_addr_mode mode) { return "SEI"; };
void *DEY(CPU_6502 *cpu, CPU_addr_mode mode) { return "DEY"; };
void *TYA(CPU_6502 *cpu, CPU_addr_mode mode) { return "TYA"; };
void *TAY(CPU_6502 *cpu, CPU_addr_mode mode) { return "TAY"; };
void *CLV(CPU_6502 *cpu, CPU_addr_mode mode) { return "CLV"; };
void *INY(CPU_6502 *cpu, CPU_addr_mode mode) { return "INY"; };
void *CLD(CPU_6502 *cpu, CPU_addr_mode mode) { return "CLD"; };
void *INX(CPU_6502 *cpu, CPU_addr_mode mode) { return "INX"; };
void *SED(CPU_6502 *cpu, CPU_addr_mode mode) { return "SED"; };
void *ASL_Acc(CPU_6502 *cpu, CPU_addr_mode mode) { return "ASL_Acc"; };
void *ROL_Acc(CPU_6502 *cpu, CPU_addr_mode mode) { return "ROL_Acc"; };
void *LSR_Acc(CPU_6502 *cpu, CPU_addr_mode mode) { return "LSR_Acc"; };
void *ROR_Acc(CPU_6502 *cpu, CPU_addr_mode mode) { return "ROR_Acc"; };
void *TXA(CPU_6502 *cpu, CPU_addr_mode mode) { return "TXA"; };
void *TXS(CPU_6502 *cpu, CPU_addr_mode mode) { return "TXS"; };
void *TAX(CPU_6502 *cpu, CPU_addr_mode mode) { return "TAX"; };
void *TSX(CPU_6502 *cpu, CPU_addr_mode mode) { return "TSX"; };
void *DEX(CPU_6502 *cpu, CPU_addr_mode mode) { return "DEX"; };
void *AAC(CPU_6502 *cpu, CPU_addr_mode mode) { return "AAC"; };
void *ASR(CPU_6502 *cpu, CPU_addr_mode mode) { return "ASR"; };
void *ARR(CPU_6502 *cpu, CPU_addr_mode mode) { return "ARR"; };
void *UNK(CPU_6502 *cpu, CPU_addr_mode mode) { return "UNK"; };
void *TAS(CPU_6502 *cpu, CPU_addr_mode mode) { return "TAS"; };
void *ATX(CPU_6502 *cpu, CPU_addr_mode mode) { return "ATX"; };
void *LAS(CPU_6502 *cpu, CPU_addr_mode mode) { return "LAS"; };
void *AXS(CPU_6502 *cpu, CPU_addr_mode mode) { return "AXS"; };
void *JMP_Abs(CPU_6502 *cpu, CPU_addr_mode mode) { return "JMP_Abs"; };
void *JMP_Ind(CPU_6502 *cpu, CPU_addr_mode mode) { return "JMP_Ind"; };
void *SYA(CPU_6502 *cpu, CPU_addr_mode mode) { return "SYA"; };
void *SXA(CPU_6502 *cpu, CPU_addr_mode mode) { return "SXA"; };


// Look up tables
// clang-format off
opcode_func_t CPU_op_table[] = {
//      0        1        2        3        4        5        6                7        8        9        A                B        C                D        E                F
        &BRK,    &ORA,    &HLT,    &SLO,    &NOP,    &ORA,    &ASL_Memory,    &SLO,    &PHP,    &ORA,    &ASL_Acc,        &AAC,    &NOP,            &ORA,    &ASL_Memory,    &SLO, //0
        &BPL,    &ORA,    &HLT,    &SLO,    &NOP,    &ORA,    &ASL_Memory,    &SLO,    &CLC,    &ORA,    &NOP,            &SLO,    &NOP,            &ORA,    &ASL_Memory,    &SLO, //1
        &JSR,    &AND,    &HLT,    &RLA,    &BIT,    &AND,    &ROL_Memory,    &RLA,    &PLP,    &AND,    &ROL_Acc,        &AAC,    &BIT,            &AND,    &ROL_Memory,    &RLA, //2
        &BMI,    &AND,    &HLT,    &RLA,    &NOP,    &AND,    &ROL_Memory,    &RLA,    &SEC,    &AND,    &NOP,            &RLA,    &NOP,            &AND,    &ROL_Memory,    &RLA, //3
        &RTI,    &EOR,    &HLT,    &SRE,    &NOP,    &EOR,    &LSR_Memory,    &SRE,    &PHA,    &EOR,    &LSR_Acc,        &ASR,    &JMP_Abs,        &EOR,    &LSR_Memory,    &SRE, //4
        &BVC,    &EOR,    &HLT,    &SRE,    &NOP,    &EOR,    &LSR_Memory,    &SRE,    &CLI,    &EOR,    &NOP,            &SRE,    &NOP,            &EOR,    &LSR_Memory,    &SRE, //5
        &RTS,    &ADC,    &HLT,    &RRA,    &NOP,    &ADC,    &ROR_Memory,    &RRA,    &PLA,    &ADC,    &ROR_Acc,        &ARR,    &JMP_Ind,        &ADC,    &ROR_Memory,    &RRA, //6
        &BVS,    &ADC,    &HLT,    &RRA,    &NOP,    &ADC,    &ROR_Memory,    &RRA,    &SEI,    &ADC,    &NOP,            &RRA,    &NOP,            &ADC,    &ROR_Memory,    &RRA, //7
        &NOP,    &STA,    &NOP,    &SAX,    &STY,    &STA,    &STX,           &SAX,    &DEY,    &NOP,    &TXA,            &UNK,    &STY,            &STA,    &STX,           &SAX, //8
        &BCC,    &STA,    &HLT,    &AXA,    &STY,    &STA,    &STX,           &SAX,    &TYA,    &STA,    &TXS,            &TAS,    &SYA,            &STA,    &SXA,           &AXA, //9
        &LDY,    &LDA,    &LDX,    &LAX,    &LDY,    &LDA,    &LDX,           &LAX,    &TAY,    &LDA,    &TAX,            &ATX,    &LDY,            &LDA,    &LDX,           &LAX, //A
        &BCS,    &LDA,    &HLT,    &LAX,    &LDY,    &LDA,    &LDX,           &LAX,    &CLV,    &LDA,    &TSX,            &LAS,    &LDY,            &LDA,    &LDX,           &LAX, //B
        &CPY,    &CPA,    &NOP,    &DCP,    &CPY,    &CPA,    &DEC,           &DCP,    &INY,    &CPA,    &DEX,            &AXS,    &CPY,            &CPA,    &DEC,           &DCP, //C
        &BNE,    &CPA,    &HLT,    &DCP,    &NOP,    &CPA,    &DEC,           &DCP,    &CLD,    &CPA,    &NOP,            &DCP,    &NOP,            &CPA,    &DEC,           &DCP, //D
        &CPX,    &SBC,    &NOP,    &ISB,    &CPX,    &SBC,    &INC,           &ISB,    &INX,    &SBC,    &NOP,            &SBC,    &CPX,            &SBC,    &INC,           &ISB, //E
        &BEQ,    &SBC,    &HLT,    &ISB,    &NOP,    &SBC,    &INC,           &ISB,    &SED,    &SBC,    &NOP,            &ISB,    &NOP,            &SBC,    &INC,           &ISB  //F
};

CPU_addr_mode CPU_addr_mode_table[] ={
//      0           1              2             3              4              5              6              7              8           9              A             B              C              D              E              F
        IMPLICIT,   INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     ACCUMULATE,  IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //0
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLICIT,    ABSOLUTE_Y,    IMPLICIT,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //1
        ABSOLUTE,   INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     ACCUMULATE,  IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //2
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLICIT,    ABSOLUTE_Y,    IMPLICIT,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //3
        IMPLICIT,   INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     ACCUMULATE,  IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //4
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLICIT,    ABSOLUTE_Y,    IMPLICIT,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //5
        IMPLICIT,   INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     ACCUMULATE,  IMMIDIATE,     INDIRECT,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //6
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLICIT,    ABSOLUTE_Y,    IMPLICIT,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //7
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     IMPLICIT,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //8
        RELATIVE,   M__IndYW,      NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_Y,    ZEROPAGE_Y,    IMPLICIT,    M__AbsYW,      IMPLICIT,    M__AbsYW,      M__AbsXW,      M__AbsXW,      M__AbsYW,      M__AbsYW,    //9
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     IMPLICIT,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //A
        RELATIVE,   INDIRECT_Y,    NONE,         INDIRECT_Y,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_Y,    ZEROPAGE_Y,    IMPLICIT,    ABSOLUTE_Y,    IMPLICIT,    ABSOLUTE_Y,    ABSOLUTE_X,    ABSOLUTE_X,    ABSOLUTE_Y,    ABSOLUTE_Y,  //B
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     IMPLICIT,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //C
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLICIT,    ABSOLUTE_Y,    IMPLICIT,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //D
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLICIT,    IMMIDIATE,     IMPLICIT,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //E
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLICIT,    ABSOLUTE_Y,    IMPLICIT,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW     //F
};

CPU_op_cycles_t CPU_op_cycles_table[] ={
//      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A,  B,  C,  D,  E,  F
        7,  6,  0,  8,  3,  3,  5,  5,  3,  2,  2,  2,  4,  4,  6,  6, //0
        2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, //1
        6,  6,  0,  8,  3,  3,  5,  5,  4,  2,  2,  2,  4,  4,  6,  6, //2
        2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, //3
        6,  6,  0,  8,  3,  3,  5,  5,  3,  2,  2,  2,  3,  4,  6,  6, //4
        2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, //5
        6,  6,  0,  8,  3,  3,  5,  5,  4,  2,  2,  2,  5,  4,  6,  6, //6
        2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, //7
        2,  6,  2,  6,  3,  3,  3,  3,  2,  2,  2,  2,  4,  4,  4,  4, //8
        2,  6,  0,  6,  4,  4,  4,  4,  2,  5,  2,  5,  5,  5,  5,  5, //9
        2,  6,  2,  6,  3,  3,  3,  3,  2,  2,  2,  2,  4,  4,  4,  4, //A
        2,  5,  0,  5,  4,  4,  4,  4,  2,  4,  2,  4,  4,  4,  4,  4, //B
        2,  6,  2,  8,  3,  3,  5,  5,  2,  2,  2,  2,  4,  4,  6,  6, //C
        2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, //D
        2,  6,  2,  8,  3,  3,  5,  5,  2,  2,  2,  2,  4,  4,  6,  6, //E
        2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7  //F
};
// clang-format on
