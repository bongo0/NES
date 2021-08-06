#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#if 0
#define CPU_RAM_SIZE 2048
#else
#define CPU_RAM_SIZE (1<<16)
#endif

typedef struct {
    uint8_t RAM[CPU_RAM_SIZE]; // CPU RAM
} BUS;



#endif //BUS_H