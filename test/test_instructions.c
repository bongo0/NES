#include "../src/CPU_6502.h"
#include <stdio.h>

#include <criterion/criterion.h>

Test(cpu_instruction_test, LDX) {

  CPU_6502 cpu;
  CPU_init(&cpu);

  const uint8_t LDX_imm = 0xa2;
  uint8_t put_to_X = 0xff;

  cpu.ram[cpu.state.PC] = LDX_imm;
  cpu.ram[cpu.state.PC + 1] = put_to_X;
  cpu.ram[cpu.state.PC + 2] = 0x00;
  cpu.ram[cpu.state.PC + 3] = 0x00;

  CPU_exec(&cpu);

  cr_expect(cpu.state.X == put_to_X,
            "LDX Immidiate: wrong value in X register.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_ZERO) == false,
            "LDX Immidiate: CPU status zero not correct.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_NEGATIVE) != false,
            "LDX Immidiate: CPU status negative not correct.");

  CPU_init(&cpu);
  put_to_X = 0x0;
  cpu.ram[cpu.state.PC] = LDX_imm;
  cpu.ram[cpu.state.PC + 1] = put_to_X;
  cpu.ram[cpu.state.PC + 2] = 0x00;
  cpu.ram[cpu.state.PC + 3] = 0x00;
  CPU_exec(&cpu);

  cr_expect(cpu.state.X == put_to_X,
            "LDX Immidiate: wrong value in X register.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_ZERO) != false,
            "LDX Immidiate: CPU status zero not correct.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_NEGATIVE) == false,
            "LDX Immidiate: CPU status negative not correct.");
}