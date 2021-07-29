#include <stdio.h>
#include "../src/CPU_6502.h"



int main(int argc, char **argv){
    
    CPU_6502 cpu;
    CPU_init(&cpu);

    for(int i = 0; i < 256; ++i){
        CPU_exec_instruction(&cpu, i);
    }

}