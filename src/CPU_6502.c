#include "CPU_6502.h"

#include <memory.h>
#include <stdint.h>

uint16_t CPU_get_operand(CPU_6502 *cpu);
uint8_t CPU_get_op(CPU_6502 *cpu);

CPU_6502 *CPU_init(CPU_6502 *cpu) {
  // CPU registers                     = at power-up
  cpu->state.A = 0;     // accumulator          = 0
  cpu->state.X = 0;     // X register           = 0
  cpu->state.Y = 0;     // Y register           = 0
  cpu->state.PC = 0;    // program counter      = 0
  cpu->state.SP = 0xFD; // stack pointer        = 0xFD
  cpu->state.P = 0x34;  // processor status reg = 0x34

  cpu->state.cycle_count = 0;
  cpu->state.page_cross = 0;
  cpu->state.operand = 0;

  cpu->state.cycles_accumulated = 0;
  return cpu;
};

uint8_t CPU_get_status_flag(CPU_6502 *cpu, uint8_t flag) {
  return (cpu->state.P & flag);
};

void CPU_set_status_flags(CPU_6502 *cpu, uint8_t flags) {
  cpu->state.P |= flags;
}

void CPU_clear_status_flags(CPU_6502 *cpu, uint8_t flags) {
  cpu->state.P &= ~flags;
};

void CPU_print_state_(CPU_6502 *cpu, FILE *fd) {
  uint8_t op = CPU_get_op(cpu); // get current op
  char *mode;
  switch (CPU_addr_mode_table[op]) {
  case IMMIDIATE: mode = "IMMIDIATE"; break;
  case ZEROPAGE: mode = "ZEROPAGE"; break;
  case ZEROPAGE_X: mode = "ZEROPAGE_X"; break;
  case ZEROPAGE_Y: mode = "ZEROPAGE_Y"; break;
  case ABSOLUTE: mode = "ABSOLUTE"; break;
  case ABSOLUTE_X: mode = "ABSOLUTE_X"; break;
  case ABSOLUTE_Y: mode = "ABSOLUTE_Y"; break;
  case INDIRECT: mode = "INDIRECT"; break;
  case INDIRECT_X: mode = "INDIRECT_X"; break;
  case INDIRECT_Y: mode = "INDIRECT_Y"; break;
  case RELATIVE: mode = "RELATIVE"; break;
  case IMPLICIT: mode = "IMPLICIT"; break;
  case ACCUMULATE:
    mode = "ACCUMULATE";
    break;
    // what are these???
  case M__AbsYW: mode = "M__AbsYW"; break;
  case M__AbsXW: mode = "M__AbsXW"; break;
  case M__IndYW: mode = "M__IndYW"; break;
  case NONE: mode = "NONE"; break;
  default: mode = "fell through"; break;
  }

  fprintf(
      fd,
      "op 0x%02x %s %s last operand 0x%02x Cycle count %u+%u Page cross %d\n"
      "AC 0x%02x "
      "X  0x%02x "
      "Y  0x%02x \n"
      "PC 0x%04x "
      "SP 0x%02x "
      "P  0x%02x \n",
      op, CPU_op_names[op], mode, cpu->state.operand,
      CPU_op_cycles_table[op].cycles, CPU_op_cycles_table[op].cross,
      cpu->state.page_cross, cpu->state.A, cpu->state.X, cpu->state.Y,
      cpu->state.PC, cpu->state.SP, cpu->state.P);
}

void CPU_log_state_simple(CPU_6502 *cpu, FILE *fd, uint16_t last_PC,
                          uint8_t last_op) {
  fprintf(fd, "%04x  %02x  ", last_PC, last_op);
  if (cpu->state.operand > 0xff)
    fprintf(fd, "%02x %02x  ", (uint8_t)cpu->state.operand,
            cpu->state.operand >> 8);
  else
    fprintf(fd, "%02x     ", cpu->state.operand);
  fprintf(fd, "%s  ", CPU_op_names[last_op]);

  fprintf(fd, "A:%02x X:%02x Y:%02x P:%02x SP:%02x CYC:%lu\n", cpu->state.A,
          cpu->state.X, cpu->state.Y, cpu->state.P, cpu->state.SP,
          cpu->state.cycles_accumulated);
}

uint8_t is_page_crossed(uint16_t adr, uint8_t reg) {
  // is this correct ???
  return ((adr + reg) & 0xFF00) != (adr & 0xFF00);
}

CPU_state CPU_get_state(CPU_6502 *CPU) { return CPU->state; };

void CPU_load_to_memory(CPU_6502 *cpu, uint8_t *data, uint16_t size) {
  memcpy(cpu->ram, data, size);
};

uint8_t CPU_read_memory(CPU_6502 *CPU, uint16_t adr) { return CPU->ram[adr]; };

void CPU_write_memory(CPU_6502 *cpu, uint16_t adr, uint8_t val) {
  cpu->ram[adr] = val;
}

// stack location ($0100-$01FF)
#define STACK_LOCATION 0x100
void CPU_stack_push(CPU_6502 *cpu, uint8_t val) {
  CPU_write_memory(cpu, cpu->state.SP + STACK_LOCATION, val);
  cpu->state.SP--;
}

