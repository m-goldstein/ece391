#ifndef MEMORY_C
#define MEMORY_C
#include "include/memory.h"
#include "include/sys.h"
#include "include/lib.h"
#define MIN_ALLOC 1024
vm_slab_t list_pages_slab;
vm_page_slab_t list_slab;
uint32_t* vm_curr_slab_page;
uint32_t* vm_curr_obj_addr;
/*
 * get_phys_addr
 *  DESCRIPTION: returns physical address associated with virtual address passed as input
 *  INPUTS:      virt_addr - virtual address used to locate corresponding physical address
 *  OUTPUTS:     none
 *  RETURN VALUE: returns page directory entry mapped to that virtual address
 *  SIDE EFFECTS: none
 */
void* get_phys_addr(uint32_t virt_addr)
{
    uint32_t pd_idx = (uint32_t)(virt_addr >> PMD_SHIFT); /* page directory index value */
    uint32_t offset = (uint32_t)(virt_addr & ~PMD_MASK);  /* offset into page           */
    uint32_t* pd_base;
    /* load pd_base with address currently in CR3 (PDBR) */
    asm volatile("              \
	    movl %%cr3, %%eax  ;\
	    "
	    : "=a"(pd_base)
	    :
	    :"cc","memory"
	    );

    uint32_t pde32 = *((uint32_t*)&pd_base[pd_idx]); /* page directory entry represented as uint32_t*/
    pde32 &= ~PMD_MASK; /* physical address algined on 4MB boundary */
    printf("Phys Address: %x\n", pde32+offset);
    return (void*)(pde32+offset); /* return corresponding PDE */
}
/*
 * __map_page_directory
 *  DESCRIPTION: maps phys_addr to an entry in system's page directory table
 *  INPUTS:      phys_addr - physical address to map to a page directory entry
 *		 virt_addr - virtual address to locate corresponding page directory to be mapped
 *		 flags     - bits of PDE's metadata fields to be set
 * OUTPUTS: none
 * RETURN VALUE: address of Page Directory Entry that has been mapped
 * SIDE EFFECTS: maps a page directory entry corresponding to virt_addr to the phys_addr passed
 *               as a parameter
 */
void *__map_page_directory(uint32_t phys_addr, uint32_t virt_addr, uint32_t flags)
{
    if (&page_directory == NULL)
	return MEM_ERROR;
    if (virt_addr % PAGE_SHIFT_EXT != ZERO) {
	virt_addr /= PAGE_SHIFT_EXT;
	virt_addr *= PAGE_SHIFT_EXT;                 /* align to 4MB boundary */
    }
    uint32_t pd_idx = (uint32_t)(virt_addr >> PMD_SHIFT) & PAGE_DIRECTORY_MAX_SIZE;
    uint32_t pde32 = page_directory.directory_table[pd_idx];/* locate page directory entry         */
    pde32 |= flags;                                         /* turn on metadata bits               */
    pde_ext_t pde = *((pde_ext_t*)&pde32);                  /* align to a 4MB page directory entry */
    pde.address = phys_addr >> PMD_SHIFT;                   /* apply bitshift                      */
    pde32 = *((uint32_t*)&pde);                             /* cast back to uint32_t type          */
    page_directory.directory_table[pd_idx] = pde32;         /* update page directory table with new entry */
    flush_tlb(); /* flush tlb */
    return (void*)&page_directory.directory_table[pd_idx]; /* return PDE just mapped */
}

/*
 * __map_page
 *  DESCRIPTION: maps phys_addr to an entry in system page table
 *  INPUTS: phys_addr - physical address to map to a page
 *	    virt_addr - virtual address to locate corresponding page to be mapped
 *	    flags     - bits to set in the PTE metadata fields
 *  OUTPUTS: none
 *  RETURN VALUE: The address of the user page mapped
 *  SIDE EFFECTS: Maps the physical address to the page pointed to by the virtual address
 *  TO BE USED FOR USER MEMORY SPACE
 */
