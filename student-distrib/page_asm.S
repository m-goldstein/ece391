#define ASM			1

/*
 * References:
 * https://www.wiki.osdev.org/Paging
 * https://www.wiki.osdev.org/Setting_Up_Paging
 * https://www.wiki.osdev.org/TLB
 */

.section    .data
    PGE_BITMASK=0x80000000	/* bit 31 of cr0 sets paging support         */
    PGD_BITMASK=0x00000000
    PSE_BITMASK=0x00000010	/* bit 4 of cr4 sets page size extension bit */

.section    .text
.global load_pde
.global flush_tlb
.global flush_tlb_global
.global trigger_page_fault
.align 4


/*
 * load_pde
 * DESCRIPTION: loads physical address of argument into cr3 control register
 *		and sets PG (bit 31) bit in cr0 control register
 * INPUTS: page_directory -- physical address (stored on stack (C-style))
 * OUTPUTS: none
 * SIDE EFFECTS: modifes cr0 and cr3 control registers
 */
load_pde:
    pushl %ebp
    movl %esp, %ebp
    xorl %ecx, %ecx
    xorl %eax, %eax
    movl 8(%ebp), %ecx
    cmpl $0, %ecx
    je  err_bad_addr	    /* check for null reference    */
    movl %ecx, %cr3         /* set page table ptr          */
    movl %cr4, %eax
    orl $PSE_BITMASK, %eax   /* set page size extension (PSE) bit */
    movl %eax, %cr4
    movl %cr0, %eax
    orl $PGE_BITMASK, %eax
    movl %eax, %cr0	    /* set paging (PG) bit */
    leave
    ret

err_bad_addr:
    leave
    ret
/*
 * flush_tlb
 * DESCRIPTION: reload cr3 with current page directory entry
 * INPUTS: none
 * OUTPUTS: none
 * SIDE EFFECTS: reloads contents of cr3 through mov instructions
 */
flush_tlb:
    movl %cr3, %eax
    movl %eax, %cr3
    ret
/*
 * flush_tlb_global
 * DESCRIPTION: similar to flush_tlb except this function clears the PGE bit in cr0,
 *		reloads the contents of cr3, and then sets the PGE bit in cr0
 * INPUTS: none
 * OUTPUTS: none
 * SIDE EFFECTS: reloads cr3 and cr0
 *
 */
flush_tlb_global:
    movl %cr0, %eax
    xorl %eax, %eax
    movl %eax, %cr0	    /* clear PGE bit */
    movl %cr3, %eax
    movl %eax, %cr3	    /* reload cr3 */
    movl %cr0, %eax
    orl PGE_BITMASK, %eax
    movl %eax, %cr0	    /* set PGE bit */
    ret
/*
 * trigger_page_fault
 * DESCRIPTION: helper function to trigger page fault exception by addressing invalid memory
 * INPUTS: pte -- page table entry
 * OUTPUTS: none
 * SIDE EFFECTS: throws a page fault.
 */
trigger_page_fault:
    movl 8(%esp), %eax
    andb $0xFE, %al
    movb $0, 0
    ret
