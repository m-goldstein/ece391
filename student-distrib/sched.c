#ifndef SCHED_C
#define SCHED_C
#include "include/sched.h"
#include "include/i8259.h"
#include "include/memory.h"
#include "include/terminal.h"
runqueue_t runqueue;
proc_t* current_proc = &(pid_htable.pids[0].pcb); // gets an empty pcb
/*
 *  init_runqueue
 *   DESCRIPTION: initializes the runqueue
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: switches to next active task
 */
void init_runqueue()
{
    runqueue.lock          = 0;              /* default value */
    runqueue.n_runnable    = 0;              /*      |        */
    runqueue.n_switches    = 0;              /*      |        */
    runqueue.timestamp     = 0;              /*      |        */
    runqueue.current_pcb  = NULL;            /*      |        */
    runqueue.idle_pcb     = NULL;            /*      |        */
    runqueue.active_array  = NULL;           /*      |        */
    runqueue.expired_array = NULL;           /*      |        */
    runqueue.first_array.bitmap		= 0; /*      |        */
    runqueue.second_array.bitmap	= 0; /*      V        */
    runqueue.active_array		= &runqueue.first_array; /* active array points to first array */
    runqueue.active_array->tasks->ops	= &queue_ops_table; /* queue operations are set */
    runqueue.expired_array		= &runqueue.second_array; /* expired array points to second array */
    runqueue.expired_array->tasks->ops	= &queue_ops_table; /* queue operations set */
    queue_t* active			= runqueue.active_array->tasks;
    queue_t* expired			= runqueue.expired_array->tasks;
    list_head_t* start			= (list_head_t*)ll_alloc(); /* allocate a node */
    LIST_HEAD(the_head);
    *start				= the_head;
    active->head			= start; /* point head/last nodes in queue to same node */
    expired->head			= start;
    active->last			= start;
    expired->last			= start;

}
proc_t* pcb;
/*
 *  switch_task
 *   DESCRIPTION: switches to the task based on the node
 *   INPUTS: node - contains info of the next task to switch to
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on fail and 0 on success
 *   SIDE EFFECTS: switches to next active task
 */
int32_t switch_task(list_head_t* node)
{
    /* verify input and value of current pcb */
    if (node == NULL || current_proc == NULL)
	return -1;
    /* check that process referred to by node is not null */
    if (node->entry == NULL)
	node = node->next;
    /* cast to a pid_t type and extract the pcb */
    pid_t* context = (pid_t*)node->entry;
    if (context == NULL)
	return -1;
    pcb = &context->pcb;		   /* process to switch to */
    if (pcb == NULL)
	return -1;
    if (pcb->pid == KERNEL_PID) {
	return -1;
    }
    terminal_session_t* current_term = getCurrentSession();
    /* If the terminal ID for this PCB is not the one showing then need to replace
     * the virtual address of the video memory with the one corresponing to the
     * terminal ID of the PCB
     */

    save_vga_state_NO_MEMORY(&(sessions[current_proc->terminal_id].vga));

    restore_vga_state_NO_MEMORY(&(sessions[pcb->terminal_id].vga));

    if(pcb->terminal_id != current_term->id)
    {
	vga_mem_base = (uint32_t)sessions[pcb->terminal_id].vga.screen_start;
    }
    else
    {
      vga_mem_base = VIDEO_START_ADDR;
    }

    // Changing vidmap based on if the process needs it or not
    if (pcb->is_vidmapped == 1 && current_proc->is_vidmapped == 1) {
      if(pcb->terminal_id == current_term->id)
        __map_user_page(VIDEO_START_ADDR, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN);
      else
        __map_user_page( (uint32_t)sessions[pcb->terminal_id].vga.screen_start, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN);

      flush_tlb();
    }
    else if (pcb->is_vidmapped == 0 && current_proc->is_vidmapped == 1) {
        __map_user_page(VIDEO_START_ADDR, USER_VIDEO_MEM_ADDR, 0);
        flush_tlb();
    }
    else if (pcb->is_vidmapped == 1 && current_proc->is_vidmapped == 0) {
      if(pcb->terminal_id == current_term->id)
        __map_user_page(VIDEO_START_ADDR, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN);
      else
        __map_user_page((uint32_t)sessions[pcb->terminal_id].vga.screen_start, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN);

      flush_tlb();
    }
    else if (pcb->is_vidmapped == 0 && current_proc->is_vidmapped == 0) {
	     // Do nothing
    }

    SAVE_ESP(current_proc->kernel_regs); /* save ESP and EBP */
    SAVE_EBP(current_proc->kernel_regs);
    uint32_t phys_addr = PHYS_ADDR_START(pcb->pid);
    uint32_t virt_addr = START_OF_USER;
    current_proc = pcb;                  /* update current proc pointer */
    curr_pid     = pcb->pid;
    runqueue.current_pcb = current_proc;
    __map_page_directory(phys_addr, virt_addr, PRESENT | RW_EN | USER_EN | EXTENDED_PAGING); /* map address space of process to be scheduled */
    flush_tlb();
    tss.ss0   = KERNEL_DS;
    tss.esp0 = KERNEL_STACK_ADDR(pcb->pid); /* update TSS fields */
    RESTORE_ESP(pcb->kernel_regs); /* restore ESP and EBP */
    RESTORE_EBP(pcb->kernel_regs);
    current_proc = pcb;
    curr_pid     = pcb->pid;
    asm volatile("leave;\nret;");
    return 0;
}
#endif
