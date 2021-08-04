#include "../src/CPU_6502.h"
#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/redirect.h>

Test(cpu_instruction_test, nestest_log) {

  mycpu_log = fopen("test/mycpu.log","w+");
  FILE *expected = fopen("test/small_nestest.log","r");
  cr_assert(mycpu_log!=NULL,"test/mycpu.log file error");
  cr_assert(expected!=NULL,"test/small_nestest.log file error");
  CPU_6502 cpu;
  CPU_init(&cpu);

  NES_ROM rom;
  ROM_load_from_disc("test/nestest.nes", &rom);

  // setup state to run all the tests in the rom
  cpu.state.PC=0xc000;
  cpu.state.P=0x24;
  cpu.state.cycles_accumulated+=7;
  CPU_load_to_memory(&cpu, &rom.data[16], 0x4000, 0xc000);
  CPU_load_to_memory(&cpu, &rom.data[16], 0x4000, 0x8000);

  for (int i = 0; i < 8991; ++i){
     CPU_exec(&cpu);
  }

  fflush(mycpu_log);
  fseek(mycpu_log, 0, SEEK_SET);
  cr_expect_file_contents_eq(mycpu_log, expected, "test/mycpu.log does not match small_nestest.log");

  fclose(mycpu_log);
  fclose(expected);
}