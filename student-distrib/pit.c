#ifndef PIT_C
#define PIT_C
#include "include/pit.h"
#include "include/i8259.h"
#include "include/idt.h"
#include "include/x86_desc.h"
#include "include/lib.h"
#include "include/terminal.h"
#include "include/sched.h"
#include "include/task.h"
#include "include/shell.h"
#define  PIT_FLAGS_MASK 0x8E00
volatile uint32_t PIT_tick = 0;
list_head_t* active_task[MAX_NUM_TERMINALS];
volatile uint32_t cur_term_id = 2; // after staring terminals 0-2 you will be on terminal 2
static regs_t regs;
/*
 *  pit_handler
 *   DESCRIPTION: handles a pit interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: switches to next active task
 */
void pit_handler()
{
    regs_t regs_;
    memcpy(&regs_, &regs, sizeof(regs_t));
    // Give time for each terminal to run
    if( (PIT_tick >= WAIT_1 && PIT_tick <END_1) || (PIT_tick >= WAIT_2 && PIT_tick <END_2) || (PIT_tick >= WAIT_3 && PIT_tick <END_3))
      {
        PIT_tick += 1;
        send_eoi(PIT_IRQ);
        return;
      }
    // Initializing terminals 0-2
    if (PIT_tick >= 0 && PIT_tick < WAIT_3) {
      if (sessions[PIT_tick].queue == NULL) {
          send_eoi(PIT_IRQ);
          attach_shell(PIT_tick % MAX_NUM_TERMINALS);
      }
    }
    cli();
    // Set the active task to the head of each session
    active_task[0] = sessions[0].queue->last; // Set active task for session 0 in active task array
    active_task[1] = sessions[1].queue->last; // Set active task for session 1 in active task array
    active_task[2] = sessions[2].queue->last; // Set active task for session 2 in active task array
    cur_term_id = (current_proc->terminal_id + 1) ; // Go to next task
    cur_term_id %= 3; /* want cur_term_id to be in range [0, 2] inclusive. */
    if (active_task[cur_term_id] != NULL) {
	switch_task(active_task[cur_term_id]);
    }
    send_eoi(PIT_IRQ);

    sti();
    asm volatile("leave; ret;");
}

/*
 *  init_pit
 *   DESCRIPTION: initializes the PIT and fills the IDT table for PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes the PIT
 */
void init_pit()
{
    uint32_t cliflags;
    cli_and_save(cliflags);
    unsigned short segSize = KERNEL_CS; // The segment is KERNEL_CS bc this stuff is in kernel
    unsigned short flags = PIT_FLAGS_MASK; // This flag corresponds to a Interrupt gate with 0 DPL,

    fill_interrupt(PIT_INTERRUPT_VEC,(uint32_t*)pit_linkage,segSize,flags);

    outb(PIT_INIT_CMD,PIT_CMD_PORT);
    outb((RELOAD_VALUE & 0xFF), PIT_LO_HI_PORT); // Need bottom 8 bits
    outb((RELOAD_VALUE & 0xFF >> 8), PIT_LO_HI_PORT); // Need top 9 bits
    enable_irq(PIT_IRQ);
    restore_flags(cliflags);
    return;
}
#endif
