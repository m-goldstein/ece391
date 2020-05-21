#ifndef PAGE_C
#define PAGE_C

#include "include/lib.h"
#include "include/x86_desc.h"
#include "include/page.h"
#define VIDEO_SIZE 80*25*2

page_directory_t page_directory;
page_table_t page_table;
page_table_t video_pages;
page_table_t user_pages;
/* paging_init
 *
 * DESCRIPTION: helper function to initialize page table entries, kernel page directory entry,
 *		and video memory page tables
 * INPUTS: none
 * OUTPUTS: none
 * SIDE EFFECTS: enables PGE in cr0, enables PSE in cr4, sets permission bits for user pages,
 *		 kernel pages, and video pages.
 */
uint8_t paging_init()
{
    uint32_t i;
    pte_t pte;
    pde_t pde;
    for(i = 0; i < PAGE_TABLE_MAX_SIZE; i++) {
	if ( ((i << 12) >= VIDEO_START_ADDR) && ((i << 12) < VIDEO_END_ADDR -1)){ // Bit shift 12 to represent the start of video start addr
	    pte.present   = 1;
	    pte.user_en   = 0;
	    pte.rw_en     = 1;
	}
	else {
	    pte.present   = 0;
	    pte.user_en   = 0;
	    pte.rw_en     = 0;
	}
	pte.w_thru    = 0;
	pte.cache_dis = 0;
	pte.accessed  = 0;
	pte.dirty     = 0;
	pte.global    = 0;
	pte.address = i;
	page_table.pages[i] = *((uint32_t*)(&pte));
    }

    pde.present   = 1;
    pde.rw_en     = 0;
    pde.user_en   = 0;
    pde.w_thru    = 0;
    pde.cache_dis = 0;
    pde.accessed  = 0;
    pde.extended_paging = 0;
    pde.global    = 0;
    pde.address   = (uint32_t)page_table.pages >> 12; // Need the top 20 bits of the addr
    page_directory.directory_table[0]   = *((uint32_t*)(&pde));

    pde_ext_t kpde;
    kpde.present = 1;
    kpde.rw_en   = 1;
    kpde.user_en = 0;
    kpde.w_thru  = 0;
    kpde.cache_dis = 0;
    kpde.accessed = 0;
    kpde.extended_paging = 1;
    kpde.global = 0;
    kpde.address = (uint32_t)KERN_START_ADDR >> 22; // Need the top 10 bits for non-extended paging addr
    page_directory.directory_table[1] = *((uint32_t*)(&kpde));
   //   for (i = 0; i < PAGE_TABLE_MAX_SIZE-1; i++) {
	 // identity_paging(page_table.pages, i, KB_SIZE);
   //   }
    for (i = 2; i < PAGE_DIRECTORY_MAX_SIZE; i++) {
	pde_ext_t pde;
	pde.present = 0;
	pde.rw_en = 0;
	pde.user_en = 0;
	pde.w_thru = 0;
	pde.cache_dis = 0;
	pde.accessed = 0;
	pde.extended_paging = 1;
	pde.global = 0;
	pde.address = i ;
	page_directory.directory_table[i] = *((uint32_t*)(&pde));
    }
    load_pde((uint32_t*)page_directory.directory_table);
    flush_tlb();
    return 0;
}
void identity_paging(uint32_t* page_table_start, uint32_t vaddr, int size)
{
    vaddr = vaddr & PAGE_MASK;
    for (; size > 0; vaddr += PAGE_SIZE, size -= PAGE_SIZE, page_table_start += 1) {
	*page_table_start = vaddr | PRESENT;
    }
}

/* Helper functions to form an interface for accessing/setting the fields of
 * Page Table Directory entries and Page Table entries.
 */

/*
 * sets user/supervisor access permission field in PDE
 */
uint8_t set_pde_user(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->user_en = flag;
    return 0;

}

/*
 * sets read/write field in PDE
 */
uint8_t set_pde_write(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->rw_en = flag;
    return 0;
}
/*
 * sets extended paging (PSE) field in PDE
 */
uint8_t set_pde_pae(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->extended_paging = flag;
    return 0;
}

/*
 * sets accessed field in PDE
 */
uint8_t set_pde_young(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->accessed = flag;
    return 0;
}

/*
 * sets present field in PDE
 */
uint8_t set_pde_present(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->present = flag;
    return 0;
}

/*
 * sets write through flag in PDE
 */
uint8_t set_pde_write_through(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->w_thru = flag;
    return 0;
}

/*
 * sets cache disabled field in PDE
 */
uint8_t set_pde_uncached(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->cache_dis = flag;
    return 0;
}

/*
 * sets global flag in PDE
 */
uint8_t set_pde_global(pde_t* pde, uint8_t flag)
{
    if (pde == NULL)
	return -1;
    pde->global = flag;
    return 0;
}

/*
 * sets user/supervisor access permissions field in PTE
 */
uint8_t set_pte_user(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->user_en = flag;
    return 0;
}

/*
 * set read/write permissions field in PTE
 */
uint8_t set_pte_write(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->rw_en = flag;
    return 0;
}

/*
 * set dirty flag in PTE
 */
uint8_t set_pte_dirty(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->dirty = flag;
    return 0;
}

/*
 * set accessed field in PTE
 */
uint8_t set_pte_young(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->accessed = flag;
    return 0;
}

/*
 * set present flag in PTE
 */
uint8_t set_pte_present(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->present = flag;
    return 0;
}

/*
 * set write through field in PTE
 */
uint8_t set_pte_write_through(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->w_thru = flag;
    return 0;
}

/*
 * set cache disabled flag in PTE
 */
uint8_t set_pte_uncached(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->cache_dis = flag;
    return 0;
}

/*
 * set global flag in PTE
 */
uint8_t set_pte_global(pte_t* pte, uint8_t flag)
{
    if (pte == NULL)
	return -1;
    pte->global = flag;
    return 0;
}

/*
 * return user/supervisor field in PDE
 */
uint8_t pde_user(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->user_en;
}

/*
 * return read/write flag in PDE
 */
uint8_t pde_write(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->rw_en;
}

/*
 * return accessed field in PDE
 */
uint8_t pde_young(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->accessed;
}

/*
 * return extended_paging field in PDE
 */
uint8_t pde_pae(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->extended_paging;
}

/*
 * return present flag in PDE
 */
uint8_t pde_present(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->present;
}

/*
 * return cache disabled field of PDE
 */
uint8_t pde_uncached(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->cache_dis;
}

/*
 * return write through field in PDE
 */
uint8_t pde_write_through(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->w_thru;
}

/*
 * return global field in PDE
 */
uint8_t pde_global(pde_t* pde)
{
    if (pde == NULL)
	return -1;
    return pde->global;
}

/*
 * return user/supervisor permissions flag in PTE
 */
uint8_t pte_user(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return pte->user_en;
}

/*
 * return read/write access flag in PTE
 */
uint8_t pte_write(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return pte->rw_en;
}

/*
 * return dirty flag in PTE
 */
uint8_t pte_dirty(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return pte->dirty;
}

/*
 * return accessed flag in PTE
 */
uint8_t pte_young(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return pte->accessed;
}

/*
 * return present flag in PTE
 */
uint8_t pte_present(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return pte->present;
}

/*
 * return write through flag in PTE
 */
uint8_t pte_write_through(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return pte->w_thru;
}

/*
 * return cache disabled flag in PTE
 */
uint8_t pte_uncached(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return pte->cache_dis;
}

/*
 * return global flag in PTE
 */
uint8_t pte_global(pte_t* pte)
{
    if (pte == NULL)
	return -1;
    return 0;
}

#endif
