#ifndef SYS_CALL_C
#define SYS_CALL_C
#include "include/sys_call.h"
#include "include/idt.h"
#include "include/lib.h"
#include "include/multiboot.h"
#include "include/i8259.h"
#include "include/keyboard.h"
#include "include/rtc.h"
#include "include/idt.h"
#include "include/task.h"
#include "include/memory.h"
#include "include/fs.h"
#include "include/directory.h"
#include "include/task.h"
#include "include/sys.h"
#include "include/sched.h"
#include "include/terminal.h"
#include "include/vga.h"
#include "include/task.h"
#define USER_PL 3
#define KERNEL_PL 0
//queue_t* current_queue;
/*
 * init_sys_call
 *   DESCRIPTION: initializes all the system calls
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: puts all the system calls in the IDT
 */
void init_sys_call()
{
  unsigned short segSize = KERNEL_CS; // The segment is KERNEL_CS bc this stuff is in kernel
  unsigned short flags = IDT_FLAGS; // This flag corresponds to a interrupt gate with 0 DPL,
                                // present flag to 1, and gate size 1 (32-bits)
  // uint32_t cpl;
  // asm volatile("mov %%cs, %0":"=r"(cpl)); // Checking current privlage level
  // printf("CPL: %x\n", cpl);
  fill_interrupt(SYS_CALL_VEC,(uint32_t*)sys_call_vector,segSize,flags);
}

static int32_t result;
/*
 * kernel_halt
 *   DESCRIPTION: terminates a process
 *   INPUTS: status - status of the program returned
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: goes back to the previous process
 */
int32_t kernel_halt()
{
  uint8_t status = 0;
  asm("			 \
	  movb %%bl, %0;\
	  "
	  : "=g"(status)
	  : /* no inputs */
	  :"cc", "memory"
     );
  cli(); /* critical section start */

  /* verify that the correct pcb is being referenced */
  pid_t* current_htable_entry = get_current_htable_entry();
  proc_t*	 proc_to_halt = (proc_t*)&current_htable_entry->pcb;
  proc_t*	 proc_to_resume;
  current_queue = sessions[current_proc->terminal_id].queue; /* point to correct queue */
  proc_to_resume   = proc_to_halt->parent; /* point to correct parent pcb */
  /* Clear the keyboard terminal of this process */
  sessions[current_proc->terminal_id].index = 0;

  /* case to handle exiting the \"base shell\" for that terminal */
  if (proc_to_resume == NULL || (proc_to_resume->pid == KERNEL_PID)) {
      pid_t* hpid          = (pid_t*)sessions[proc_to_halt->terminal_id].queue->head->entry;
      current_proc         = &hpid->pcb;
      set_curr_file_table(current_proc->file_table_num);
      runqueue.current_pcb = current_proc;
      curr_pid             = current_proc->pid; 
      JMP_TO_USER(current_proc->entry_point); /* resume current process */
  }
  /* disassociate pcb from its resources */
  close_proc(proc_to_halt);
  __map_user_page(VIDEO_START_ADDR, USER_VIDEO_MEM_ADDR, 0); // Un-map the video memory
  current_queue->ops->remove(current_queue->last); /* remove node from end of queue */
  if(curr_pid >= 1) {
      next_pid = next_free_pid();
  }
  curr_pid = proc_to_resume->pid; /* change curr_pid and point current_proc 
				     to the correct pcb */
  current_proc   = proc_to_resume;
  current_proc->child = NULL;
  set_curr_file_table(current_proc->file_table_num); /* re-establish file table instance */
  if (curr_file_table == NULL) {
      init_file_table(curr_file_table);
  }
  runqueue.current_pcb = current_proc; 
  tss.esp0 =  KERNEL_STACK_ADDR(curr_pid);
  asm volatile("movl %0, %%edx"::"r"((int32_t)status)); // Need to deal with switching stacks so save the var in a reg
  RESTORE_ESP(proc_to_resume->kernel_regs);
  RESTORE_EBP(proc_to_resume->kernel_regs);

  asm volatile("movl %%edx,%0":"=d"(result)::"cc","memory"); /* populate result before returning */
  __map_page_directory(PHYS_ADDR_START(curr_pid), START_OF_USER, PRESENT | RW_EN | USER_EN | EXTENDED_PAGING); /* Return to previous process address space */
  /* handle case where process dies by exceptiom */
  if(exception_flag == 1)
  {
      result = DIE_BY_EXECPT;
      exception_flag = 0;
  }
  flush_tlb();

  // Special ctrl-L is needed if the command is shell
  if(current_proc->terminal_id == current_session)
  {
    if(strncmp( (const int8_t*)current_proc->command , "shell", strlen((int8_t*) current_proc->command)) == 0)
      shell_showing = 1;
    else
      shell_showing = 0;
  }
  /* update runqueue count of runnable processes */
  runqueue.n_runnable -= 1;
  sti();
  return (int32_t)result;
}

