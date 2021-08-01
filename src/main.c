#include "../src/CPU_6502.h"
#include <stdio.h>

int main(int argc, char **argv) {

  CPU_6502 cpu;
  CPU_init(&cpu);

  /* for (int i = 0; i < 256; ++i) {
    CPU_exec_instruction(&cpu, i);
    if(i%16==0)printf("\n");
  printf("%s:%d+%d| ", CPU_op_names[i],CPU_op_cycles_table[i].cycles,
         CPU_op_cycles_table[i].cross);
  } */
  cpu.state.A = 0xff;
  CPU_exec_instruction(&cpu, 0xaa);
  CPU_print_state(&cpu, stdout);
  
}