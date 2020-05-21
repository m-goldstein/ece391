#ifndef TESTS_H
#define TESTS_H

#include "types.h"

// test launcher
void launch_tests();

extern void invalid_opcode();
extern void sys_call(uint32_t num);

#endif /* TESTS_H */