/*
 * kernel_execute
 *   DESCRIPTION: attempts to load and execute a new program
 *   INPUTS: command - name of proces to execute
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the page directory entry to the new process
 */

uint8_t kern_cmd_buf[TERMINAL_BUF_SIZE];
uint8_t cmd_buf[TERMINAL_BUF_SIZE];
uint8_t arg_buf[TERMINAL_BUF_SIZE];
uint8_t file_buf[TERMINAL_BUF_SIZE];
proc_t* pcb;
int32_t kernel_execute()
{
    uint8_t* command;
    asm ("                      \
	    movl %%ebx, %0     ;\
	    "
	    : "=g"(command)
	    : /* no inputs */
	    :"cc","memory"
	);
    if(command == NULL)
	     return P_FAIL;
    /* check that we do not try to execute more than a fixed number of processes */
    if(runqueue.n_runnable >= MAX_PROCESSES)
      return -1;
    //vga_printf("Num Processes: %x\n", runqueue.n_runnable);
    cli();
    /* update value for next_pid and establish pointer to next table entry where
     * pcb resources are statically allocated */
    next_pid                    = next_free_pid();
    pid_t* htable_entry         = get_next_free_htable_entry();
    pcb                         = &htable_entry->pcb;
    pid_t* parent               = (pid_t*)getCurrentSession()->queue->last->entry;
    proc_t* parent_proc         = &parent->pcb; /* pointer to parent process */
    uint32_t index              = 0;
    proc_t temp;
    int32_t cmd_len             = parse_file(file_buf, command);
    if (cmd_len == -1)
	return P_FAIL;

    strcpy((int8_t*)cmd_buf, (int8_t*)command); /* copy into kernel space      */
    parse_command_args(&temp, cmd_buf);         /* populate fields in temp pcb */
    // Need to copy from user to kernel space the string so it doesn't disappear
    strcpy((int8_t*)file_buf, (const int8_t*)temp.command);
    strcpy((int8_t*)arg_buf, (const int8_t*)temp.args);
    pcb = mk_proc(file_buf, arg_buf); /* populate entries of pcb to either 
					 defaults or values depending PID */
    curr_pid = pcb->pid;              /* update curr_pid and next_pid     */
    next_pid = next_free_pid();
    pcb->terminal_id = parent_proc->terminal_id; /* associate pcb to a tty session */
    parent_proc->child = pcb; /* establish parent/child relationship among processes */
    pcb->parent = parent_proc;
    pcb->state  = TASK_RUNNING; /* set the state */
    // Check if file exists
    int8_t valid = check_elf((uint8_t*)file_buf);
    if(valid == P_FAIL || cmd_buf == NULL || strlen((const int8_t*)cmd_buf) == 0) 
	// File was not found or not valid P_FAIL == -1
	return P_FAIL;
    uint32_t phys_addr    = PHYS_ADDR_START(pcb->pid); 
    uint32_t virt_addr    = START_OF_USER;
    /* Need to change the physical address since its a new program */

    /* Need to associate/initialize a file table to the process that is separate from the parent */
    if (pcb->open_files == NULL || pcb->open_files == parent_proc->open_files) {
	int32_t num = next_free_file_table();
	if (num == -1)
	    return -1;
	pcb->file_table_num = num;
	set_curr_file_table(pcb->file_table_num);
	pcb->open_files     = curr_file_table;
	init_file_table(pcb->open_files);
	file_table_bitmap |= (1 << pcb->file_table_num);
    }
    fs_t* proc_files = pcb->open_files->files;
    current_queue = getCurrentSession()->queue; /* update current queue pointer */
    current_queue->ops->insert_back(htable_entry->node); /* enqueue entry */
    memcpy((void*)kern_cmd_buf, (void*)cmd_buf, cmd_len);
    //parse_command_args(pcb, kern_cmd_buf);
    index = 1 + bitscan_reverse(pcb->open_files->bitmap); /* calculate free index in array of file structs */
    uint8_t current_f_idx = index;
    fs_t* file = &(proc_files[current_f_idx]);		/* pointer to current file */

    fs_open(current_f_idx, file_buf, 0);	/* open the file so it may be used by the proc */
    /* bookkeeping variables to handle the memcpy */
    __map_page_directory(phys_addr, virt_addr, PRESENT | RW_EN | USER_EN | EXTENDED_PAGING);
    flush_tlb();
    uint32_t length = file->inode->length;

    // Copy over the code to the right place in virual memory
    fs_read(current_f_idx, (uint8_t*)(USER_CODE_LOAD_ADDR), length);
    fs_close(current_f_idx, pcb->command, 0);
    elf_section_header_table_t* elf_header = (elf_section_header_table_t*)(USER_CODE_LOAD_ADDR);
    pcb->entry_point = elf_header->entry; /* point to executable's entry point */


    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_STACK_ADDR(pcb->pid); /* Kernel stack for this pid declared by this macro */
    SAVE_REGS(pcb->kernel_regs); /* save hardware context */
    SAVE_REGS(parent_proc->kernel_regs); /* ^ */
    SAVE_ESP(parent_proc->kernel_regs); /* save ESP */
    SAVE_EBP(parent_proc->kernel_regs); /* save EBP */
    //SAVE_REGS(pcb->kernel_regs);
    current_proc = pcb; /* update current_proc pointer */

    // Special ctrl is needed for the shell process
    if(pcb->terminal_id == current_session)
    {
      if(strncmp( (const int8_t*)pcb->command , "shell", strlen((int8_t*) pcb->command)) == 0)
        shell_showing = 1;
      else
        shell_showing = 0;
    }

    runqueue.current_pcb = current_proc;
    runqueue.n_runnable += 1; /* increment number of runnable processes */
    JMP_TO_USER(pcb->entry_point); /* set up stack for IRET and perform context switch */
    sti();
    return 0;
}

