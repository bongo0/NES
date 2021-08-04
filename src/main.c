#include "../src/CPU_6502.h"
#include "../src/ROM.h"

#include <stdio.h>

int main(int argc, char **argv) {
  char *rom_path;
  if (argc > 1) {
    rom_path = argv[1];
  } else {
    printf("input: [rom path]\n");
    exit(1);
  }
  mycpu_log = fopen("test/mycpu.log", "w+");
  if (mycpu_log == NULL)
    printf("mycpu.log file error");
  CPU_6502 cpu;
  CPU_init(&cpu);

  NES_ROM rom;
  ROM_load_from_disc(rom_path, &rom);

  cpu.state.PC = 0xc000;
  cpu.state.P = 0x24;
  cpu.state.cycles_accumulated += 7;
  CPU_load_to_memory(&cpu, &rom.data[16], 0x4000, 0xc000);
  CPU_load_to_memory(&cpu, &rom.data[16], 0x4000, 0x8000);

  for (int i = 0; i < 8992; ++i) {
    CPU_exec(&cpu);
  }

  ROM_free(&rom);
}