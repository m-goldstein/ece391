#ifndef TASK_C
#define TASK_C
#include "include/task.h"
#include "include/sys.h"
#include "include/sys_call.h"
#include "include/multiboot.h"
#include "include/x86_desc.h"
#include "include/memory.h"
#include "include/lib.h"
#include "include/sched.h"
volatile int16_t next_pid = 0; /* next available PID */
volatile int16_t curr_pid = 0; /* current PID        */
pid_htable_t pid_htable;
proc_t* idle;                /* ptr to idle process (PID = 0)   */
proc_t* kern;                /* ptr to kernel process (PID = 0) */
queue_t active_tasks;
queue_t* current_queue;
/*
 *
 * init_idle_task
 *   DESCRIPTION: Initializes the idle task
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Creates the idle task that is always running
 */
void init_idle_task()
{
    current_queue = &runqueue.active_array->tasks[0];
    current_queue->ops  = &queue_ops_table;
    kern = (proc_t*)&pid_htable.pids[KERNEL_PID].pcb;
    set_curr_file_table(0);
    kern->open_files = curr_file_table;
    kern->file_table_num = 0;
    set_entry_by_index(pid_htable, KERNEL_PID);
    kern->entry_point   = PCB_START_ADDR(KERNEL_PID);
    uint32_t phys_addr  = PHYS_ADDR_START(KERNEL_PID);
    kern->pid = KERNEL_PID;
    kern->active = 1;
    kern->terminal_id = 0;
    kern->parent = kern;
    kern->child  = kern;
    SAVE_REGS(kern->kernel_regs);
    kern->state = TASK_RUNNING;
    kern->num_open_files = 0;
    pid_htable.pids[KERNEL_PID].pcb  = *kern;
    kern->kernel_regs.esp = __4MB__;
    kern->kernel_regs.ebp = __4MB__;
    idle = kern;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t)idle->kernel_regs.esp; /* set up first stack */
    tss.esp1 = (uint32_t)idle->kernel_regs.esp + __512KB__; /* set up second stack */
    SAVE_ESP(idle->kernel_regs); /* save ESP */
    SAVE_EBP(idle->kernel_regs); /* save EBP */
    curr_pid = KERNEL_PID;       /* curr_pid = 0 */
    next_pid = 1;
    runqueue.current_pcb = kern;
    runqueue.idle_pcb    = kern;
    runqueue.n_runnable   = 0;
    __map_page_directory(phys_addr, phys_addr, PRESENT | RW_EN | EXTENDED_PAGING);
}
/*
 * mk_proc
 *   DESCRIPTION: Makes a process
 *   INPUTS: command: command
 *           args: arguments
 *   OUTPUTS: 0 on success, -1 for failure
 *   RETURN VALUE: integer
 *   SIDE EFFECTS: Makes a process and sets variables
 */

proc_t* mk_proc(uint8_t* command, uint8_t* args)
{
    next_pid = next_free_pid();   /* get next available pid */
    if (next_pid < 1 || next_pid >= MAX_PIDS)  /* check validity         */
	return NULL;
    pid_t* htable_entry = &pid_htable.pids[next_pid];// get_next_free_htable_entry();
    proc_t*        pcb  = &htable_entry->pcb;
    htable_entry->node = (list_head_t*)ll_alloc();
    htable_entry->node->next = NULL;
    htable_entry->node->prev = NULL;
    htable_entry->node->entry = (void*)htable_entry;
    //curr_pid = next_free_pid();
    pcb->pid = next_pid;
    set_entry_by_index(pid_htable, pcb->pid);
    //next_pid = next_free_pid();
    // Assign the next approprate file table to the pcb
    int32_t num = next_free_file_table();
    if (num == -1)
	num = 0;
    pcb->file_table_num = num;
    set_curr_file_table(pcb->file_table_num);
    pcb->open_files = curr_file_table;
    init_file_table(pcb->open_files);
    file_table_bitmap |= (1 << pcb->file_table_num);
    pcb->active = 1;
    pcb->terminal_id = getCurrentSession()->id;
    switch_terminals(pcb->terminal_id);
    pcb->entry_point = 0; // default value
    pcb->stack_addr  = 0; // default
    pcb->is_vidmapped = 0;		 /* default */
    pcb->num_open_files = 0;
    memcpy((int8_t*)pcb->command, (const int8_t*)command,strlen((const int8_t*)command)+1); // Plus one is for the NULL char
    memcpy((int8_t*)pcb->args, (int8_t*)args,strlen((const int8_t*)args)+1); // Plus one is for the NULL char
    return &pid_htable.pids[next_pid].pcb;
}
/*
 * close_proc
 *   DESCRIPTION: Closes a given process
 *   INPUTS: proc: process
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Clears process and sets to NULL
 */