/*
 * kernel_read
 *   DESCRIPTION: reads data from the keyboard, file, device, or directory
 *   INPUTS: fd - the device
 *           buf -
 *           nbytes -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: execute the system call
 */
int32_t kernel_read()
{

    int32_t fd;
    void* buf;
    int32_t nbytes;
    asm ("			    \
	    movl %%ebx, %0         ;\
	    movl %%ecx, %1         ;\
	    movl %%edx, %2         ;\
	    "

	    :"=g"(fd),"=g"(buf),"=g"(nbytes)
	    : /* no inputs */
	    :"cc","memory"
	);
    uint32_t bytes_read;

    if(nbytes < 0 || fd < 0 || buf == NULL || fd >= MAX_NUM_FD) // Can read negative number of values, negative fd, or buff pointer NULL
	return -1;
    pid_t* htable_entry = get_current_htable_entry();
    proc_t* current_pcb  = (proc_t*)&htable_entry->pcb; /* get current process */

    set_curr_file_table(current_pcb->file_table_num);
    //file_table_t* files = curr_file_table;
    //file_table_t* files   = current_proc->open_files;
    file_table_t*   files   = current_pcb->open_files;
    if((files->bitmap & (1 << fd)) == 0) // Checks if the fd is open
	return -1;
    fs_t* files_ptr     = files->files;
    fs_t* file = &files_ptr[fd];
    // Read can only be three things keyboard, file, device or directory
    // stdout should be using terminal max bytes_read can be 128
    // stdin should return -1
    // if a file, you should only copy nbytes into buffer
    // if a device(stdin or rtc), it should just work if open is done correctly
    // Also there is one for directory
    bytes_read = file->op_ptr->read(fd, buf, nbytes);
    return bytes_read;
}

/*
 * kernel_write
 *   DESCRIPTION: writes to terminal or a device
 *   INPUTS: fd - the device
 *           buf -
 *           nbytes -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: execute the write system call
 */
