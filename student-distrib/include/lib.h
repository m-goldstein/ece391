/* lib.h - Defines for useful library functions
 * vim:ts=4 noexpandtab
 */

#ifndef _LIB_H
#define _LIB_H
#include "types.h"
#define NUM_CRTC_REGS 25
#define MSB_BITSHIFT  31
#define CHECK_MSB(x) (((x) & (1 <<  (MSB_BITSHIFT))) ? (1) : (0))

/* helper function defined as macro to save ESP */
#define SAVE_ESP(x)										     \
	asm volatile("movl %%esp, %0":"=g"(x.esp)::"cc","memory");           

/* helper function defined as macro to save EBP */
#define SAVE_EBP(x)										     \
	asm volatile("movl %%ebp, %0":"=g"(x.ebp)::"cc","memory");

/* helper fn defined as macro to restore ESP */
#define RESTORE_ESP(x)									     \
	asm volatile("movl %0, %%esp"::"g"(x.esp):"cc","memory");

/* helper fn defined as macro to restore EBP */
#define RESTORE_EBP(x)								      \
	asm volatile("movl %0, %%ebp"::"g"(x.ebp):"cc","memory");

/* helper fn defined as macro to save hardware context */
#define SAVE_REGS(x)					                      \
	do {													  \
		asm volatile ("movw %%gs, %0":"=g"(x.gs)::"memory");			  \
		asm volatile ("movw %%fs, %0":"=g"(x.fs)::"memory");            \
		asm volatile ("movw %%ds, %0":"=g"(x.ds)::"memory");			  \
		asm volatile ("movw %%ss, %0":"=g"(x.ss)::"memory");			  \
		asm volatile ("movw %%cs, %0":"=g"(x.cs)::"memory");			  \
		asm volatile ("movw %%es, %0":"=g"(x.es)::"memory");			  \
		asm volatile ("movl %%edi, %0":"=g"(x.edi)::"memory");		  \
		asm volatile ("movl %%esi, %0":"=g"(x.esi)::"memory");		  \
		asm volatile ("movl %%ebp, %0":"=g"(x.ebp)::"memory");		  \
		asm volatile ("movl %%esp, %0":"=g"(x.esp)::"memory");		  \
		asm volatile ("movl %%ebx, %0":"=g"(x.ebx)::"memory");		  \
		asm volatile ("movl %%edx, %0":"=g"(x.edx)::"memory");		  \
		asm volatile ("movl %%ecx, %0":"=g"(x.ecx)::"memory");		  \
		asm volatile ("movl %%eax, %0":"=g"(x.eax)::"memory");		  \
		asm volatile ("pushfl; popl %0":"=r"(x.eflags)		  \
				::"memory","cc");							  \
	}while(0);

/* helper fn defined as macro to restore hardware context */
#define RESTORE_REGS(x)													\
	do {																\
		asm volatile("xorl %%eax, %%eax":);								\
		asm volatile("movw %%ax, %%ds"::"a"(x.ds));						\
		asm volatile("movw %%ax, %%es"::"a"(x.es));						\
		asm volatile("movw %%ax, %%fs"::"a"(x.fs));						\
		asm volatile("movw %%ax, %%ss"::"a"(x.ss));						\
		asm volatile("movw %%ax, %%gs"::"a"(x.gs));						\
		asm volatile("movl %%eax, %%esp"::"a"(x.esp));					\
		asm volatile("movl %%eax, %%ebp"::"a"(x.ebp));					\
		asm volatile("pushl %%eax;\npopfl"::"a"(x.eflags):"memory");	\
		asm volatile("movl %0, %%eax"::"a"(x.eax));						\
		asm volatile("movl %0, %%ebx"::"a"(x.ebx));						\
		asm volatile("movl %0, %%ecx"::"a"(x.ecx));						\
		asm volatile("movl %0, %%edx"::"a"(x.edx));						\
		asm volatile("movl %0, %%esi"::"a"(x.esi));						\
		asm volatile("movl %0, %%edi"::"a"(x.edi));						\
	} while(0);

