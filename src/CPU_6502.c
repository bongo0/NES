#include "CPU_6502.h"

#include <stdint.h>

/*
design hmm..

    cpu_process_op( cpu, op_code ){

        op_table[op_code](cpu, op_addr_mode_table[op_code]);

    }
*/

typedef struct cpu_ {
  // CPU registers                                = at power-up
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
} CPU_6502;

// void* (CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BRK(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BPL(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *JSR(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BMI(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *RTI(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BVC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *RTS(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BVS(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *NOP(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BCC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *LDY(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BCS(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *CPY(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BNE(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *CPX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BEQ(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ORA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *AND(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *EOR(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ADC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *STA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *LDA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *CPA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SBC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *HLT(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *LDX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SLO(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *RLA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SRE(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *RRA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SAX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *AXA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *LAX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *DCP(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ISB(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *BIT(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *STY(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ASL_Memory(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ROL_Memory(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *LSR_Memory(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ROR_Memory(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *STX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *DEC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *INC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *PHP(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *CLC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *PLP(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SEC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *PHA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *CLI(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *PLA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SEI(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *DEY(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *TYA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *TAY(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *CLV(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *INY(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *CLD(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *INX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SED(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ASL_Acc(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ROL_Acc(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *LSR_Acc(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ROR_Acc(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *TXA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *TXS(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *TAX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *TSX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *DEX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *AAC(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ASR(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ARR(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *UNK(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *TAS(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *ATX(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *LAS(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *AXS(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *JMP_Abs(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *JMP_Ind(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SYA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};
void *SXA(CPU_6502 *cpu, CPU_addr_mode mode){return NULL;};

// clang-format off
opcode_func_t op_table[] = {
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

CPU_addr_mode addr_mode_table[] ={
//      0           1              2             3              4              5              6              7              8           9              A           B              C              D              E              F
        IMPLIED,    INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     M__Acc,     IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //0
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLIED,    ABSOLUTE_Y,    IMPLIED,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //1
        ABSOLUTE,   INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     M__Acc,     IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //2
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLIED,    ABSOLUTE_Y,    IMPLIED,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //3
        IMPLIED,    INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     M__Acc,     IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //4
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLIED,    ABSOLUTE_Y,    IMPLIED,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //5
        IMPLIED,    INDIRECT_X,    NONE,         INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     M__Acc,     IMMIDIATE,     INDIRECT,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //6
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLIED,    ABSOLUTE_Y,    IMPLIED,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //7
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     IMPLIED,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //8
        RELATIVE,   M__IndYW,      NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_Y,    ZEROPAGE_Y,    IMPLIED,    M__AbsYW,      IMPLIED,    M__AbsYW,      M__AbsXW,      M__AbsXW,      M__AbsYW,      M__AbsYW,    //9
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     IMPLIED,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //A
        RELATIVE,   INDIRECT_Y,    NONE,         INDIRECT_Y,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_Y,    ZEROPAGE_Y,    IMPLIED,    ABSOLUTE_Y,    IMPLIED,    ABSOLUTE_Y,    ABSOLUTE_X,    ABSOLUTE_X,    ABSOLUTE_Y,    ABSOLUTE_Y,  //B
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     IMPLIED,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //C
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLIED,    ABSOLUTE_Y,    IMPLIED,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW,    //D
        IMMIDIATE,  INDIRECT_X,    IMMIDIATE,    INDIRECT_X,    ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      ZEROPAGE,      IMPLIED,    IMMIDIATE,     IMPLIED,    IMMIDIATE,     ABSOLUTE,      ABSOLUTE,      ABSOLUTE,      ABSOLUTE,    //E
        RELATIVE,   INDIRECT_Y,    NONE,         M__IndYW,      ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    ZEROPAGE_X,    IMPLIED,    ABSOLUTE_Y,    IMPLIED,    M__AbsYW,      ABSOLUTE_X,    ABSOLUTE_X,    M__AbsXW,      M__AbsXW     //F
};
// clang-format on
