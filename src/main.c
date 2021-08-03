#include "../src/CPU_6502.h"
#include "../src/ROM.h"

#include <stdio.h>

int main(int argc, char **argv) {

  CPU_6502 cpu;
  CPU_init(&cpu);

  NES_ROM rom;

  ROM_load_from_disc("./test/nestest.nes", &rom);
  cpu.state.PC = 0x0000;
  CPU_load_to_memory(&cpu, &rom.data[16], 2048);

  for (int i = 0; i < 2; ++i) CPU_exec(&cpu);
}