#ifndef PAGE_H
#define PAGE_H

#include "types.h"
#define ZERO			0x00000000
#define PAGE_OFFSET		0x0000000c
#define PAGE_BITSHIFT		0x0000000C
#define PAGE_TABLE_MAX_SIZE	0x000003FF
#define PAGE_DIRECTORY_MAX_SIZE	0x000003FF
#define KB_SIZE			0x00000400
#define MB_SIZE			0x00100000
#define EXT_PMD_SHIFT		0x00000016
#define EXT_PMD_MASK		0xFFE00000
#define PMD_SHIFT		0x00000016
#define PMD_OFFSET		0x0000000C
#define PMD_MASK		0xFFC00000
#define PAGE_MASK		0xFFFFF000
#define PAGE_SHIFT		(4)*(KB_SIZE)
#define PAGE_SHIFT_EXT		(4)*(MB_SIZE)
#define PAGE_SIZE		PAGE_SHIFT
#define PAGE_SIZE_EXT		PAGE_SHIFT_EXT
#define KERN_START_ADDR		0x00400000
#define KERN_END_ADDR		0x00800000
#define VIDEO_START_ADDR    	0x000B8000
#define VIDEO_END_ADDR          0x000C0000
/* helpful macros for metadata fields of PTE/PDE structs */
enum paging_metadata {
    PRESENT	    = 0x1,
    RW_EN	    = (0x1 << 1),
    USER_EN	    = (0x1 << 2),
    W_THRU	    = (0x1 << 3),
    CACHE_DIS	    = (0x1 << 4),
    ACCESSED	    = (0x1 << 5),
    DIRTY           = (0x1 << 6),
    EXTENDED_PAGING = (0x1 << 7),
    GLOBAL	    = (0x1 << 8)
};
struct _page_dir_entry {

    uint8_t present : 1;		/* flag indicating whether referred-to page is in main memory */
    uint8_t rw_en : 1;			/* flag indicating access rights of page		      */

    uint8_t user_en : 1;		/* field containing priveledge level required to access page.
					    if bit = 1, anyone can access; else only supervisor        */

    uint8_t w_thru : 1;			/* field indicating whether write through (=1) or
					    write back(=0) strategy should be used.		      */
    uint8_t cache_dis : 1;		/* flag indicating whether hardware caching
					   is disabled (=1) or enabled (=0)	                      */
    uint8_t accessed :1;		/* flag set each time paging unit addresses the page frame    */
    uint8_t reserved : 1;
    uint8_t extended_paging : 1;	/* flag indicating whether page entry refers to a 4kb (=0)
					   or 4mb (=1) page frame				      */
    uint8_t global : 1; 		/* applies to page table entries. ignore for now?   	      */
    uint8_t avail : 3;
    uint32_t address : 20;		/* field containing 20 MSBs of page frame physical address.
					In this case, contains a page table.
					If extended paging is used, only 10 MSBs are siginifcant.  */
    //uint32_t (*rd) (struct _page_dir_entry* pde);
} __attribute__((packed));
typedef struct _page_dir_entry pde_t;

struct _ext_page_dir_entry {

    uint8_t present : 1;		/* flag indicating whether referred-to page is in main memory */

    uint8_t rw_en : 1;			/* flag indicating access rights of page		      */
    uint8_t user_en : 1;		/* field containing priveledge level required to access page.
					    if bit = 1, anyone can access; else only supervisor        */

    uint8_t w_thru : 1;			/* field indicating whether write through (=1) or
					    write back(=0) strategy should be used.		      */

    uint8_t cache_dis : 1;		/* flag indicating whether hardware caching
					   is disabled (=1) or enabled (=0)	                      */
    uint8_t accessed : 1;		/* flag set each time paging unit addresses the page frame    */
    uint8_t reserved : 1;
    uint8_t extended_paging : 1;	/* flag indicating whether page entry refers to a 4kb (=0)
					   or 4mb (=1) page frame				      */

    uint8_t global : 1; 		/* applies to page table entries. ignore for now?   	      */
    uint16_t avail: 13;
    uint32_t address : 10;	   /* field containing 10 MSBs of page frame physical address.
				   In this case, contains a page table.
				   If extended paging is used, only 10 MSBs are siginifcant.  */
    //uint32_t (*rd) (struct _ext_page_dir_entry* pde);
} __attribute__((packed));
typedef struct _ext_page_dir_entry pde_ext_t;

struct page_directory {
	uint32_t directory_table[KB_SIZE] __attribute__((aligned(PAGE_SHIFT)));
	uint32_t ext_directory_table[KB_SIZE] __attribute__((aligned(PAGE_SHIFT)));
};
typedef struct page_directory page_directory_t;

struct _page_table_entry {