page_table_t user_pte;
void *__map_page(uint32_t phys_addr, uint32_t virt_addr, uint32_t flags)
{
    if (page_table.pages == NULL || page_directory.directory_table == NULL)
	return MEM_ERROR;

    if (virt_addr % PAGE_SHIFT_EXT != ZERO) {
	virt_addr /= PAGE_SHIFT_EXT;
	virt_addr *= PAGE_SHIFT_EXT;                 /* align to 4MB boundary */
    }

    uint32_t pte32;
    uint32_t pde32;
    uint32_t pd_idx = (uint32_t)(virt_addr >> 22) & PAGE_DIRECTORY_MAX_SIZE; /* index into page directory */
    uint32_t pt_idx = (uint32_t)(virt_addr >> 12) & PAGE_TABLE_MAX_SIZE; /* index into page table */
    pde32 = page_directory.directory_table[pd_idx]; /* PDE as uint32_t */
    pde32 |= flags;
    pde_t pde = *((pde_t*)&pde32);
    pde.extended_paging = 0;
    pde.address = ((uint32_t)&page_table.pages) >> 12;
    page_directory.directory_table[pd_idx] = *((uint32_t*)&pde);

    pte32 = page_table.pages[pt_idx];
    pte32 |= flags;
    pte_t pte = *((pte_t*)&pte32);
    pte.address = phys_addr >> 12; // Bit shift by 12 bc you need the top 20 bits
    page_table.pages[pt_idx] = *((uint32_t*)&pte);
    flush_tlb();
    return (void*)page_table.pages[pt_idx]; /* return PTE just mapped */
}
/*
 * __map_user_page
 *  DESCRIPTION: maps physical address to a page in userspace and sets metadata bits
 *		 according to value in flags.
 *  INPUTS: phys_addr - the physical address to map to the page
 *	    virt_addr - the virtual address to map to the page
 *	    flags     - values for the metadata bits in the PTE struct
 *  OUTPUTS: none
 *  RETURN VALUE: address of the start of the user page table
 *  SIDE EFFECTS: maps the phys_addr to the user page indexed by the virtual addr
 */

void *__map_user_page(uint32_t phys_addr, uint32_t virt_addr, uint32_t flags)
{
    if (user_pte.pages == NULL || page_directory.directory_table == NULL)
	return MEM_ERROR;

    if (virt_addr % PAGE_SHIFT_EXT != ZERO) {
	virt_addr /= PAGE_SHIFT_EXT;
	virt_addr *= PAGE_SHIFT_EXT;                 /* align to 4MB boundary */
    }

    uint32_t pte32;
    uint32_t pde32;
    uint32_t pd_idx = (uint32_t)(virt_addr >> 22) & PAGE_DIRECTORY_MAX_SIZE; /* index into page directory */
    uint32_t pt_idx = (uint32_t)(virt_addr >> 12) & PAGE_TABLE_MAX_SIZE; /* index into page table */
    pde32 = page_directory.directory_table[pd_idx]; /* PDE as uint32_t */
    pde32 |= flags;
    pde_t pde = *((pde_t*)&pde32);
    pde.extended_paging = 0;
    pde.address = ((uint32_t)&user_pte.pages) >> 12; // Bit shift by 12 bc you need the top 20 bits
    page_directory.directory_table[pd_idx] = *((uint32_t*)&pde);

    pte32 = user_pte.pages[pt_idx];
    pte32 |= flags;
    pte_t pte = *((pte_t*)&pte32);
    pte.address = phys_addr >> 12;
    user_pte.pages[pt_idx] = *((uint32_t*)&pte);
    flush_tlb();
    return (void*)user_pte.pages[pt_idx]; /* return PTE just mapped */
}

/* get_current_slab
 *  DESCRIPTION:  function to set pointer to offset into page where objects
 *	          are dynamically allocated
 *  INPUTS:       none
 *  OUTPUTS:      none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: adjusts offset into page where objects are allocated from
 */
void get_current_slab()
{
    uint32_t i;
    if (*list_slab.page == NULL) {
	if (list_pages_slab.bitmap & 0xCFF)
	    increment_slab_page();
	for (i = 0; i < MAX_PAGES; i++) {
	    if ((list_pages_slab.bitmap & (1 << i)) == 0) {
		vm_curr_slab_page = &list_pages_slab.object_table.pages[i];
		list_pages_slab.bitmap |= (1 << i);
		break;
	    }
	}
	*list_slab.page = vm_curr_slab_page;
    }
}

/* init_vm_slab
 *  DESCRIPTION:  initializes page in memory where objects are dynamically
 *		  allocated from using a slab allocator.
 *  INPUTS:       none
 *  OUTPUTS:      none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: maps page in memory where dynamic allocation will occur.
 */
void init_vm_slab()
{
    list_slab.page = &vm_curr_slab_page;
    //__map_page((uint32_t)list_slab.page, (uint32_t)vm_curr_slab_page, PRESENT | RW_EN | USER_EN);
    get_current_slab();
    if (vm_curr_slab_page == NULL)
	return;
    *list_slab.page = vm_curr_slab_page;
    list_slab.count = 0;
    list_slab.bitmap = 0;
    vm_curr_obj_addr = *list_slab.page;
}