uint8_t CPU_stack_pop(CPU_6502 *cpu) {
  cpu->state.SP++;
  return CPU_read_memory(cpu, STACK_LOCATION + cpu->state.SP);
}

// does not change the state
uint8_t CPU_get_op(CPU_6502 *CPU) {
  return CPU_read_memory(CPU, CPU->state.PC);
};

void CPU_exec(CPU_6502 *cpu) {
  cpu->state.page_cross = 0;

  uint16_t last_PC = cpu->state.PC;
  uint8_t op = CPU_get_op(cpu);
  CPU_op_cycles_t ct = CPU_op_cycles_table[op];
  cpu->state.operand = CPU_get_operand(cpu);
  CPU_exec_instruction(cpu, op);

  // cycles
  for (uint8_t i = 0; i < ct.cycles; ++i) {
    // cycle
    cpu->state.cycles_accumulated++;
  }
  if (ct.cross && cpu->state.page_cross) {
    // cycle
    cpu->state.cycles_accumulated++;
  }
  // printf("\n");
  CPU_log_state_simple(cpu, stdout, last_PC, op);
};

inline void CPU_exec_instruction(CPU_6502 *CPU, uint8_t op_code) {
  CPU_op_table[op_code](CPU, CPU_addr_mode_table[op_code]);

  // printf("0x%02x == %s  mode: ", op_code, tmp);
  // printf("\n");
};

//#v	Immediate	Uses the 8-bit operand itself as the value for the
// operation, rather than fetching a value from a memory address.
uint16_t CPU_get_immidiate(CPU_6502 *cpu) {
  uint16_t operand = CPU_read_memory(cpu, ++cpu->state.PC);
  cpu->state.operand = operand;
  return operand;
};

// Implicit	Instructions like RTS or CLC have no address operand, the
// destination of results are implied.
uint16_t CPU_get_implicit(CPU_6502 *cpu) { return 0; };

// d	Zero page	Fetches the value from an 8-bit address on the zero
// page.
uint16_t CPU_get_zeropage(CPU_6502 *cpu) {
  uint16_t operand = CPU_read_memory(cpu, ++cpu->state.PC);
  cpu->state.operand = operand;
  return operand;
};

// d,x	Zero page indexed	val = PEEK((arg + X) % 256)	4
uint16_t CPU_get_zeropage_idx_X(CPU_6502 *cpu) {
  uint16_t operand = CPU_read_memory(cpu, ++cpu->state.PC);
  operand = (operand + cpu->state.X) % 256;
  cpu->state.operand = operand;
  return operand;
};

// d,y	Zero page indexed	val = PEEK((arg + Y) % 256)	4
uint16_t CPU_get_zeropage_idx_Y(CPU_6502 *cpu) {
  uint16_t operand = CPU_read_memory(cpu, ++cpu->state.PC);
  operand = (operand + cpu->state.Y) % 256;
  cpu->state.operand = operand;
  return operand;
};

//(a)	Indirect	The JMP instruction has a special indirect addressing
// mode that can jump to the address stored in a 16-bit pointer anywhere in
// memory.
uint16_t CPU_get_indirect(CPU_6502 *cpu) {
  uint8_t low = CPU_read_memory(cpu, ++cpu->state.PC);
  uint8_t high = CPU_read_memory(cpu, ++cpu->state.PC);
  uint16_t operand = low | (high << 8);
  cpu->state.operand = operand;
  return operand;
};

//(d,x)	Indexed indirect	val = PEEK(PEEK((arg + X) % 256) + PEEK((arg +
// X
//+ 1) % 256) * 256)	6
uint16_t CPU_get_indirect_idx_X(CPU_6502 *cpu) {
  uint16_t operand;
  uint16_t z = CPU_read_memory(cpu, ++cpu->state.PC);
  z += cpu->state.X;

  if (z == 0xff)
    operand = CPU_read_memory(cpu, 0xFF) | (CPU_read_memory(cpu, 0x00) << 8);
  else
    operand = CPU_read_memory(cpu, z) | (CPU_read_memory(cpu, z + 1) << 8);

  // should we check page cross here ??

  cpu->state.operand = operand;
  return operand;
};

//(d),y	Indirect indexed	val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256)
//* 256 + Y)	5+ (+ means add a cycle for write instructions or for page
// wrapping on read instructions)
uint16_t CPU_get_indirect_idx_Y(CPU_6502 *cpu) {
  uint16_t operand;
  uint16_t z = CPU_read_memory(cpu, ++cpu->state.PC);

  if (z == 0xff)
    operand = CPU_read_memory(cpu, 0xFF) | (CPU_read_memory(cpu, 0x00) << 8);
  else
    operand = CPU_read_memory(cpu, z) | (CPU_read_memory(cpu, z + 1) << 8);

  cpu->state.page_cross = is_page_crossed(operand, cpu->state.Y);

  operand += cpu->state.Y;
  cpu->state.operand = operand;
  return operand;
};

// a	Absolute	Fetches the value from a 16-bit address anywhere in
// memory.
uint16_t CPU_get_absolute(CPU_6502 *cpu) {
  uint8_t low = CPU_read_memory(cpu, ++cpu->state.PC);
  uint8_t high = CPU_read_memory(cpu, ++cpu->state.PC);
  uint16_t operand = low | (high << 8);
  cpu->state.operand = operand;
  return operand;
};

