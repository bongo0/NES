#define NESTEST_LOG_COMP
#include "../src/BUS.h"
#include "../src/CPU_6502.h"
#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/redirect.h>

Test(cpu_instruction_test, nestest_log) {
cr_expect_eq(1,2,"rewrite this test with tracelog");
/*   mycpu_log = fopen("test/mycpu.log","w+");
  FILE *expected = fopen("test/small_nestest.log","r");
  cr_assert(mycpu_log!=NULL,"test/mycpu.log file error");
  cr_assert(expected!=NULL,"test/small_nestest.log file error");

  NES_BUS nes;
  NES_ROM rom;
  ROM_load_from_disc("test/nestest.nes", &rom);
  
  BUS_init(&nes,&rom);
  

  // setup state to run all the tests in the rom
  nes.cpu.state.PC=0xc000;
  nes.cpu.state.P=0x24;
  nes.cpu.state.cycles_accumulated+=7;

  for (int i = 0; i < 26553; ++i){ // takes 26553 cycles
     CPU_tick(&nes.cpu);
  }

  fflush(mycpu_log);
  fseek(mycpu_log, 0, SEEK_SET);
  cr_expect_file_contents_eq(mycpu_log, expected, "test/mycpu.log does not match small_nestest.log");

  fclose(mycpu_log);
  fclose(expected); */
}