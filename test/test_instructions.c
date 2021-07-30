#include "../src/CPU_6502.h"
#include <stdio.h>

#include <criterion/criterion.h>

Test(cpu_instruction_test, LDX) {

  CPU_6502 cpu;

  CPU_init(&cpu);
  const uint8_t LDX_imm = 0xa2;
  uint8_t put_to_X = 0xff;
  uint8_t test_LDX_dat[] = { LDX_imm, put_to_X, 0xBE, 0xEF };
  CPU_load_to_memory(&cpu, test_LDX_dat, 4);

  //CPU_print_state(&cpu, stdout);
  CPU_exec(&cpu);

  cr_expect(cpu.state.X == put_to_X,
            "LDX Immidiate: wrong value in X register.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_ZERO) == false,
            "LDX Immidiate: CPU status zero not correct.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_NEGATIVE) != false,
            "LDX Immidiate: CPU status negative not correct.");

  
  CPU_init(&cpu);
  put_to_X = 0x0;
  test_LDX_dat[1] = put_to_X;
  CPU_load_to_memory(&cpu, test_LDX_dat, 4);
  CPU_exec(&cpu);

  cr_expect(cpu.state.X == put_to_X,
            "LDX Immidiate: wrong value in X register.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_ZERO) != false,
            "LDX Immidiate: CPU status zero not correct.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_NEGATIVE) == false,
            "LDX Immidiate: CPU status negative not correct.");
}


Test(cpu_instruction_test, LDA) {

  CPU_6502 cpu;

  CPU_init(&cpu);
  const uint8_t LDA_imm = 0xa9;
  uint8_t put_to_A = 0xff;
  uint8_t test_LDA_dat[] = { LDA_imm, put_to_A, 0xBE, 0xEF };
  CPU_load_to_memory(&cpu, test_LDA_dat, 4);
  CPU_exec(&cpu);

  cr_expect(cpu.state.A == put_to_A,
            "LDA Immidiate: wrong value in A register.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_ZERO) == false,
            "LDA Immidiate: CPU status zero not correct.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_NEGATIVE) != false,
            "LDA Immidiate: CPU status negative not correct.");

  CPU_init(&cpu);
  put_to_A = 0x0;
  test_LDA_dat[1] = put_to_A;
  CPU_load_to_memory(&cpu, test_LDA_dat, 4);
  CPU_exec(&cpu);

  cr_expect(cpu.state.A == put_to_A,
            "LDA Immidiate: wrong value in A register.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_ZERO) != false,
            "LDA Immidiate: CPU status zero not correct.");
  cr_expect(CPU_get_status_flag(&cpu, CPU_STATUS_NEGATIVE) == false,
            "LDA Immidiate: CPU status negative not correct.");
}