// a,x	Absolute indexed	val = PEEK(arg + X)	4+ (+ means add a cycle
// for write instructions or for page wrapping on read instructions)
uint16_t CPU_get_absolute_idx_X(CPU_6502 *cpu) {
  uint16_t operand = CPU_get_absolute(cpu);
  cpu->state.page_cross = is_page_crossed(operand, cpu->state.X);

  operand += cpu->state.X;

  cpu->state.operand = operand;
  return operand;
};

// a,y	Absolute indexed	val = PEEK(arg + Y)	4+ (+ means add a cycle
// for write instructions or for page wrapping on read instructions)
uint16_t CPU_get_absolute_idx_Y(CPU_6502 *cpu) {
  uint16_t operand = CPU_get_absolute(cpu);
  cpu->state.page_cross = is_page_crossed(operand, cpu->state.Y);

  operand += cpu->state.Y;

  cpu->state.operand = operand;
  return operand;
};

// A	Accumulator	Many instructions can operate on the accumulator, e.g.
// LSR A. Some assemblers will treat no operand as an implicit A where
// applicable.
uint16_t CPU_get_accumulate(CPU_6502 *cpu) { return 0; };

// label	Relative	Branch instructions (e.g. BEQ, BCS) have a
// relative addressing mode that specifies an 8-bit signed offset relative to
// the current PC.
uint16_t CPU_get_relative(CPU_6502 *cpu) { return CPU_get_immidiate(cpu); };

uint16_t CPU_get_operand(CPU_6502 *cpu) {
  CPU_addr_mode mode = CPU_addr_mode_table[CPU_get_op(cpu)];
  switch (mode) {
  case IMMIDIATE: return CPU_get_immidiate(cpu);
  case ZEROPAGE: return CPU_get_zeropage(cpu);
  case ZEROPAGE_X: return CPU_get_zeropage_idx_X(cpu);
  case ZEROPAGE_Y: return CPU_get_zeropage_idx_Y(cpu);
  case ABSOLUTE: return CPU_get_absolute(cpu);
  case ABSOLUTE_X: return CPU_get_absolute_idx_X(cpu);
  case ABSOLUTE_Y: return CPU_get_absolute_idx_Y(cpu);
  case INDIRECT: return CPU_get_indirect(cpu);
  case INDIRECT_X: return CPU_get_indirect_idx_X(cpu);
  case INDIRECT_Y: return CPU_get_indirect_idx_Y(cpu);
  case RELATIVE: return CPU_get_relative(cpu);
  case IMPLICIT: return CPU_get_implicit(cpu);
  case ACCUMULATE:
    return CPU_get_accumulate(cpu);
    // what are these???
  case M__AbsYW: return 0;
  case M__AbsXW: return 0;
  case M__IndYW: return 0;
  case NONE: return 0;
  default: return 0;
  }
  return 0;
};

// helpers for instructions
void CPU_branch_relative(CPU_6502 *cpu, uint8_t b) {
  int8_t jmp_offset = (int8_t)CPU_get_operand(cpu);
  if (b) {

    // TODO irq stuff

    // page cross ...
    cpu->state.page_cross = is_page_crossed(cpu->state.PC, jmp_offset);

    cpu->state.PC += jmp_offset;
  }
};

void CPU_set_register(CPU_6502 *cpu, uint8_t *reg, uint8_t val) {
  CPU_clear_status_flags(cpu, CPU_STATUS_ZERO | CPU_STATUS_NEGATIVE);
  if (val == 0) {
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  } else if (val & 0x80) {
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);
  }
  *reg = val;
};
//######################################################
//    INSTRUCTIONS
// void* (CPU_6502 *cpu, CPU_addr_mode mode){return "";};

void *BPL(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, !CPU_get_status_flag(cpu, CPU_STATUS_NEGATIVE));
  return "BPL";
};

void *BMI(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, CPU_get_status_flag(cpu, CPU_STATUS_NEGATIVE));
  return "BMI";
};

void *BCC(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, !CPU_get_status_flag(cpu, CPU_STATUS_CARRY));
  return "BCC";
};

void *BCS(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, CPU_get_status_flag(cpu, CPU_STATUS_CARRY));
  return "BCS";
};

void *BVC(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, CPU_get_status_flag(cpu, CPU_STATUS_OVERFLOW));
  return "BVC";
};

void *BVS(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, !CPU_get_status_flag(cpu, CPU_STATUS_OVERFLOW));
  return "BVS";
};

void *BNE(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, !CPU_get_status_flag(cpu, CPU_STATUS_ZERO));
  return "BNE";
};

void *BEQ(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_branch_relative(cpu, CPU_get_status_flag(cpu, CPU_STATUS_CARRY));
  return "BEQ";
};

void *LDY(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.Y, cpu->state.operand);
  return "LDY";
};

void *LDA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A, cpu->state.operand);
  return "LDA";
};

void *LDX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.X, cpu->state.operand);
  return "LDX";
};

void *CLC(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY);
  return "CLC";
};

