#ifndef TASK_H
#define TASK_H
#include "types.h"
#include "x86_desc.h"
#include "sys.h"
#include "fs.h"
#define KERNEL_PID		          0
#define CMD_NAME_MAX_LEN	      32
#define CMD_ARGS_MAX_LEN	      1024
#define RLIMIT_STACK_SIZE       4MB/2 // max page frames owned by file
#define KERNEL_PRIO		          0
#define REAL_TIME_PRIO		      1
#define INTERACTIVE_PRIO	      2
#define REGULAR_PRIO		        3
#define MAX_PIDS                256
#define P_SUCCESS		            0
#define P_FAIL			            -1
/* assign macros to ELF magic words */
enum elf_magic_numbers {
    ELF_MAGIC_WORD_0 = 0x7f,
    ELF_MAGIC_WORD_1 = 0x45,
    ELF_MAGIC_WORD_2 = 0x4c,
    ELF_MAGIC_WORD_3 = 0x46
};
typedef enum elf_magic_numbers elf_magic_num_t;

/* define structure to store hardware context */
struct hw_regs {
    uint16_t gs;
    uint16_t gs_pad;
    uint16_t fs;
    uint16_t fs_pad;
    uint16_t ds;
    uint16_t ds_pad;

    uint16_t ss;
    uint16_t ss_pad;
    uint16_t cs;
    uint16_t cs_pad;
    uint16_t es;
    uint16_t es_pad;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eflags;


};
typedef struct hw_regs regs_t;

/* assign macros to task states */
enum task_state_t{
    TASK_RUNNING         = 0x01,
    TASK_INTERRUPTIBLE   = 0x02,
    TASK_UNINTERRUPTABLE = 0x04,
    TASK_STOPPED         = 0x08,
    TASK_ZOMBIE          = 0x10
};
/* PCB structure */
struct process_control_block {
    file_table_t* open_files;
    regs_t user_regs;               /* to save/restore user registers and context   */
    regs_t kernel_regs;             /* to save/restore kernel registers and context */
    uint8_t stack_addr;             /* new task's ESP in user space, 0 for kernel threads */
    uint8_t terminal_id;	    /* associated terminal session                        */
    uint8_t active;
    int16_t pid;                   /* associated PID */
    struct process_control_block* parent; /* process parent */
    struct process_control_block* child;  /* process child  */
    uint8_t command[CMD_NAME_MAX_LEN];    /* associated command   */
    uint8_t args[CMD_ARGS_MAX_LEN];       /* associated arguments */
    uint32_t entry_point;                 /* executable entry point address */
    int32_t  file_table_num;
    int8_t   num_open_files;
    uint8_t  state;
    uint8_t  priority;
    uint8_t  is_vidmapped;		  /* flag whether process has vidmapping */
    int32_t  rtc_freq;          /* current rtc_freq the rtc read is running at*/
};
typedef struct process_control_block proc_t;
//Struct for a task
/*typedef struct task_struct {
    list_head_t* task_list;
    proc_t* thread_info;
    uint8_t num_open_files;
    uint16_t pid;
    uint8_t state;
    struct task_struct* next;
    struct task_struct* prev;
} task_struct_t; */


//Hash table of tasks (PID is the index in hash)
//Uses Separate Chaining
typedef struct pid_htable_entry {
    int16_t pid;
    proc_t	   pcb;
    list_head_t* node;
} pid_t;

typedef struct pid_hash_struct{
    pid_t pids[MAX_PIDS];
    uint32_t bitmap;
} pid_htable_t;
extern queue_t* current_queue;
extern pid_htable_t pid_htable;
//extern proc_t* current_proc;
/* Used at boot time to create sentinel task */
extern void init_task(proc_t* thread_info); /* initialize a task with Process descriptor passed as parameter */
extern pid_t* get_current_htable_entry();
extern pid_t* get_next_free_htable_entry();
extern int16_t getpid(); /* get PID of current process/task */
extern int16_t next_free_pid(); /* return next available PID */
extern proc_t* mk_proc(uint8_t* command, uint8_t* args); /* make a PCB with command and args fields filled in */
extern int32_t switch_to_user_proc(int16_t pid); /* switch execution context to another user program */
extern int32_t switch_to_kernel_proc(int16_t pid); /* switch execution context to kernel space */
extern void save_regs(regs_t* regs); /* save registers */
extern void restore_regs(regs_t regs); /* restore registers */
extern void init_idle_task(); /* initialize idle task (pid = 0) */
extern int32_t parse_file(uint8_t* file_name, uint8_t* command);
extern void parse_command_args(proc_t* pcb, uint8_t* command); /* parse command and fill command field in pcb */
extern void parse_args(proc_t* pcb, uint8_t* command); /* parse command and fill command field in pcb */
extern void* read_proc_by_pcb();
extern void* read_proc_by_pid(int16_t pid);
extern int32_t is_valid_elf_header(uint8_t* elf_data); /* check if file data is a valid ELF */
extern volatile int16_t next_pid ; /* next available pid  */
extern volatile int16_t curr_pid ; /* pid of current task */
extern void  close_proc(proc_t* proc); /* close the process and free system resources from its PCB */
#endif