int32_t kernel_write()
{
    int32_t fd;
    void* buf;
    int32_t nbytes;
    asm ("			    \
	    movl %%ebx, %0         ;\
	    movl %%ecx, %1         ;\
	    movl %%edx, %2         ;\
	    "

	    :"=g"(fd),"=g"(buf),"=g"(nbytes)
	    : /* no inputs */
	    :"cc","memory"
	);
    uint32_t pos;
    if(nbytes < 0 || fd < 0 || buf == NULL || fd >= MAX_NUM_FD) // Cant write negative bytes, negative fd, or write to NULL
	return -1;
    cli();
    //curr_file_table = &file_table[current_proc->file_table_num];
    //file_table_t* files = curr_file_table;
    file_table_t* files   = current_proc->open_files;
    if((files->bitmap & (1 << fd)) == 0) // Checks if the fd is open
	return -1;
    fs_t* files_ptr     = files->files; /* pointers used to make code more readable */
    fs_t* file = &files_ptr[fd];
    pos = file->op_ptr->write(fd, (uint8_t*)buf, nbytes);
    sti();
    return pos;
}

/*
 * kernel_open
 *   DESCRIPTION: provides access to file system
 *   INPUTS: filename -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: opens a 'file' and places it in the
 */
int32_t kernel_open()
{

    uint8_t* filename;
    asm ("			    \
	    movl %%ebx, %0         ;\
	    "

	    :"=g"(filename)
	    : /* no inputs */
	    :"cc","memory"
	);

    pid_t* hpid = get_current_htable_entry();
    proc_t* current_pcb = &hpid->pcb;
    dentry_t dentry;

    //set_curr_file_table(current_pcb->file_table_num);
//    file_table_t* files = curr_file_table;

    file_table_t* files = current_pcb->open_files;
    fs_t* files_ptr     = files->files;
    uint32_t index = 0;

    if(files->bitmap == 0xFF || filename == NULL|| strlen((const int8_t*)filename) == 0) // 0xFF means all the spots in the FD is full
	return -1;

    index = bitscan_reverse(files->bitmap);
    index += 1;
    uint32_t valid = read_dentry_by_name((const uint8_t*)filename, &dentry); /* populate dentry */
    if(valid == -1) // File was not found
	return -1;

    fs_t* file = &files_ptr[index];
    files->bitmap |= (1 << index); // Put an entry in the bitmap
    /* Open a device in this case its only RTC */
    if (dentry.file_type == 0 && 0 == strncmp((const int8_t*)filename, "rtc", strlen((const int8_t*)filename))) {
	file->op_ptr = &rtc_ops_table;
        file->op_ptr->open(index, filename, 0);
	current_pcb->priority = REAL_TIME_PRIO;
    }
    else if (dentry.file_type == 1) { /* Means that the 'file' to open is a directory */
	file->op_ptr = &dir_ops_table;
        file->op_ptr->open(index, filename, 0);
	current_pcb->priority = REGULAR_PRIO;
    }
    else { 
	file->op_ptr = &f_ops_table;
	current_pcb->priority = REGULAR_PRIO;
	file->op_ptr->open(index, filename, 0); /* Else just open a regular file */
    }
    current_pcb->num_open_files += 1;
    return index;
}

/*
 * kernel_close
 *   DESCRIPTION: closes a file descriptor
 *   INPUTS: fd -
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
int32_t kernel_close()
{

    int32_t fd;
    asm ("			    \
	    movl %%ebx, %0         ;\
	    "
	    :"=g"(fd)
	    :/* no inputs*/
	    :"cc","memory"
	);
    if(fd <2 || fd>= MAX_NUM_FD) // Cant close stdout and stdin or negative fds or more than the max
	return -1;


    pid_t* hpid = get_current_htable_entry();
    proc_t* current_pcb = &hpid->pcb;
    file_table_t* files = current_pcb->open_files;
    if((files->bitmap & (1 << fd)) == 0) // Checks if the fd is open
	return -1;
    fs_t* files_ptr     = files->files;
    fs_t* file = &files_ptr[fd];
    file->op_ptr->close(fd,(uint8_t*)"", 0);	/* perform file's close routine */
    current_pcb->num_open_files -= 1;
    return 0;
}

/*
 * kernel_getargs
 *   DESCRIPTION: gets the arguments when the program was called
 *   INPUTS: buf - the buffer to save the args to
 *           nbytes - how many bytes to copy over
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success -1 on fail (negative nbyes, no args, not enough space)
 *   SIDE EFFECTS:
 */
