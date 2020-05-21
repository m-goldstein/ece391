#ifndef IDT_H
#define IDT_H
#include "types.h"
// There are 32 exceptions
#define NUM_OF_EXCEPT 32
#define DIE_BY_EXECPT 256

#define KEYBOARD_INTERRUPT_VEC 0x21
#define PIT_INTERRUPT_VEC      0x20

/* assign each sys call number to an integer value */
enum sys_calls {
     __HALT       = 1,
     __EXECUTE    = 2,
     __READ       = 3,
     __WRITE      = 4,
     __OPEN       = 5,
     __CLOSE      = 6,
     __GETARGS    = 7,
     __VIDMAP     = 8,
     __SETHANDLER = 9,
     __SIGRETURN  = 10
};
extern uint32_t exception_flag;
// Assembly linkages of all the exceptions/first 32 interrupts
extern void idt0();
extern void idt1();
extern void idt2();
extern void idt3();
extern void idt4();
extern void idt5();
extern void idt6();
extern void idt7();
extern void idt8();
extern void idt9();
extern void idt10();
extern void idt11();
extern void idt12();
extern void idt13();
extern void idt14();
extern void idt15();
extern void idt16();
extern void idt17();
extern void idt18();
extern void idt19();
extern void idt20();
extern void idt21();
extern void idt22();
extern void idt23();
extern void idt24();
extern void idt25();
extern void idt26();
extern void idt27();
extern void idt28();
extern void idt29();
extern void idt30();
extern void idt31();
extern int idt128();

/*initializes the idt*/
extern void init_idt();

/*handles the exception*/
extern void exception_handler(int num);

/*handles a sys_call*/
extern int sys_call_handler(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/*Fills and IDT entry*/
extern void fill_interrupt(int num, uint32_t* offset, uint16_t seg, uint16_t flags);

extern void init_sys_call();




#endif
