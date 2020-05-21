#ifndef SHELL_C
#define SHELL_C
#include "include/i8259.h"
#include "include/shell.h"
#include "include/x86_desc.h"
#include "include/multiboot.h"
#include "include/fs.h"
#include "include/vga.h"
#include "include/pit.h"
#include "include/sched.h"
#include "include/task.h"
#define KERNEL_PL 0
runqueue_t runqueue;
proc_t* current_proc;
/*
 *  init_shell
 *   DESCRIPTION: initializes one shell
 *   INPUTS: none
 *   OUTPUTS: a void pointer to the process
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes a shell
 */
void* init_shell()
{
    SAVE_ESP(current_proc->kernel_regs); /* save ESP and EBP */
    SAVE_EBP(current_proc->kernel_regs);
    pid_t* parent = get_current_htable_entry(); /* pointer to parent process resources */
    pid_t* shell  = get_next_free_htable_entry(); /* pointer to next process resources */
    curr_pid                   = next_pid; 
    set_entry_by_index(pid_htable, curr_pid); /* set bitmap entry to indicate pid is in use */
    proc_t* shell_pcb          = &shell->pcb; /* pointers introduced for code clarity */
    proc_t* parent_pcb         = &parent->pcb;
    shell_pcb->parent          = parent_pcb; /* establish relationship between pcbs */
    shell_pcb->priority        = INTERACTIVE_PRIO;
    parent_pcb->child          = shell_pcb;
    current_proc               = shell_pcb;
    shell->node                = (list_head_t*)ll_alloc(); /* allocate a listnode for the shell */
    shell->node->pid = curr_pid;
    shell->node->entry = (void*)shell;
    current_queue = &runqueue.active_array->tasks[0]; /* point current queue to first queue entry in active array */
    current_queue->head = shell->node; /* point head and last to same node */
    current_queue->last = shell->node;
    current_queue->ops  = &queue_ops_table; /* point to operations struct */
    return (void*)shell->node;
}
/*
 *  attach_shell
 *   DESCRIPTION: attaches a shell to a session
 *   INPUTS: none
 *   OUTPUTS: a void pointer to the process
 *   RETURN VALUE: none
 *   SIDE EFFECTS: attaches a shell to a session
 */
void* attach_shell(uint32_t term_id)
{
    cli();
    terminal_session_t* current_term = getCurrentSession(); /* pointer to current tty */
    /* Save context of current session */
    if(term_id == 0)
      save_vga_state_NO_MEMORY(&sessions[term_id].vga);
    else
      save_vga_state_NO_MEMORY(&sessions[term_id-1].vga);
    if(term_id != current_term->id)
    {
      vga_mem_base = (uint32_t)sessions[term_id].vga.screen_start;
    }
    else
      vga_mem_base = VIDEO_START_ADDR;
    restore_vga_state_NO_MEMORY(&(sessions[term_id].vga));

    pid_t* shell               = get_next_free_htable_entry(); /* point to next available process resources */
    curr_pid                   = next_free_pid(); 
    set_entry_by_index(pid_htable, curr_pid); /* mark PID as in use in bitmap */
    next_pid                   = next_free_pid(); /* update next_pid */
    proc_t* shell_pcb          = &shell->pcb;
    shell_pcb->parent          = NULL; /* base shell has no parent */
    shell_pcb->priority        = INTERACTIVE_PRIO;
    shell->node                = (list_head_t*)ll_alloc(); /* allocate memory for a list node */
    shell_pcb->terminal_id     = term_id; /* shell instance associated to a specific console */
    shell->pid                 = curr_pid;
    shell_pcb->pid             = curr_pid; /* assign a PID */
    shell->node->entry         = (void*)shell;
    sessions[term_id].queue    = &runqueue.active_array->tasks[term_id]; /* point queue to correct queue in runqueue structure */
    current_queue              = sessions[term_id].queue;
    current_queue->ops         = &queue_ops_table; /* associate queue to operations structure */
    current_queue->head        = shell->node; /* point head and last nodes of list to head node */
    current_queue->last        = shell->node;
    current_queue->ops->insert_back(shell->node); /* enqueue node to back of list */
    sessions[term_id].queue    = current_queue;
    int32_t num                = next_free_file_table(); /* assign/initialize a file table for the shell */
    if (num == -1)
	return 0;
    shell_pcb->file_table_num  = num;
    set_curr_file_table(shell_pcb->file_table_num);
    shell_pcb->open_files      = curr_file_table;
    init_file_table(shell_pcb->open_files);
    file_table_bitmap |= (1 << shell_pcb->file_table_num);
    fs_t* proc_files = shell_pcb->open_files->files;
    uint32_t phys_addr    = PHYS_ADDR_START(shell_pcb->pid);
    uint32_t virt_addr    = START_OF_USER;
    /* Need to change the physical address since its a new program */
    __map_page_directory(phys_addr, virt_addr, PRESENT | RW_EN | USER_EN | EXTENDED_PAGING);

    shell_pcb->state = TASK_RUNNING;
    /* Place command name in the pcb */
    parse_command_args(shell_pcb, (uint8_t*)"shell");
    int32_t index = 1 + bitscan_reverse(shell_pcb->open_files->bitmap);
    uint8_t current_f_idx = index; /* assign FD# to process */ 
    fs_t* file = &(proc_files[current_f_idx]);		/* pointer to current file */
    file->op_ptr->open(current_f_idx, shell_pcb->command, 0);	/* open the file so it may be used by the proc */
    /* bookkeeping variables to handle the memcpy */
    uint32_t length = file->inode->length;

    // Copy over the code to the right place in virual memory
    file->op_ptr->read(current_f_idx, (uint8_t*)USER_CODE_LOAD_ADDR, length);
    file->op_ptr->close(current_f_idx, shell_pcb->command, 0);
    elf_section_header_table_t* elf_header = (elf_section_header_table_t*)(USER_CODE_LOAD_ADDR);
    shell_pcb->entry_point = elf_header->entry; /* point to executable's entry point */
    shell_pcb->state       = TASK_RUNNING;
    SAVE_ESP(shell_pcb->kernel_regs); /* save ESP */
    SAVE_EBP(shell_pcb->kernel_regs); /* save EBP */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_STACK_ADDR(shell_pcb->pid); // Kernel stack for this pid declared by this macro
    SAVE_REGS(current_proc->kernel_regs);
    curr_pid = current_proc->pid;
    current_proc = shell_pcb;
    //send_eoi(0);
    PIT_tick += 1;
    curr_file_table = &file_table[current_proc->file_table_num];

    runqueue.n_runnable += 1; /* increment number of running processes */
    sti();
    JMP_TO_USER(shell_pcb->entry_point); /* set up stack for IRET and perform context switch */
    return (void*)shell->node;
}
#endif
