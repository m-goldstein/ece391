
#ifndef PIT_H
#define PIT_H
#include "types.h"
#define PIT_CMD_PORT      0x43
#define PIT_LO_HI_PORT    0x40
#define PIT_IRQ           0x00
#define PIT_INIT_CMD      0x36 // This initialize channels zero, sets it to lo/hi config, and sets it to square wave
#define RELOAD_VALUE      29830 // this sets it to aprox 25ms = (RELOAD_VALUE) * 3000 / 3579545
#define WAIT_1               1
#define END_1               10
#define WAIT_2              11
#define END_2               20
#define WAIT_3              21
#define END_3               30


// 0xE90B // 59659 This sets the effectve time between interrupts to 50ms aprox
// Equation for time between interrupts
// time in ms = reload_value / (3579545 / 3) * 1000
extern void pit_handler();
extern volatile uint32_t PIT_tick;
extern void init_pit();
extern void pit_linkage();
#endif
// 1193182 input signal
// 40 reload