int32_t kernel_getargs()
{
    pid_t* hpid = get_current_htable_entry();
    proc_t* current_pcb = &hpid->pcb;
    uint8_t* buf;
    int32_t nbytes;
    asm ("  movl %%ebx, %0         ;\
	    movl %%ecx, %1         ;\
	    "
	    :"=g"(buf), "=g"(nbytes)
	    : /*no inputs */
	    :"cc","memory"
	);
    if (CHECK_MSB(nbytes)|| nbytes<0 || buf == NULL ||current_pcb->args[0] == (uint8_t)"" ) //Check if the nbytes is negative or the pointer is NULL or there are no args
	return -1;
    if(strlen((const int8_t*)current_pcb->args) > nbytes) //Check that there is enough space to cpy over
	return -1;

    uint8_t* src = current_pcb->args;
    memcpy((void*)buf, (void*)src, (uint32_t)(strlen((const int8_t*)current_pcb->args) + 1)); // copy over the argument to the given pointer
    return 0;

}

/*
 * kernel_vidmap
 *   DESCRIPTION: maps the text-mode video memory to user space at a pre-set virtual address
 *   INPUTS: screen_start - pointer to the pointer of the video memory
 *   OUTPUTS:
 *   RETURN VALUE: -1 on failure or the address of the video memory
 *   SIDE EFFECTS:
 */
int32_t kernel_vidmap()
{
    uint8_t** screen_start;
    asm ("		       \
	    movl %%ebx, %0;   \
	    "
	    :"=g"(screen_start)
	    :/* no inputs */
	    :"cc","memory"
	);
    if (screen_start == NULL) // Check if the given pointer to pointer is NULL
	return -1;
    //Check that the give pointer is within the loaded code
    if((uint32_t)screen_start < USER_CODE_LOAD_ADDR || (uint32_t)screen_start > (START_OF_USER + __4MB__))
	return -1;

    cli();
    if(current_proc->terminal_id == current_session)
      __map_user_page(VIDEO_START_ADDR, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN);
    else
      __map_user_page((uint32_t)sessions[current_proc->terminal_id].vga.screen_start, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN);

    flush_tlb();

    *screen_start = (uint8_t*)USER_VIDEO_MEM_ADDR;
    current_proc->is_vidmapped = 1; /* set is_vidmapped flag */
    sti();
    return USER_VIDEO_MEM_ADDR;
}

/* EXTRA CREDIT
 * init_sys_call
 *   DESCRIPTION: initializes all the system calls
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: puts all the system calls in the IDT
 */
int32_t kernel_set_handler()
{
    return 0;
}

/* EXTRA CREDIT
 * init_sys_call
 *   DESCRIPTION: initializes all the system calls
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: puts all the system calls in the IDT
 */
int32_t kernel_sigreturn(void)
{
    return 0;
}

/* EXTRA CREDIT
 * check_elf
 *   DESCRIPTION: checks that the file to look at is an elf file
 *   INPUTS: file_name: the filename you want to check
 *   OUTPUTS:
 *   RETURN VALUE: returns -1 if not valid 0 if valid
 *   SIDE EFFECTS: checks the filename
 */
int32_t check_elf(uint8_t* file_name)
{
    uint32_t valid;
    dentry_t process_entry;
    boot_block_t* boot_block =  (boot_block_t*)fs_img_addr;

    valid = read_dentry_by_name((const uint8_t*)file_name, &process_entry);
    if(valid == -1)
	return -1;
    inode_t* elf_inode   = (inode_t*)((process_entry.inode_num + 1)*BLOCK_SIZE + (uint8_t*)fs_img_addr); // + 1 for offset of bootblock
    uint8_t* start_elf = (boot_block->num_inodes + 1 + elf_inode->data_blocks[0])*BLOCK_SIZE + (uint8_t*)fs_img_addr; // + 1 for offset of bootblock
    if ((start_elf[0] == ELF_MAGIC_WORD_0) &&
	    (start_elf[1] == ELF_MAGIC_WORD_1) &&
	    (start_elf[2] == ELF_MAGIC_WORD_2) &&
	    (start_elf[3] == ELF_MAGIC_WORD_3))
	return 0;
    else
	return -1;
}

#endif
