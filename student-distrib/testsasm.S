# vim:ts=4 noexpandtab

#define ASM   1
.globl invalid_opcode, sys_call

invalid_opcode:
  movl %cr5,%eax
  ret

sys_call:
  pushl %ebp
  movl %esp, %ebp
  movl 8(%ebp),%eax
  int $0x80
  leave
  ret
