#ifndef SYSCALL_H
#define SYSCALL_H
#include "types.h"
#define	N_IO_OPS			  0x0004
#define IDT_FLAGS		   	0xEF00
#define __4KB__			    0x00001000
#define __8KB__			    0x00002000
#define __512KB__		    0x00080000
#define __4MB__			    0x00400000
#define __8MB__			    0x00800000
#define PCB_MASK		    0xFFFFE000
#define MAX_PROCESSES		    8
/*
 * PHYS_ADDR_START
 *  DESCRIPTION:  physcial address corresponding to pid number
 *  INPUTS:       x -- pid number of current process
 *  OUTPUTS:      none
 *  RETURN VALUE: starting physical address for process's address space
 *  SIDE EFFECTS: none
 */
#define PHYS_ADDR_START(x)	    ((__4MB__)+(x)*(__4MB__))

/*
 * PCB_START_ADDR
 *  DESCRIPTION:  starting address for process control block
 *  INPUTS:       x -- pid of current pcb
 *  OUTPUTS:      none
 *  RETURN VALUE: address corresponding to start of PCB
 *  SIDE EFFECTS: none
 */
#define PCB_START_ADDR(x)	    ( (__8MB__) - ((__8KB__) * (x)))
#define USER_CODE_LOAD_ADDR	    0x08048000
#define USER_STACK_ADDR		    ((0x08400000) - (0x4))
#define USER_VIDEO_MEM_ADDR	    0x08C00000
#define USER_VIDEO_MEM_OFFSET_MASK  0x0FF00000
/*
 * KERNEL_STACK_ADDR
 *  DESCRIPTION: starting address for kernel stack determined by pid
 *  INPUTS:      x -- pid corresponding to currrent process
 *  OUTPUTS:     none
 *  RETURN VALUE: starting address of kernel stack for the pcb associated with pid
 *  SIDE EFFECTS: none
 */
#define KERNEL_STACK_ADDR(x)	    ( (__8MB__) - (0x2000)*(x))
#define START_OF_USER             0x08000000

/*
 * USER_VIDEO_MEM_OFFSET
 *  DESCRIPTION : calculate offset into user program's video memory
 *  INPUTS      : x -- screen start address
 *  OUTPUTS     : none
 *  RETURN VALUE: offset into user video memory
 *  SIDE EFFECTS: none
 */
#define USER_VIDEO_MEM_OFFSET(x)    \
    (( ((uint32_t)(x)) - ((uint32_t)(START_OF_USER)) ))
#define USER_VIDEO_PAGE_UPPER_RANGE ((__4MB__))
#define SYS_CALL_VEC          0x80
/* JMP_TO_USER
 *  DESCRIPTION:  sets up stack for context switch via IRET
 *  INPUTS:       entry -- address to begin execution in userspace
 *  OUTPUTS:      none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: changes current execution context. modifies current priviledge ring and EIP.
 */
#define JMP_TO_USER(entry)		                                       \
    asm volatile("cli			    \n"                                \
	    "movw %1, %%ax		    \n"                                \
	    "movw %%ax, %%ds	    \n"                                \
	    "movw %%ax, %%es           \n"                                \
	    "movw %%ax, %%fs           \n"                                \
	    "movw %%ax, %%gs           \n"                                \
	    "movl %%esp, %%eax         \n"				       \
	    "pushl %1                  \n"				       \
	    "pushl %2                  \n"				       \
	    "pushfl                    \n"				       \
	    "popl %%eax                \n"                                \
	    "orl $0x200, %%eax         \n"				       \
	    "pushl %%eax               \n"				       \
	    "pushl %3                  \n"				       \
	    "pushl %0                  \n"				       \
	    "iret                      \n"				       \
	    :							       \
	    :"g"(entry), "g"(USER_DS), "g"(USER_STACK_ADDR), "g"(USER_CS) \
	    :"cc","memory","%eax"					       \
	    );
typedef int32_t (*f_ptr)(int32_t, uint8_t*, uint32_t);
struct io_ops_table {
    f_ptr open;
    f_ptr close;
    f_ptr read;
    f_ptr write;
};
typedef struct io_ops_table io_table_t;
extern io_table_t rtc_ops_table;
extern io_table_t dir_ops_table;
extern io_table_t f_ops_table;
extern io_table_t term_ops_table;

/*Inititializes sys calls*/
extern void init_sys_call();

/* User sys_calls*/
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);

/*Kernel handling the sys calls*/
extern int32_t kernel_halt();
extern int32_t kernel_execute();
extern int32_t kernel_read();
extern int32_t kernel_write();
extern int32_t kernel_open();
extern int32_t kernel_close();
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
// extern int32_t set_handler(int32_t signum, void* handler_address);
// extern int32_t sigreturn(void);
extern int32_t sys_call_vector();
/*Fills and IDT entry*/
extern void fill_interrupt(int num, uint32_t* offset, uint16_t seg, uint16_t flags);

/*Function checks if its a ELF file*/
extern int32_t check_elf(uint8_t* file_name);

#endif
