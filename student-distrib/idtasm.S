# vim:ts=4 noexpandtab

#define ASM   1

.text

.globl idt0, idt1, idt2, idt3, idt4, idt5, idt6, idt7, idt8, idt9
.globl idt10, idt11, idt12, idt13, idt14, idt15, idt16, idt17, idt18, idt19
.globl idt20, idt21, idt22, idt23, idt24, idt25, idt26, idt27, idt28, idt29
.globl idt30, idt31, idt128

.align 4

/*
  This assembly file contain all pointers to functions needed by the
  first 32 exceptions
*/


idt0:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $0
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %ecx
  popl %eax

idt1:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $1
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt2:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $2
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt3:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $3
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt4:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $4
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt5:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $5
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt6:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $6
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt7:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $7
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt8:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $8
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt9:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $9
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt10:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $10
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %ecx
  popl %eax

idt11:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $11
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt12:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $12
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt13:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $13
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt14:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $14
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt15:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $15
  call exception_handler #assertion_failure
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt16:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $16
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt17:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $17
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt18:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $18
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt19:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $19
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt20:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $0
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %ecx
  popl %eax

idt21:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $1
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt22:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $2
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt23:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $3
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt24:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $4
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt25:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $5
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt26:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $6
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt27:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $7
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt28:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $8
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt29:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $9
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

idt30:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $0
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %ecx
  popl %eax

idt31:
  #caller saved registers
  pushl %eax
  pushl %ecx
  pushl %edx

  pushl $1
  call exception_handler
  addl $4, %esp # clear the stack

  popl %edx
  popl %eax
  popl %ecx

/*
idt128:
  #caller saved registers
  pushl %ebp
  movl  %esp, %ebp

  pushl %eax
  pushl %ecx
  pushl %edx
  pushl %ebx

  pushl %edx
  pushl %ecx
  pushl %ebx
  pushl %eax
  call sys_call_handler
  addl $16, %esp # clear the stack

  popl %ebx
  popl %edx
  popl %ecx
  popl %eax

  leave
  iret #or rti?
  */