void close_proc(proc_t* proc)
{
    if (proc == NULL) /* validate input */
	return;
    pid_t* htable_entry = &pid_htable.pids[proc->pid]; /* pointer to table entry containing process resources */
    if (htable_entry == NULL)
    	return;

    ll_free(htable_entry->node); /* reclaim memory allocated to list node */
    set_curr_file_table(proc->file_table_num); /* point current file table to process file table */
    file_table_t* files = curr_file_table;
    if (files != NULL) {
	fs_t* file = files->files;
	int32_t pos;
	for (pos = 0; pos < MAX_NUM_FD; pos++) { /* disassociate file from its resources */
		if (&file[pos] == NULL) {
		    continue;
		}
		fs_t* file = &file[pos];
		file->op_ptr    = NULL;
		file->buffer    = NULL;
		file->sess      = NULL;
		file->inode     = NULL;
		file->inode_num = 0;
		file->f_pos     = 0;
		file->flags     = 0;
		files->bitmap &= ~(1 << pos);
	    }
    }
    files->bitmap        = 0; /* clear file bitmap */
    file_table_bitmap   &= ~(1 << proc->file_table_num); /* clear file table bitmap entry */
    proc->file_table_num = 0; /* clear file table number field */
    clear_entry_by_index(pid_htable, proc->pid);
    proc->open_files     = NULL; /* disassociate pcb from file table instance */
    proc->num_open_files = 0;
    // proc->user_regs      = *((regs_t*)NULL);
    // proc->kernel_regs    = *((regs_t*)NULL);
    proc->entry_point    = 0; /* clear entries */
    proc->pid            = 0; /*      |        */
    proc->active         = 0; /*      V        */
    proc->state          = TASK_STOPPED; /* update state */
    proc->is_vidmapped   = 0; /* clear is_vidmapped flag */
    memset((void*)proc->command, NULL, CMD_NAME_MAX_LEN); /* clear command 
							     and args fields */
    memset((void*)proc->args, NULL, CMD_ARGS_MAX_LEN);

}
/*
 * next_free_pid
 *   DESCRIPTION: Getter function for next_pid
 *   INPUTS: none
 *   OUTPUTS: next_pid
 *   RETURN VALUE: integer
 *   SIDE EFFECTS: Returns next_pid
 */

int16_t next_free_pid()
{
    uint32_t i;
    for (i = next_pid % MAX_PIDS; i < MAX_PIDS; i++) { /* use modulus to assign pids cyclically */
	if ((pid_htable.bitmap & (1 << i)) == 0)
	    return i; /* return next free index */
    }
    return -1;
}
/* parse_file
 *  DESCRIPTION : helper function to parse file name from a command
 *  INPUTS      : file_name -- buffer to store parsed file name.
 *	 	  command   -- buffer to parse file name from.
 *  OUTPUTS     : none
 *  RETURN VALUE: returns length of file_name as an integer or -1 on error
 *  SIDE EFFECTS: file_name argument stores the parsed file name (leading symbols
 *                up to but excluding the first space.
 */
int32_t parse_file(uint8_t* file_name, uint8_t* command)
{
    if (file_name == NULL || command == NULL)
    {
	return -1;
    }
    uint32_t i = 0;
    for (; command[i] != '\n' && command[i] != ' '
	    && command[i] != '\r' && command[i] != '\0' && i < CMD_NAME_MAX_LEN; i++) {
	file_name[i] = command[i];
    }
    file_name[i] = '\0';
    return i;
}
/*
 * parse_command_args
 *   DESCRIPTION: Parses the command and args for a process control block
 *   INPUTS: pcb: Process Control Block
 *           command : command for process
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets new command and args for pcb if it is valid
 */

void parse_command_args(proc_t* pcb, uint8_t* command)
{
    /* validate inputs (check for NULL) */
    if (pcb == NULL || command == NULL)
	return;
    uint32_t index = 0;
    uint32_t parse_space = 0;
    uint32_t pcb_cmd_index = 0;
    uint32_t pcb_args_index = 0;
    int32_t  length = strlen( (int8_t*)command);
    /* check for space or newline or end of string */
    for (index = 0; index < length; index++) {
    	if ((command[index] == '\n') || (command[index] == '\0') || (command[index] == ' ' && parse_space == 1))
    	    break;
      if(command[index] != ' ')
      {
    	 pcb->command[pcb_cmd_index++] = command[index]; /* update command field in PCB */
       parse_space = 1;
      }
    }
    pcb->command[pcb_cmd_index] = '\0'; /* add null terminating charachter to end of string */

    parse_space = 0;

    for (; index < length; index++) {
      if ((command[index] == '\n') || (command[index] == '\0'))
    	    break;
      if(command[index] != ' ')
        parse_space = 1;
      if(parse_space == 1)
      {
    	 pcb->args[pcb_args_index++] = command[index]; /* update args field in PCB */
      }
    }
    pcb->args[pcb_args_index] = '\0';
}
/*
 *   parse_args
 *   DESCRIPTION: Parses the coomand for a process control block
 *   INPUTS: pcb: Process Control Block
 *           command : command for process
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets new command for pcb if it is valid
 */