void *CLI(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_INTERUPT_DISABLE);
  return "CLI";
};

void *CLV(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_OVERFLOW);
  return "CLV";
};

void *CLD(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_DECIMAL);
  return "CLD";
};

void *SEC(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  return "SEC";
};

void *SEI(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_status_flags(cpu, CPU_STATUS_INTERUPT_DISABLE);
  return "SEI";
};

void *SED(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_status_flags(cpu, CPU_STATUS_DECIMAL);
  return "SED";
};

void *NOP(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_get_operand(cpu);
  return "NOP";
};

void *INY(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.Y, cpu->state.Y + 1);
  return "INY";
};

void *INX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.X, cpu->state.X + 1);
  return "INX";
};

void *DEX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.X, cpu->state.X - 1);
  return "DEX";
};

void *DEY(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.Y, cpu->state.Y - 1);
  return "DEY";
};

void *TAY(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.Y, cpu->state.A);
  return "TAY";
};

void *TAX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.X, cpu->state.A);
  return "TAX";
};

void *TSX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.X, cpu->state.SP);
  return "TSX";
};

void *TXA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A, cpu->state.X);
  return "TXA";
};

void *TYA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A, cpu->state.Y);
  return "TYA";
};

void *TXS(CPU_6502 *cpu, CPU_addr_mode mode) {
  cpu->state.SP = cpu->state.X;
  return "TXS";
};

void *AND(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A, cpu->state.A & cpu->state.operand);
  return "AND";
};

void *EOR(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A, cpu->state.A ^ cpu->state.operand);
  return "EOR";
};

void *ORA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A, cpu->state.A | cpu->state.operand);
  return "ORA";
};

void *ADC(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = cpu->state.operand;
  uint16_t res = (uint16_t)cpu->state.A + (uint16_t)val;
  res += CPU_get_status_flag(cpu, CPU_STATUS_CARRY) ? 1 : 0;
  CPU_clear_status_flags(cpu, CPU_STATUS_ZERO | CPU_STATUS_CARRY |
                                  CPU_STATUS_NEGATIVE | CPU_STATUS_OVERFLOW);
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  if ((~cpu->state.A ^ val) & (cpu->state.A ^ res) & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_OVERFLOW);
  if (res > 0xff)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  CPU_set_register(cpu, &cpu->state.A, (uint8_t)res);
  return "ADC";
};

void *SBC(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = cpu->state.operand ^ 0xff;
  uint16_t res = (uint16_t)cpu->state.A + (uint16_t)val;
  res += CPU_get_status_flag(cpu, CPU_STATUS_CARRY) ? 1 : 0;
  CPU_clear_status_flags(cpu, CPU_STATUS_ZERO | CPU_STATUS_CARRY |
                                  CPU_STATUS_NEGATIVE | CPU_STATUS_OVERFLOW);
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  if ((cpu->state.A ^ val) & (cpu->state.A ^ res) & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_OVERFLOW);
  if (res > 0xff)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  CPU_set_register(cpu, &cpu->state.A, (uint8_t)res);
  return "SBC";
};

void *CPX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);

  if (cpu->state.X >= cpu->state.operand)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  if (cpu->state.X == cpu->state.operand)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  if (((cpu->state.X - cpu->state.operand) & 0x80) == 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  return "CPX";
};

void *CPA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);

  if (cpu->state.A >= cpu->state.operand)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  if (cpu->state.A == cpu->state.operand)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  if (((cpu->state.A - cpu->state.operand) & 0x80) == 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  return "CPA";
};

void *CPY(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);

  if (cpu->state.Y >= cpu->state.operand)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  if (cpu->state.Y == cpu->state.operand)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  if (((cpu->state.Y - cpu->state.operand) & 0x80) == 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  return "CPY";
};

void *INC(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  CPU_clear_status_flags(cpu, CPU_STATUS_NEGATIVE | CPU_STATUS_ZERO);
  uint8_t val = CPU_read_memory(cpu, adr);
  val++;

  if (val == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_write_memory(cpu, adr, val);
  return "INC";
};

void *DEC(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  CPU_clear_status_flags(cpu, CPU_STATUS_NEGATIVE | CPU_STATUS_ZERO);
  uint8_t val = CPU_read_memory(cpu, adr);
  val--;

  if (val == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_write_memory(cpu, adr, val);
  return "DEC";
};

void *BIT(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = cpu->state.operand;
  CPU_clear_status_flags(cpu, CPU_STATUS_ZERO | CPU_STATUS_OVERFLOW |
                                  CPU_STATUS_NEGATIVE);
  if ((cpu->state.A & val) == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  if (val & 0x40)
    CPU_set_status_flags(cpu, CPU_STATUS_OVERFLOW);
  if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);
  return "BIT";
};

void *STA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_write_memory(cpu, cpu->state.operand, cpu->state.A);
  return "STA";
};

void *STX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_write_memory(cpu, cpu->state.operand, cpu->state.X);
  return "STX";
};

void *STY(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_write_memory(cpu, cpu->state.operand, cpu->state.Y);
  return "STY";
};

void *PHA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_stack_push(cpu, cpu->state.A);
  return "PHA";
};

