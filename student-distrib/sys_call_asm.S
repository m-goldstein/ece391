.data					# section declaration
        BAD_CALL      = -1
        MAX_SYS_CALL  = 9
        SYS_CALL_VEC  = 128
        HALT          = 1
        EXECUTE       = 2
        READ          = 3
        WRITE         = 4
        OPEN          = 5
        CLOSE         = 6
        GETARGS       = 7
        VIDMAP        = 8
        EAX_OFFSET    = 32 # offset to get the eax value back from pop eax
.text

.globl sys_call_vector
.globl execute
.globl halt
.globl open
.globl close
.globl read
.globl write
.globl getargs
.globl vidmap
.align 4

# interrupt vector for sys calls 0x80/128
sys_call_vector:
  pushl %eax
  pushal
  cmpl $0, %eax
  jle bad_sys_call
  cmpl $MAX_SYS_CALL,%eax
  jb no_bad_sys_call
  movl $BAD_CALL,EAX_OFFSET(%esp)
  jmp resume_usr_space

no_bad_sys_call:
  call *sys_jump_table(,%eax,4) # 32-bit pointers so 4*8 = 32 (4 memory locations)
  movl %eax,EAX_OFFSET(%esp)
  jmp resume_usr_space

bad_sys_call:
  movl $BAD_CALL,EAX_OFFSET(%esp)

resume_usr_space:
  popal
  popl %eax
  iret

sys_jump_table:
  .long 0, kernel_halt, kernel_execute, kernel_read, kernel_write, kernel_open, kernel_close, kernel_getargs, kernel_vidmap, kernel_set_handler, kernel_sigreturn
/*
 * halt
 *   DESCRIPTION: terminates a process
 *   INPUTS: status - !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
halt:
  pushl %ebp
  movl  %esp, %ebp

  #Callee Saved registers
  pushl %ebx
  pushl %esi
  pushl %edi

  #move arguments into the right registers
  xorl %ebx,%ebx
  movl 8(%ebp),%ebx # (uint8_t)status argument
  movl $HALT, %eax # sys call num HALT
  int $SYS_CALL_VEC

  leave
  ret

/*
 * execute
 *   DESCRIPTION: attempts to load and execute a new program
 *   INPUTS: command - !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
execute:
  pushl %ebp
  movl  %esp, %ebp

  #Callee Saved registers
  pushl %ebx
  pushl %esi
  pushl %edi

  #move arguments into the right registers
  xorl %ebx,%ebx
  movl 8(%ebp),  %ebx # (uint8_t*)status Command
  movl 12(%ebp), %ecx
  movl 16(%ebp), %edx
  movl $EXECUTE, %eax # sys call num EXECUTE
  int $SYS_CALL_VEC

  leave
  ret

/*
 * read
 *   DESCRIPTION: reads data from the keyboard, file, device, or directory
 *   INPUTS: fd - the device
 *           buf -
 *           nbytes -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
read:
  pushl %ebp
  movl  %esp, %ebp

  #Callee Saved registers
  pushl %ebx
  pushl %esi
  pushl %edi

  #move arguments into the right registers
  movl 8(%ebp), %ebx # (int32_t)fd argument
  movl 12(%ebp),%ecx # (void*)buf argument
  movl 16(%ebp),%edx # (int32_t)nbytes argument
  movl $READ,   %eax # sys call num READ
  int $SYS_CALL_VEC

  leave
  ret

/*
 * write
 *   DESCRIPTION: writes to terminal or a device
 *   INPUTS: fd - the device
 *           buf -
 *           nbytes -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
write:
  pushl %ebp
  movl  %esp, %ebp

  #Callee Saved registers
  pushl %ebx
  pushl %esi
  pushl %edi

  #move arguments into the right registers
  movl 8(%ebp), %ebx # (int32_t)fd argument
  movl 12(%ebp),%ecx # (void*)buf argument
  movl 16(%ebp),%edx # (int32_t)nbytes argument
  movl $WRITE,  %eax # sys call num WRITE
  int $SYS_CALL_VEC

  leave
  ret

/*
 * open
 *   DESCRIPTION: provides access to file system
 *   INPUTS: filename -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
open:
  pushl %ebp
  movl  %esp, %ebp

  #Callee Saved registers
  pushl %ebx
  pushl %esi
  pushl %edi

  #move arguments into the right registers
  movl 8(%ebp),%ebx # (uint8_t*)filename argument
  movl $OPEN, %eax # sys call num OPEN
  int $SYS_CALL_VEC

  leave
  ret

/*
 * close
 *   DESCRIPTION: closes a file descriptor
 *   INPUTS: fd -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
close:
  pushl %ebp
  movl  %esp, %ebp

  #Callee Saved registers
  pushl %ebx
  pushl %esi
  pushl %edi

  #move arguments into the right registers
  movl 8(%ebp),%ebx # (int32_t)fd argument
  movl $CLOSE, %eax # sys call num CLOSE
  int $SYS_CALL_VEC

  leave
  ret

/*
 * getargs
 *   DESCRIPTION: Reads program's command line arguments into user-level buffer
 *   INPUTS: buf:
            nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
getargs:
  pushl %ebp
  movl %esp, %ebp

  pushl %ebx
  pushl %esi
  pushl %edi

  movl 8(%ebp), %ebx # (uint8_t *)buf argument
  movl 12(%ebp), %ecx # (uint32_t)nbytes argument
  movl $GETARGS, %eax # sys call getargs
  int $SYS_CALL_VEC

  leave
  ret

/*
 * vidmap
 *   DESCRIPTION: Maps text-mode video memeory into user space
 *   INPUTS: start_screen:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
vidmap:
  pushl %ebp
  movl %esp, %ebp

  pushl %ebx
  pushl %esi
  pushl %edi

  movl 8(%ebp), %ebx # (uintt8_t**) screen_start argument
  movl $VIDMAP, %eax # sys call vidmap
  int $SYS_CALL_VEC

  leave
  ret

