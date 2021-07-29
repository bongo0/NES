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

CPU_state CPU_get_state(CPU_6502 *CPU) { return CPU->state; };

void CPU_exec_instruction(CPU_6502 *CPU, uint8_t op_code) {
  char *tmp = CPU_op_table[op_code](CPU, CPU_addr_mode_table[op_code]);
  printf("0x%02x == %s\n", op_code, tmp);
};

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
void *LDX(CPU_6502 *cpu, CPU_addr_mode mode) { return "LDX"; };
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
