#ifndef KILLSCREEN_C
#define KILLSCREEN_C
#include "include/vga.h"
#include "include/killscreen.h"
/* show_kill_screen 
 * DESCRIPTION: print the hardware context (register values) when the exception occured
 * INPUTS: none
 * OUTPUTS: prints the register values at the time when exception occured 
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void show_kill_screen()
{
    uint16_t fs;
    uint16_t es;
    uint16_t ds;
    uint16_t cs;
    uint32_t eax;
    uint32_t ebp;
    uint32_t esp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    asm volatile ("mov %%fs, %0":"=r"(fs));
    asm volatile ("mov %%es, %0":"=r"(es));
    asm volatile ("mov %%ds, %0":"=r"(ds));
    asm volatile ("mov %%cs, %0":"=r"(cs));
    asm volatile("mov %%eax, %0":"=r"(eax));
    asm volatile("mov %%ebp, %0":"=r"(ebp));
    asm volatile("mov %%esp, %0":"=r"(esp));
    asm volatile("mov %%edi, %0":"=r"(edi));
    asm volatile("mov %%esi, %0":"=r"(esi));
    asm volatile("mov %%edx, %0":"=r"(edx));
    asm volatile("mov %%ecx, %0":"=r"(ecx));
    asm volatile("mov %%ebx, %0":"=r"(ebx));
    vga_printf("\nDumping Registers:\nEAX: %x\nEBX: %x\nECX: %x\nEDX: %x\nEDI: %x\nESI: %x\nEBP: %x\nESP: %x\nFS: %x\nES: %x\nDS: %x\nCS: %x\n", eax, ebx, ecx, edx, edi, esi, ebp, esp, fs, es, ds, cs);
}
#endif
