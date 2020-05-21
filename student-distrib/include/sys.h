#ifndef SYS_H
#define SYS_H
#define RT_PL	          100
#define BATCH_PL        40
#define KERN_PL	        0
#define N_PL	          140
#include "sys_call.h"
#include "page.h"
#include "vga.h"
#include "lib.h"
/* generic doubly linked list structure */
struct list_head {
    int16_t pid;
    void*  entry;
    struct list_head* next;
    struct list_head* prev;
};
typedef struct list_head list_head_t;
extern list_head_t* head;
extern list_head_t* last;
/* helper macro to initialize list node */
#define LIST_NODE(node_name)		    \
    struct list_head node_name;		    \
    (&node_name)->next = NULL;		    \
    (&node_name)->prev = NULL;

/* helper macro to initialize list head */
#define LIST_HEAD(list_name)		    \
    struct list_head list_name ;	    \
    LIST_HEAD_INIT(&list_name);

/* helper macro to set an index in a table with a bitmap */
#define set_entry_by_index(table, index)		\
    table.bitmap |= (1 << index);

/* helper macro to clear an index in a table with a bitmap */
#define clear_entry_by_index(table, index)		\
    table.bitmap &= ~(1 << index);

/* helper macro to check if a node is a head of a list */
#define IS_HEAD(x)		    \
    ( (((&x)->next) == (&x)) && ( ((&x)->prev) == (&x)) )

/* generic queue_ops function pointer type */
typedef int32_t (*queue_f_ptr)(list_head_t*);
struct queue_ops_table {
    queue_f_ptr insert_front;
    queue_f_ptr insert_back;
    queue_f_ptr remove;
    queue_f_ptr length;
}; /* queue operations function table */
typedef struct queue_ops_table queue_ops_t;
struct queue { /* queue data structure */
    struct list_head* head;
    struct list_head* last;
    queue_ops_t *ops;
};
typedef struct queue queue_t;

/* virtual memory slab structure for slab allocator  */
struct vm_page_slab {
    uint32_t** page;
    uint32_t  count;
    uint32_t bitmap;
};

typedef struct vm_page_slab vm_page_slab_t;

/* structure for pages of slab objects to be dynamically allocated */
struct vm_obj_slab {
    page_table_t object_table;
    uint32_t bitmap;
};
typedef struct vm_obj_slab vm_slab_t;

/* LIST_HEAD_INIT
 *  DESCRIPTION : helper function to initialize the head of a doubly linked list
 *  INPUT       : name -- node to initialize as head
 *  OUTPUTS     : none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: assigns node next and previous links to itself
 */
static inline void LIST_HEAD_INIT(struct list_head* name)
{
    if (name == NULL)
	return;
    if (head == NULL){
	head = name;
	last = name;
    }
    name->next = name;
    name->prev = name;
}
/* priority array strcture containing #active fields, bitmap, and queue array */
typedef struct prio_array {
    uint8_t n_active;
    uint8_t bitmap; // 140 priorities
    queue_t   tasks[N_PL];
} prio_array_t;

extern vm_slab_t list_pages_slab;
extern vm_page_slab_t list_slab;
extern uint32_t* vm_curr_obj_addr;
extern void print_list(list_head_t* head);
extern int32_t insert_front(list_head_t* node); /* insert node at front of queue */
extern int32_t insert_back(list_head_t* node);  /* enqueue node at back */
extern int32_t delete_node(list_head_t* node);  /* delete a node from queue */
extern int32_t is_empty();                      /* check if queue is empty  */
extern int32_t length();                        /* return queue length      */
extern int test_insert_delete();                /* sanity checks            */
extern void free_node(list_head_t* node);       /* free node and its resources */
extern queue_ops_t queue_ops_table; 
extern queue_t active_tasks;
#endif
