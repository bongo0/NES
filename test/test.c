#include <stdio.h>
#include "../src/CPU_6502.h"

#include <criterion/criterion.h>


Test(cpu_instruction_test, nop){
    cr_expect(true, "Write a test for No Op!!");
}