/* increment_slab_page
 *  DESCRIPTION:  checks if page is fully and adjusts offset to start of next page
 *                or location where next object should be allocated to
 *  INPUTS:       none
 *  OUTPUTS:      none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: adjusts offset into page and value of vm_curr_slab_page pointer
 */
void increment_slab_page() {
    uint32_t i;
    for (i = 0; i < MAX_PAGES; i++) {
	if ( 0 == (list_pages_slab.bitmap & (1 << i)) ) {
	    vm_curr_slab_page = &list_pages_slab.object_table.pages[i];
	    list_pages_slab.bitmap |= (1 << i);
	    list_slab.count         = 0;
	    list_slab.bitmap        = 0;
	    *list_slab.page         = vm_curr_slab_page;
	    vm_curr_obj_addr        = vm_curr_slab_page;
	}
    }
}

/* get_next_obj
 *  DESCRIPTION:  returns pointer to location where next object can be allocated
 *                from the slab
 *  INPUTS:       none
 *  OUTPUTS:      none
 *  RETURN VALUE: a void pointer to base memory address where object
 *                can be slab allocated
 *  SIDE EFFECTS: adjusts value of vm_curr_obj_addr pointer
 */
void* get_next_obj() {
    uint32_t* addr;
    uint32_t i;
    get_current_slab();
    if (vm_curr_slab_page == NULL)
	return (void*)NULL;
    uint32_t nodes_per_page = MAX_NODES_PER_PAGE;
    if (list_slab.count > nodes_per_page) {
	increment_slab_page();
    }
    for (i = 0; i < nodes_per_page; i++) {
	if ((list_slab.bitmap & (1 << i)) == 0)
	    break;
    }
    vm_curr_obj_addr += sizeof(list_head_t);
    addr = vm_curr_obj_addr;
    list_slab.count += 1;
    list_slab.bitmap |= (1 << i);

    return (void*)addr;
}

/* __vm_alloc_entry
 *  DESCRIPTION:  allocates a region of memory where the slab object
 *                may be allocated to
 *  INPUTS:       none
 *  OUTPUTS:      none
 *  RETURN VALUE: void pointer to address where object may be allocated
 *  SIDE EFFECTS: none
 */
void* __vm_alloc_entry()
{
    if (vm_curr_slab_page == NULL || vm_curr_obj_addr == NULL) {
	//init_vm_slab();
    }
    vm_curr_obj_addr = (uint32_t*)get_next_obj();
    return (void*)vm_curr_obj_addr;
}

/* __vm_free_list_entry
 *  DESCRIPTION:  frees a region of memory where the slab object was allocated to
 *  INPUTS:       void pointer to address of object to be freed
 *  OUTPUTS:      none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: adjusts value of vm_curr_obj_addr to reclaim
 *                memory previously allocated
 */
void __vm_free_list_entry(void* addr) {
    if (addr == NULL) {
	return;
    }
    vm_curr_obj_addr = (uint32_t*)addr;
    vm_curr_obj_addr -= sizeof(list_head_t);
    //__map_page((uint32_t)list_slab.page, (uint32_t)addr, 0);
}

/* ll_alloc
 *  DESCRIPTION:  allocates a region of memory in the slab for an list_head_t
 *                object.
 *  INPUTS:       none
 *  OUTPUTS:      none
 *  RETURN VALUE: a void pointer pointing to starting address in slab where object
 *                may be allocated to
 *  SIDE EFFECTS: adjusts vm_curr_obj_addr pointer to allocate memory to object
 */
list_head_t allocations[32]; // 32 because uint32_t only has 32 bits
volatile uint32_t allocations_bitmap = 0; // All allocations valid at the beginning
void* ll_alloc()
{
    uint32_t temp_bitmap = allocations_bitmap;
    int i = 0;
    while( (temp_bitmap >> i++ & 0x01) != 0){}
    allocations_bitmap |= (0x01 << (i-1)); // minus one is due to i++
    return &(allocations[i-1]); // minus one is due to i++
}

/* ll_free
 *  DESCRIPTION:  reclaims a region of memory in the slab where a list_head_t
 *                object was previously allocated.
 *  INPUTS:       none
 *  OUTPUTS:      none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: adjusts vm_curr_obj_addr pointer to reclaim memory previously
 *                allocated to object.
 */
void ll_free(void* addr)
{
    if (addr == NULL)
	return;
    uint32_t i = 0;
    for(;i<32;i++) // 32 because 32 bits in uint32_t
    {
	if(&allocations[i] == addr)
	{
	    allocations_bitmap &= ~(0x1 << i);
	    return;
	}
    }
    return;
}
#endif