void *PLA(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A, CPU_stack_pop(cpu));
  return "PLA";
};

void *PLP(CPU_6502 *cpu, CPU_addr_mode mode) {
  cpu->state.P = CPU_stack_pop(cpu) & 0xcf;
  return "PLP";
};

void *PHP(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_stack_push(cpu, cpu->state.P | CPU_STATUS_BREAK | CPU_STATUS_RESERVED);
  return "PHP";
};

void *JSR(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;

  uint16_t pc = cpu->state.PC - 1;

  CPU_stack_push(cpu, (uint8_t)(pc >> 8));
  CPU_stack_push(cpu, (uint8_t)pc);

  cpu->state.PC = adr;
  return "JSR";
};

void *RTS(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t low = CPU_stack_pop(cpu);
  uint8_t high = CPU_stack_pop(cpu);
  uint16_t adr = low | (high << 8);

  cpu->state.PC = adr + 1;
  return "RTS";
};

void *RTI(CPU_6502 *cpu, CPU_addr_mode mode) {

  cpu->state.P = CPU_stack_pop(cpu);

  uint8_t low = CPU_stack_pop(cpu);
  uint8_t high = CPU_stack_pop(cpu);
  uint16_t adr = low | (high << 8);

  cpu->state.PC = adr;
  return "RTI";
};

void *SLO(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);

  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  val <<= 1;
  if (val == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_set_register(cpu, &cpu->state.A, cpu->state.A | val);

  CPU_write_memory(cpu, cpu->state.operand, val);
  return "SLO";
};

void *SRE(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);

  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (val & 0x01)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  val >>= 1;
  if (val == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_set_register(cpu, &cpu->state.A, cpu->state.A ^ val);

  CPU_write_memory(cpu, cpu->state.operand, val);

  return "SRE";
};

void *RLA(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);

  uint8_t carry = CPU_get_status_flag(cpu, CPU_STATUS_CARRY);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);

  val = (val << 1 | (carry ? 1 : 0));

  if (val == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_set_register(cpu, &cpu->state.A, cpu->state.A & val);

  CPU_write_memory(cpu, cpu->state.operand, val);

  return "RLA";
};

void *RRA(CPU_6502 *cpu, CPU_addr_mode mode) {

  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);

  uint8_t carry = CPU_get_status_flag(cpu, CPU_STATUS_CARRY);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (val & 0x01)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);

  val = (val >> 1 | (carry ? 0x80 : 0));

  if (val == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_set_register(cpu, &cpu->state.A, cpu->state.A & val);

  uint16_t res = (uint16_t)cpu->state.A + (uint16_t)val +
                 (CPU_get_status_flag(cpu, CPU_STATUS_CARRY) ? 1 : 0);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  if (~(cpu->state.A ^ val) & (cpu->state.A ^ res) & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_OVERFLOW);
  if (res > 0xff)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  CPU_set_register(cpu, &cpu->state.A, res);

  CPU_write_memory(cpu, cpu->state.operand, val);

  return "RRA";
};

void *SAX(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_write_memory(cpu, cpu->state.operand, cpu->state.A & cpu->state.X);
  return "SAX";
};

void *LAX(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);
  CPU_set_register(cpu, &cpu->state.A, val);
  CPU_set_register(cpu, &cpu->state.X, val);
  return "LAX";
};

void *DCP(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);
  val--;

  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (cpu->state.A >= val)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  if (cpu->state.A == val)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  if (((cpu->state.A - val) & 0x80) == 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_write_memory(cpu, cpu->state.operand, val);
  return "DCP";
};

void *ISB(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);
  val++;

  uint16_t res = (uint16_t)cpu->state.A + (uint16_t)val +
                 (CPU_get_status_flag(cpu, CPU_STATUS_CARRY) ? 1 : 0);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if ((uint8_t)res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if ((uint8_t)res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  if (~(cpu->state.A ^ val) & (cpu->state.A ^ res) & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_OVERFLOW);
  if (res > 0xff)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  CPU_set_register(cpu, &cpu->state.A, res);

  CPU_write_memory(cpu, cpu->state.operand, val);
  return "ISB";
};

void *AAC(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_set_register(cpu, &cpu->state.A,
                   cpu->state.A & CPU_read_memory(cpu, cpu->state.operand));
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY);
  if (CPU_get_status_flag(cpu, CPU_STATUS_NEGATIVE))
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  return "AAC";
};

void *ASR(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY);
  CPU_set_register(cpu, &cpu->state.A,
                   cpu->state.A & CPU_read_memory(cpu, cpu->state.operand));
  if (cpu->state.A & 1)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  CPU_set_register(cpu, &cpu->state.A, cpu->state.A >> 1);
  return "ASR";
};

void *ARR(CPU_6502 *cpu, CPU_addr_mode mode) {

  CPU_set_register(
      cpu, &cpu->state.A,
      ((cpu->state.A & CPU_read_memory(cpu, cpu->state.operand)) >> 1) |
          (CPU_get_status_flag(cpu, CPU_STATUS_CARRY) ? 0x80 : 0));

  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_OVERFLOW);
  if (cpu->state.A & 0x40)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  if ((CPU_get_status_flag(cpu, CPU_STATUS_CARRY) ? 1 : 0) ^
      ((cpu->state.A >> 5) & 1))
    CPU_set_status_flags(cpu, CPU_STATUS_OVERFLOW);
  return "ARR";
};

