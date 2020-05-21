#ifndef MEMORY_H
#define MEMORY_H
#include "page.h"
#define MEM_SUCCESS	         1
#define MEM_ERROR	           0
#define MAX_PAGES	        1024
#define MAX_NODES_PER_PAGE 256
extern uint32_t* vm_curr_slab_page;
extern uint32_t* vm_curr_obj_addr;
extern void get_current_slab();
extern void init_vm_slab() ;
extern void* get_next_obj();
extern void increment_slab_page();
/* map a physical address to page directory entry corresponding to virt_addr */
extern void *__map_page_directory(uint32_t phys_addr, uint32_t virt_addr, uint32_t flags);

/* map a phys_addr to page table entry corresponding to virt_addr */
extern void *__map_page(uint32_t phys_addr, uint32_t virt_addr, uint32_t flags);

/* map a phys_addr to user page table entry corresponding to virt_addr */
extern void *__map_user_page(uint32_t phys_addr, uint32_t virt_addr, uint32_t flags);
/* return the physical address associated with a virtual address */
extern void *get_phys_addr(uint32_t virt_addr);
extern void* __vm_alloc_list_entry();
extern void  __vm_free_list_entry(void* addr);
extern void init_vm_slab();
extern void* ll_alloc();
extern void  ll_free(void* addr);
#endif
