#ifndef IDT_C
#define IDT_C

#include "include/idt.h"
#include "include/lib.h"
#include "include/x86_desc.h"
#include "include/i8259.h"
#include "include/keyboard.h"
#include "include/sys_call.h"
#include "include/killscreen.h"
#include "include/sys_call.h"
#include "include/task.h"
// Pointers to all the excepion functions
// Externed from the idtasm.S
uint32_t exception_flag = 0;
static uint32_t interrupt_pointers[] =
{
	(uint32_t)idt0, (uint32_t)idt1, (uint32_t)idt2, (uint32_t)idt3, (uint32_t)idt4, (uint32_t)idt5, (uint32_t)idt6, (uint32_t)idt7, (uint32_t)idt8, (uint32_t)idt9,
	(uint32_t)idt10, (uint32_t)idt11, (uint32_t)idt12, (uint32_t)idt13, (uint32_t)idt14, (uint32_t)idt15, (uint32_t)idt16, (uint32_t)idt17, (uint32_t)idt18, (uint32_t)idt19,
	(uint32_t)idt20, (uint32_t)idt21, (uint32_t)idt22, (uint32_t)idt23, (uint32_t)idt24, (uint32_t)idt25, (uint32_t)idt26, (uint32_t)idt27, (uint32_t)idt28, (uint32_t)idt29,
	(uint32_t)idt30, (uint32_t)idt31
};

// All the messages that are printed out when an exception is seen
static char * exceptionMsg[] = {
  "Exception 0: Divide Error\n",
  "Exception 1: Debug Exception\n",
  "Exception 2: NMI Interrupt\n",
  "Exception 3: Breakpoint Exception\n",
  "Exception 4: Overflow Exception\n",
  "Exception 5: BOUND Range Exceeded Exception\n",
  "Exception 6: Invalid Opcode Exception\n",
  "Exception 7: Device Not Available Exception\n",
  "Exception 8: Double Fault Exception\n",
  "Exception 9: Coprocessor Segment Overrun\n",
  "Exception 10: Invalid TSS Exception\n",
  "Exception 11: Segment Not Present\n",
  "Exception 12: Stack Fault Exception\n",
  "Exception 13: General Protection Exception\n",
  "Exception 14: Page-Fault Exception\n",
  "Exception 15: Intel Reserved COVID19\n",
  "Exception 16: x87 FPU Floating-Point Error\n",
  "Exception 17: Alignment Check Exception\n",
  "Exception 18: Machine-Check Exception\n",
  "Exception 19: SIMD Floating-Point Exception\n"
  "Exception 20: COVD19\n",
  "Exception 21: UIUC\n",
  "Exception 22: User Program Exception\n",
  "Exception 23: \n",
  "Exception 24: \n",
  "Exception 25: \n",
  "Exception 26: \n",
  "Exception 27: \n",
  "Exception 28: \n",
  "Exception 29: \n",
  "Exception 30: \n",
  "Exception 31: \n"
};
/*
 * fill_interrupt
 *   DESCRIPTION: Helper function that fills out one of the 256 entries in the
 *                in the IDT
 *   INPUTS: num - the idt entry you're filling out
 *           offset - the function pointer that you're interrupt will go to
 *           seg - segment selector number (should be KERNEL_CS)
 *           flags - the flags that determine DPL, P, D, and what type of gate
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: fills out an entry in the IDT
 */
void fill_interrupt(int num, uint32_t* offset, uint16_t seg, uint16_t flags)
{
  idt[num].offset_15_00 = (uint32_t)(offset) & 0xFFFF; // need bottom 16 bits
  //idt[num].seg_selector = seg;
  idt[num].reserved4 = flags & 0xFF; // need the bottom 8 bits
  idt[num].reserved3 = (flags & 0x100) >> 8; // need 9th bit
  idt[num].reserved2 = (flags & 0x200) >> 9; // need 10th bit
  idt[num].reserved1 = (flags & 0x400) >> 10; // need 11th bit
  idt[num].size = (flags & 0x800) >> 11; // need 12th bit
  idt[num].reserved0 = (flags & 0x1000) >> 12; // need 13th bit
  idt[num].dpl = (flags & 0x6000) >> 13; // need 14&15th bit
  idt[num].seg_selector = seg;
  idt[num].present = (flags & 0x8000) >> 15; // need 16th bit
  idt[num].offset_31_16 = ((uint32_t)(offset) & 0xFFFF0000) >> 16; // need top 16 bits of offeset
  return;
}

/*
 * init_idt
 *   DESCRIPTION: initializes the IDT with exceptions
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes the IDT with exceptions
 */
void init_idt()
{
  unsigned short segSize = KERNEL_CS; // The segment is KERNEL_CS bc this stuff is in kernel
  unsigned short flags = 0x8E00; // This flag corresponds to a interrupt gate with 0 DPL,
                                 // present flag to 1, and gate size 1 (32-bits)
  int i;
  for(i =0; i<NUM_OF_EXCEPT;i++)
    fill_interrupt(i,(uint32_t*)interrupt_pointers[i],segSize,flags);

	init_sys_call();
  // Load the idt
  lidt(idt_desc_ptr);
  return;
}

/*
 * exception_handler
 *   DESCRIPTION: handles an exception given a number
 *   INPUTS: num - the exception number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: currently just prints a message of what the exception is
 */
void exception_handler(int num)
{
  vga_printf((int8_t *)exceptionMsg[num]);

	if(curr_pid == 0) // If the execption happens in the kernel stop and show kill screen
	{
		show_kill_screen();
		while(1);
	}
	//Return to shell/parent program if exception happens in user program
	asm volatile("movl %0, %%ebx"::"r"(DIE_BY_EXECPT));
	exception_flag = 1;
	kernel_halt();
  return;
}


#endif