void *ATX(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);
  CPU_set_register(cpu, &cpu->state.A, val);
  CPU_set_register(cpu, &cpu->state.X, cpu->state.A);
  CPU_set_register(cpu, &cpu->state.A, cpu->state.A);

  return "ATX";
};

void *AXS(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);
  uint8_t ax = (cpu->state.A & cpu->state.X) - val;

  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY);
  if ((cpu->state.A & cpu->state.X) >= val)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);

  CPU_set_register(cpu, &cpu->state.X, ax);
  return "AXS";
};

void *SYA(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t high = cpu->state.operand >> 8;
  uint8_t low = cpu->state.operand & 0xff;
  uint8_t val = cpu->state.Y & (high + 1);
  // maybe not correct
  CPU_write_memory(cpu, ((cpu->state.Y & (high + 1)) << 8) | low, val);
  return "SYA";
};

void *SXA(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t high = cpu->state.operand >> 8;
  uint8_t low = cpu->state.operand & 0xff;
  uint8_t val = cpu->state.X & (high + 1);
  // maybe not correct
  CPU_write_memory(cpu, ((cpu->state.X & (high + 1)) << 8) | low, val);
  return "SXA";
};

void *HLT(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_read_memory(cpu, cpu->state.operand);
  return "HLT";
};

void *AXA(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  CPU_write_memory(cpu, adr, ((adr >> 8) + 1) & cpu->state.A & cpu->state.X);
  return "AXA";
};

void *UNK(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_read_memory(cpu, cpu->state.operand);
  return "UNK";
};

void *TAS(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  cpu->state.SP = cpu->state.X & cpu->state.A;
  CPU_write_memory(cpu, adr, cpu->state.SP & ((adr >> 8) + 1));
  return "TAS";
};

void *LAS(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t val = CPU_read_memory(cpu, cpu->state.operand);
  CPU_set_register(cpu, &cpu->state.A, val & cpu->state.SP);
  CPU_set_register(cpu, &cpu->state.X, cpu->state.A);
  cpu->state.SP = cpu->state.A;
  return "LAS";
};

void *JMP_Abs(CPU_6502 *cpu, CPU_addr_mode mode) {
  cpu->state.PC = cpu->state.operand;
  return "JMP_Abs";
};

void *JMP_Ind(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t adr = cpu->state.operand;
  if ((adr & 0xff)) {
    uint8_t low = CPU_read_memory(cpu, adr);
    uint8_t high = CPU_read_memory(cpu, adr - 0xff);
    adr = low | (high << 8);
  } else {
    uint8_t low = CPU_read_memory(cpu, adr);
    uint8_t high = CPU_read_memory(cpu, adr + 1);
    adr = low | (high << 8);
  }
  cpu->state.SP = adr;
  return "JMP_Ind";
};

void *ASL_Acc(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (cpu->state.A & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = cpu->state.A << 1;

  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_set_register(cpu, &cpu->state.A, res);

  return "ASL_Acc";
};

void *ASL_Memory(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  uint8_t val = CPU_read_memory(cpu, adr);

  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = val << 1;

  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_write_memory(cpu, adr, res);
  return "ASL_Memory";
};

void *ROR_Acc(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t carry = CPU_get_status_flag(cpu, CPU_STATUS_CARRY);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (cpu->state.A & 1)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = (cpu->state.A >> 1 | (carry ? 0x80 : 0));
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);
  CPU_set_register(cpu, &cpu->state.A, res);
  return "ROR_Acc";
};

void *ROR_Memory(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  uint8_t val = CPU_read_memory(cpu, adr);

  uint8_t carry = CPU_get_status_flag(cpu, CPU_STATUS_CARRY);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (val & 1)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = (val >> 1 | (carry ? 0x80 : 0));
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_write_memory(cpu, adr, res);
  return "ROR_Memory";
};