void parse_args(proc_t* pcb, uint8_t* command)
{
    /* validate inputs (check for NULL) */
    if (pcb == NULL || command == NULL)
	return;
    uint32_t index = 0;
    /* check for space or newline or end of string */
    for (index = 0; index < ustrlen(command); index++) {
	if ( (command[index] == ' ') || (command[index] == '\n') || (command[index] == '\0'))
	    break;
    }
    if (strncmp((int8_t*)command, (int8_t*)"shell", strlen((int8_t*)"shell")) == 0) {
	pcb->args[0] = (uint8_t)'\0';
	return;
    }
    int32_t idx = 0;
    if (ustrlen((uint8_t*)command) > index) {
	for (idx = 0; idx < ustrlen((uint8_t*)command) - index - 1; idx++) {
	    pcb->args[idx] = command[index+idx+1];
	}
    }
    for(;idx<CMD_ARGS_MAX_LEN;idx++)
      pcb->args[idx] = '\0'; /* add null terminating charachter to end of string */
}
/*
 * is_valid_elf_header
 *   DESCRIPTION: Checks if file is executable
 *   INPUTS: elf_dataL the bytes checked to see if it is executable
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success, -1 for failure
 *   SIDE EFFECTS: Checks the four magic numbers that represent an executable
 */

int32_t is_valid_elf_header(uint8_t* elf_data)
{
    read_proc_by_pcb();
    //printf("%x %x %x %x\n", elf_data[0], elf_data[1], elf_data[2], elf_data[3]);
    /* compare elf data to ELF magic words */
    if ( (elf_data[0] == ELF_MAGIC_WORD_0) && (elf_data[1] == ELF_MAGIC_WORD_1) && (elf_data[2] == ELF_MAGIC_WORD_2) && (elf_data[3] == ELF_MAGIC_WORD_3) )
	return P_SUCCESS;
    return P_FAIL;
}
/*
 * read_proc_by_pcb
 *   DESCRIPTION: Reads process from PCB
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: Pointer to a process
 *   SIDE EFFECTS: Returns a process based of the stack and the PCB
 */

void* read_proc_by_pcb()
{
    uint32_t pcb_mask  = PCB_MASK; /* current PCB located by ANDing with high bits of ESP */
    asm volatile("		      \
		    andl %%esp, %0  ; \
		    mov %0, %1      ; \
		 "
		:
		:"a"(pcb_mask), "g"(current_proc)
		:"cc", "memory"
		);
    return (void*)current_proc; /* update and return PCB pointed by current proc */
}
/*
 * read_proc_by_pid
 *   DESCRIPTION: Reads a process from its PID number
 *   INPUTS: pid: process ID
 *   OUTPUTS: none
 *   RETURN VALUE: A pointer to the process
 *   SIDE EFFECTS: Process is returned by its PID
 */

void* read_proc_by_pid(int16_t pid)
{
    /* check for negative input */
    if (CHECK_MSB(pid) == 1)
	return NULL;
    /* point to corresponding proc in processes table */
    proc_t* proc = (proc_t*)&pid_htable.pids[pid].pcb;
    if (proc == NULL) /* check for null */
	return NULL;
    return (void*)proc;
}
/*
 * get_next_free_htable_entry
 *   DESCRIPTION: gets the next free pcb
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: A pointer to the process
 *   SIDE EFFECTS: Process is returned by its PID
 */
pid_t* get_next_free_htable_entry()
{
    //int16_t pid = next_free_pid();
    return &pid_htable.pids[next_pid];
}
/*
 * get_current_htable_entry
 *   DESCRIPTION: gets the current htable entry/pcb
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: A pointer to the process
 *   SIDE EFFECTS: Process is returned by its PID
 */
pid_t* get_current_htable_entry()
{
    pid_t* entry = &pid_htable.pids[curr_pid];
    if (entry == NULL)
	return NULL;
    return entry;
}
#endif