    uint8_t present : 1;		/* flag indicating whether referred-to page is in main memory */
    uint8_t rw_en : 1;			/* flag indicating access rights of page		      */
    uint8_t user_en : 1;		/* field containing priveledge level required to access page.
					    if bit = 1, anyone can access; else only supervisor        */
    uint8_t w_thru : 1;			/* field indicating whether write through (=1) or
					    write back(=0) strategy should be used.		      */
    uint8_t cache_dis : 1;		/* flag indicating whether hardware caching
					   is disabled (=1) or enabled (=0)	                      */
    uint8_t accessed :1;		/* flag set each time paging unit addresses the page frame    */
    uint8_t dirty : 1;			/* flag indicating whether page is dirty		      */
    uint8_t reserved : 1;		/* applies to page table entries. ignore for now?   	      */

    uint8_t global : 1;
    uint32_t avail  : 3;
    uint32_t address : 20;	    	/* field containing 20 MSBs of page frame physical address. */
    //uint32_t (*rd) (struct _page_table_entry* pte);
} __attribute__((packed));
typedef struct _page_table_entry pte_t;
struct page_table {
    uint32_t pages[KB_SIZE] __attribute__((aligned(PAGE_SHIFT)));
};
typedef struct page_table page_table_t;



extern uint8_t pde_user (pde_t* pde);   	     /* returns user_en flag        */
extern uint8_t pde_write(pde_t* pde);                /* returns the rw_en flag      */
extern uint8_t pde_pae(pde_t* pde);	             /* returns extended_paging flag */
extern uint8_t pde_young(pde_t* pde);   	     /* returns the accessed flag   */
extern uint8_t pde_present(pde_t* pde);		     /* returns the present flag    */
extern uint8_t pde_uncached(pde_t* pde);	     /* returns the cache_dis flag  */
extern uint8_t pde_write_through(pde_t* pde);        /* returns the w_thru flag	    */
extern uint8_t pde_global(pde_t* pde);               /* returns the global flag     */

extern uint8_t pte_user (pte_t* pte);	    	     /* returns user_en flag        */
extern uint8_t pte_write(pte_t* pte);	    	     /* returns the rw_en flag      */
extern uint8_t pte_dirty(pte_t* pte);	    	     /* returns the dirty flag      */
extern uint8_t pte_young(pte_t* pte);	    	     /* returns the accessed flag   */
extern uint8_t pte_present(pte_t* pte);	             /* returns the present flag    */
extern uint8_t pte_write_through(pte_t* pte);	     /* returns the w_thru flag	    */
extern uint8_t pte_uncached(pte_t* pte);	     /* returns the cache_dis flag */
extern uint8_t pte_global(pte_t* table);	     /* returns the global flag     */

extern uint8_t set_pde_user (pde_t* pde, uint8_t flag);   /* sets user_en flag         */
extern uint8_t set_pde_write(pde_t* pde, uint8_t flag);   /* sets the rw_en flag       */
extern uint8_t set_pde_pae(pde_t* pde, uint8_t flag);     /* sets extended_paging flag */
extern uint8_t set_pde_young(pde_t* pde, uint8_t flag);   /* sets the accessed flag    */
extern uint8_t set_pde_present(pde_t* pde, uint8_t flag); /* sets the present flag     */
extern uint8_t set_pde_write_through(pde_t* pde, uint8_t flag); /* sets the w_thru flag      */
extern uint8_t set_pde_uncached(pde_t* pde, uint8_t flag); /* sets the cache_dis flag  */
extern uint8_t set_pde_global(pde_t* pde, uint8_t flag);   /* sets the global flag    */
extern uint8_t set_pte_user (pte_t* pte, uint8_t flag);     /* sets user_en flag         */
extern uint8_t set_pte_write(pte_t* pte, uint8_t flag);     /* sets the rw_en flag       */
extern uint8_t set_pte_dirty(pte_t* pte, uint8_t flag);     /* sets the dirty flag       */
extern uint8_t set_pte_young(pte_t* pte, uint8_t flag);     /* sets the accessed flag    */
extern uint8_t set_pte_present(pte_t* pte, uint8_t flag);   /* sets the present flag     */
extern uint8_t set_pte_write_through(pte_t* pte, uint8_t flag); /* sets the w_thru flag  */
extern uint8_t set_pte_uncached(pte_t* pte, uint8_t flag);  /* sets the cache_dis flag   */
extern uint8_t set_pte_global(pte_t* pte, uint8_t flag);    /* sets the global flag      */

extern page_table_t page_table;
extern page_table_t user_pte;
extern page_directory_t page_directory;
extern page_table_t list_pte;
/* ASM functions */
extern void load_pde(uint32_t* page_directory);
extern void flush_tlb();
extern void flush_tlb_global();
static inline void flush_tlb_single(uint32_t addr) {
    asm volatile("invlpg (%0)"::"r" (addr) : "memory");
}
extern void trigger_page_fault(uint32_t* pte);
void identity_paging(uint32_t* page_table_start, uint32_t vaddr, int size);
extern uint8_t paging_init();

#endif