int32_t printf(int8_t *format, ...);
void putc(uint8_t c);
int32_t puts(int8_t *s);
extern int8_t *itoa(uint32_t value, int8_t* buf, int32_t radix);
int8_t *strrev(int8_t* s);
uint32_t strlen(const int8_t* s);
uint32_t ustrlen(const uint8_t* s);
void clear(void);
/*Control L like how you would expect it*/
extern void ctrlL();
/*Backspace on screen*/
extern void backspace();
/* CRTC Registers for QEMU VGA */
extern unsigned short qemu_crtc_regs[NUM_CRTC_REGS]; 
void* memset(void* s, int32_t c, uint32_t n);
void* memset_word(void* s, int32_t c, uint32_t n);
void* memset_dword(void* s, int32_t c, uint32_t n);
void* memcpy(void* dest, const void* src, uint32_t n);
void* memmove(void* dest, const void* src, uint32_t n);
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n);
int8_t* strcpy(int8_t* dest, const int8_t*src);
int8_t* strncpy(int8_t* dest, const int8_t*src, uint32_t n);
uint8_t* ustrcpy(uint8_t* dest, uint8_t* src, uint32_t n);
/* Userspace address-check functions */
int32_t bad_userspace_addr(const void* addr, int32_t len);
int32_t safe_strncpy(int8_t* dest, const int8_t* src, int32_t n);
extern void test_interrupts();
/* Port read functions */
/* Inb reads a byte and returns its value as a zero-extended 32-bit
 * unsigned int */
static inline uint32_t inb(port) {
    uint32_t val;
    asm volatile ("             \n\
            xorl %0, %0         \n\
            inb  (%w1), %b0     \n\
            "
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Reads two bytes from two consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them zero-extended
 * */
static inline uint32_t inw(port) {
    uint32_t val;
    asm volatile ("             \n\
            xorl %0, %0         \n\
            inw  (%w1), %w0     \n\
            "
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Reads four bytes from four consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them */
static inline uint32_t inl(port) {
    uint32_t val;
    asm volatile ("inl (%w1), %0"
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}
/* Bitscan operations. bitscan_reverse returns
 * position of MSB that is set
 * bitscan_forward returns the position of LSB
 * that is set.
 */
extern inline int32_t bitscan_reverse(uint32_t bitmap);
extern inline int32_t bitscan_forward(uint32_t bitmap);
/* Writes a byte to a port */
#define outb(data, port)                \
do {                                    \
    asm volatile ("outb %b1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Writes two bytes to two consecutive ports */
#define outw(data, port)                \
do {                                    \
    asm volatile ("outw %w1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Writes four bytes to four consecutive ports */
#define outl(data, port)                \
do {                                    \
    asm volatile ("outl %l1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Clear interrupt flag - disables interrupts on this processor */
#define cli()                           \
do {                                    \
    asm volatile ("cli"                 \
            :                           \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Save flags and then clear interrupt flag
 * Saves the EFLAGS register into the variable "flags", and then
 * disables interrupts on this processor */
#define cli_and_save(flags)             \
do {                                    \
    asm volatile ("                   \n\
            pushfl                    \n\
            popl %0                   \n\
            cli                       \n\
            "                           \
            : "=r"(flags)               \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Set interrupt flag - enable interrupts on this processor */
#define sti()                           \
do {                                    \
    asm volatile ("sti"                 \
            :                           \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Restore flags
 * Puts the value in "flags" into the EFLAGS register.  Most often used
 * after a cli_and_save_flags(flags) */
#define restore_flags(flags)            \
do {                                    \
    asm volatile ("                   \n\
            pushl %0                  \n\
            popfl                     \n\
            "                           \
            :                           \
            : "r"(flags)                \
            : "memory", "cc"            \
    );                                  \
} while (0)

#endif /* _LIB_H */