void *ROL_Acc(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint8_t carry = CPU_get_status_flag(cpu, CPU_STATUS_CARRY);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (cpu->state.A & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = (cpu->state.A << 1 | (carry ? 1 : 0));
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_set_register(cpu, &cpu->state.A, res);
  return "ROL_Acc";
};

void *ROL_Memory(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  uint8_t val = CPU_read_memory(cpu, adr);

  uint8_t carry = CPU_get_status_flag(cpu, CPU_STATUS_CARRY);
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (val & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = (val << 1 | (carry ? 1 : 0));
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_write_memory(cpu, adr, res);
  return "ROL_Memory";
};

void *LSR_Acc(CPU_6502 *cpu, CPU_addr_mode mode) {
  CPU_clear_status_flags(cpu, CPU_STATUS_CARRY | CPU_STATUS_NEGATIVE |
                                  CPU_STATUS_ZERO);
  if (cpu->state.A & 1)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = cpu->state.A >> 1;
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_set_register(cpu, &cpu->state.A, res);
  return "LSR_Acc";
};

void *LSR_Memory(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t adr = cpu->state.operand;
  uint8_t val = CPU_read_memory(cpu, adr);

  if (val & 1)
    CPU_set_status_flags(cpu, CPU_STATUS_CARRY);
  uint8_t res = val >> 1;
  if (res == 0)
    CPU_set_status_flags(cpu, CPU_STATUS_ZERO);
  else if (res & 0x80)
    CPU_set_status_flags(cpu, CPU_STATUS_NEGATIVE);

  CPU_write_memory(cpu, adr, res);

  return "LSR_Memory";
};

void *BRK(CPU_6502 *cpu, CPU_addr_mode mode) {
  uint16_t pc = cpu->state.PC + 1;
  CPU_stack_push(cpu, (uint8_t)(pc >> 8));
  CPU_stack_push(cpu, (uint8_t)pc);

  uint8_t flag = cpu->state.P | CPU_STATUS_BREAK | CPU_STATUS_RESERVED;

  if (cpu->state.need_nmi) {
    cpu->state.need_nmi = 0;
    CPU_stack_push(cpu, flag);
    CPU_set_status_flags(cpu, CPU_STATUS_INTERUPT_DISABLE);
    uint8_t low = CPU_read_memory(cpu, CPU_NMI_VECTOR);
    uint8_t high = CPU_read_memory(cpu, CPU_NMI_VECTOR + 1);
    cpu->state.PC = low | (high << 8);
  } else {
    CPU_stack_push(cpu, flag);
    CPU_set_status_flags(cpu, CPU_STATUS_INTERUPT_DISABLE);
    uint8_t low = CPU_read_memory(cpu, CPU_IRQ_VECTOR);
    uint8_t high = CPU_read_memory(cpu, CPU_IRQ_VECTOR + 1);
    cpu->state.PC = low | (high << 8);
  }

  cpu->state.last_need_nmi = 0; // weird stuff ??
  return "BRK";
};

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
//   0,      1,      2,      3,      4,      5,      6,      7,      8,      9,      A,      B,      C,      D,      E,      F
/*0*/{7,0},  {6,0},  {0,0},  {8,0},  {3,0},  {3,0},  {5,0},  {5,0},  {3,0},  {2,0},  {2,0},  {2,0},  {4,0},  {4,0},  {6,0},  {6,0}, //0
/*1*/{2,1},  {5,1},  {0,0},  {8,0},  {4,0},  {4,0},  {6,0},  {6,0},  {2,0},  {4,1},  {2,0},  {7,0},  {4,1},  {4,1},  {7,0},  {7,0}, //1
/*2*/{6,0},  {6,0},  {0,0},  {8,0},  {3,0},  {3,0},  {5,0},  {5,0},  {4,0},  {2,0},  {2,0},  {2,0},  {4,0},  {4,0},  {6,0},  {6,0}, //2
/*3*/{2,1},  {5,1},  {0,0},  {8,0},  {4,0},  {4,0},  {6,0},  {6,0},  {2,0},  {4,1},  {2,0},  {7,0},  {4,1},  {4,1},  {7,0},  {7,0}, //3
/*4*/{6,0},  {6,0},  {0,0},  {8,0},  {3,0},  {3,0},  {5,0},  {5,0},  {3,0},  {2,0},  {2,0},  {2,0},  {3,0},  {4,0},  {6,0},  {6,0}, //4
/*5*/{2,1},  {5,1},  {0,0},  {8,0},  {4,0},  {4,0},  {6,0},  {6,0},  {2,0},  {4,1},  {2,0},  {7,0},  {4,1},  {4,1},  {7,0},  {7,0}, //5
/*6*/{6,0},  {6,0},  {0,0},  {8,0},  {3,0},  {3,0},  {5,0},  {5,0},  {4,0},  {2,0},  {2,0},  {2,0},  {5,0},  {4,0},  {6,0},  {6,0}, //6
/*7*/{2,1},  {5,1},  {0,0},  {8,0},  {4,0},  {4,0},  {6,0},  {6,0},  {2,0},  {4,1},  {2,0},  {7,0},  {4,1},  {4,1},  {7,0},  {7,0}, //7
/*8*/{2,0},  {6,0},  {2,0},  {6,0},  {3,0},  {3,0},  {3,0},  {3,0},  {2,0},  {2,0},  {2,0},  {2,0},  {4,0},  {4,0},  {4,0},  {4,0}, //8
/*9*/{2,1},  {6,0},  {0,0},  {6,0},  {4,0},  {4,0},  {4,0},  {4,0},  {2,0},  {5,0},  {2,0},  {5,0},  {5,0},  {5,0},  {5,0},  {5,0}, //9
/*A*/{2,0},  {6,0},  {2,0},  {6,0},  {3,0},  {3,0},  {3,0},  {3,0},  {2,0},  {2,0},  {2,0},  {2,0},  {4,0},  {4,0},  {4,0},  {4,0}, //A
/*B*/{2,1},  {5,1},  {0,0},  {5,1},  {4,0},  {4,0},  {4,0},  {4,0},  {2,0},  {4,1},  {2,0},  {4,1},  {4,1},  {4,1},  {4,1},  {4,1}, //B
/*C*/{2,0},  {6,0},  {2,0},  {8,0},  {3,0},  {3,0},  {5,0},  {5,0},  {2,0},  {2,0},  {2,0},  {2,0},  {4,0},  {4,0},  {6,0},  {6,0}, //C
/*D*/{2,1},  {5,1},  {0,0},  {8,0},  {4,0},  {4,0},  {6,0},  {6,0},  {2,0},  {4,1},  {2,0},  {7,0},  {4,1},  {4,1},  {7,0},  {7,0}, //D
/*E*/{2,0},  {6,0},  {2,0},  {8,0},  {3,0},  {3,0},  {5,0},  {5,0},  {2,0},  {2,0},  {2,0},  {2,0},  {4,0},  {4,0},  {6,0},  {6,0}, //E
/*F*/{2,1},  {5,1},  {0,0},  {8,0},  {4,0},  {4,0},  {6,0},  {6,0},  {2,0},  {4,1},  {2,0},  {7,0},  {4,1},  {4,1},  {7,0},  {7,0}  //F
};


char* CPU_op_names[] = {
//      0       1       2       3       4       5       6              7       8       9       A            B      C            D      E               F
        "BRK",  "ORA",  "HLT",  "SLO",  "NOP",  "ORA",  "ASL_Memory",  "SLO",  "PHP",  "ORA",  "ASL_Acc",  "AAC",  "NOP",      "ORA",  "ASL_Memory",  "SLO", //0
        "BPL",  "ORA",  "HLT",  "SLO",  "NOP",  "ORA",  "ASL_Memory",  "SLO",  "CLC",  "ORA",  "NOP",      "SLO",  "NOP",      "ORA",  "ASL_Memory",  "SLO", //1
        "JSR",  "AND",  "HLT",  "RLA",  "BIT",  "AND",  "ROL_Memory",  "RLA",  "PLP",  "AND",  "ROL_Acc",  "AAC",  "BIT",      "AND",  "ROL_Memory",  "RLA", //2
        "BMI",  "AND",  "HLT",  "RLA",  "NOP",  "AND",  "ROL_Memory",  "RLA",  "SEC",  "AND",  "NOP",      "RLA",  "NOP",      "AND",  "ROL_Memory",  "RLA", //3
        "RTI",  "EOR",  "HLT",  "SRE",  "NOP",  "EOR",  "LSR_Memory",  "SRE",  "PHA",  "EOR",  "LSR_Acc",  "ASR",  "JMP_Abs",  "EOR",  "LSR_Memory",  "SRE", //4
        "BVC",  "EOR",  "HLT",  "SRE",  "NOP",  "EOR",  "LSR_Memory",  "SRE",  "CLI",  "EOR",  "NOP",      "SRE",  "NOP",      "EOR",  "LSR_Memory",  "SRE", //5
        "RTS",  "ADC",  "HLT",  "RRA",  "NOP",  "ADC",  "ROR_Memory",  "RRA",  "PLA",  "ADC",  "ROR_Acc",  "ARR",  "JMP_Ind",  "ADC",  "ROR_Memory",  "RRA", //6
        "BVS",  "ADC",  "HLT",  "RRA",  "NOP",  "ADC",  "ROR_Memory",  "RRA",  "SEI",  "ADC",  "NOP",      "RRA",  "NOP",      "ADC",  "ROR_Memory",  "RRA", //7
        "NOP",  "STA",  "NOP",  "SAX",  "STY",  "STA",  "STX",         "SAX",  "DEY",  "NOP",  "TXA",      "UNK",  "STY",      "STA",  "STX",         "SAX", //8
        "BCC",  "STA",  "HLT",  "AXA",  "STY",  "STA",  "STX",         "SAX",  "TYA",  "STA",  "TXS",      "TAS",  "SYA",      "STA",  "SXA",         "AXA", //9
        "LDY",  "LDA",  "LDX",  "LAX",  "LDY",  "LDA",  "LDX",         "LAX",  "TAY",  "LDA",  "TAX",      "ATX",  "LDY",      "LDA",  "LDX",         "LAX", //A
        "BCS",  "LDA",  "HLT",  "LAX",  "LDY",  "LDA",  "LDX",         "LAX",  "CLV",  "LDA",  "TSX",      "LAS",  "LDY",      "LDA",  "LDX",         "LAX", //B
        "CPY",  "CPA",  "NOP",  "DCP",  "CPY",  "CPA",  "DEC",         "DCP",  "INY",  "CPA",  "DEX",      "AXS",  "CPY",      "CPA",  "DEC",         "DCP", //C
        "BNE",  "CPA",  "HLT",  "DCP",  "NOP",  "CPA",  "DEC",         "DCP",  "CLD",  "CPA",  "NOP",      "DCP",  "NOP",      "CPA",  "DEC",         "DCP", //D
        "CPX",  "SBC",  "NOP",  "ISB",  "CPX",  "SBC",  "INC",         "ISB",  "INX",  "SBC",  "NOP",      "SBC",  "CPX",      "SBC",  "INC",         "ISB", //E
        "BEQ",  "SBC",  "HLT",  "ISB",  "NOP",  "SBC",  "INC",         "ISB",  "SED",  "SBC",  "NOP",      "ISB",  "NOP",      "SBC",  "INC",         "ISB"  //F

};
// clang-format on
