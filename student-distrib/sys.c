#ifndef SYS_C
#define SYS_C
#include "include/vga.h"
#include "include/sys.h"
#include "include/task.h"
#include "include/memory.h"
#include "include/sched.h"
#include "include/task.h"

list_head_t* head;
list_head_t* last;
uint32_t* vm_curr_obj_addr;

queue_ops_t queue_ops_table = {
    &insert_front,
    &insert_back,
    &delete_node,
    &length
};
queue_t active_tasks;
/* delete_node
 *  DESCRIPTION:  removes a node from the back of the list associated 
 *                with current_queue
 *  INPUTS:       node -- a pointer to a list_head_t node that is to be removed.
 *  OUTPUTS:      none
 *  RETURN VALUE: 0 on success, -1 on fail.
 *  SIDE EFFECTS: removes node from list
 */
int32_t delete_node(list_head_t* node)
{
    /* verify queue head and last pointers and input */
    if (node == NULL || current_queue->head == NULL || current_queue->last == NULL )
	return -1;
    // check if removing head
    if (node == current_queue->head) {
	return -1; // cannot remove swapper
    }
    else { // removing middle node
	list_head_t* ref = current_queue->head;
	list_head_t* prev = current_queue->head;
	/* traverse queue to node using a reference node and previous pointer */
	while (ref != current_queue->head || ref != node) {
	    prev = ref;
	    ref = ref->next;
	    if (ref == node)
		break;
	}
	/* case 1: removing node between head and tail */
	if (ref != current_queue->last) {
	    prev->next = ref->next;
	    ref->next->prev = prev;
	    ref->prev->next = prev->next;
	} else { /* case 2: removing tail */
	    prev->next = ref->next;
	    ref->next->prev = prev;
	    prev->next->prev = prev;
	    current_queue->last = prev;
	    current_queue->head->prev = current_queue->last;

	}
    }
    free_node(node);
    return 0;
}
/* free_node
 *  DESCRIPTION:  disassociates next and prev references from the node. 
 *		  Sets node to NULL.
 *  INPUTS:       node -- node to free
 *  OUTPUTS:      none
 *  SIDE EFFECTS: sets prev and next pointers to NULL. points node to NULL.
 *  RETURN VALUE: none
 */
void free_node(list_head_t* node)
{
    if (node == NULL)
	return;
    node->next = NULL;
    node->prev = NULL;
    node = NULL;
    return;
}
/*
 * insert_front
 *  DESCRIPTION:  inserts a node to the front of the list pointed to
 *                by current_queue.
 *  INPUTS:       node -- pointer to node to insert
 *  OUTPUTS:      none
 *  SIDE EFFECTS: inserts node at the front of the list
 *  RETURN VALUE: none
 */
int32_t insert_front(list_head_t* node) {
    if (node == NULL)
	return -1;
    if (is_empty()) {
	head = node;
    }
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
    node->prev = head;
    last = node;
    return 0;
}

/*
 * insert_back
 *  DESCRIPTION:  inserts a node to the back of the list pointed to
 *                by current_queue.
 *  INPUTS:       node -- pointer to node to insert
 *  OUTPUTS:      none
 *  SIDE EFFECTS: inserts node at the back of the list
 *  RETURN VALUE: 0 on success, -1 on fail.
 */
int32_t insert_back(list_head_t* node)
{
    if (node == NULL)
	return -1;
    /* if the queue is empty... establish head/last pointers */
    if (is_empty()) {
	current_queue->head = node;
	current_queue->last = node;
	return 0;
    }
       else { /* insert node at end of list */
	   current_queue->last->next       = node;
	   node->prev                      = current_queue->last;
	   current_queue->last             = current_queue->last->next;
       } /* check if node is last and update last pointer in queue */
       if (current_queue->last != NULL && current_queue->last != node) {
	   current_queue->last = node;
       }
       /* establish forward and previous links between last and head of queue */
       current_queue->last->next = current_queue->head;
       current_queue->head->prev = current_queue->last;

       return 0;
}

/*
 * print_list
 *  DESCRIPTION:  prints the address of each entry in the list
 *                starting from head and up until last.
 *  INPUTS:       head -- pointer to node to start traversing.
 *  OUTPUTS:      prints the addresses of each element during the traversal.
 *  SIDE EFFECTS: none
 *  RETURN VALUE: none
 */
void print_list(struct list_head* head)
{
    if (head == NULL)
	return;
    struct list_head* n = head;
    while (n  != NULL) {
	if (n != NULL) {
	    vga_printf("%x -> ", n);
	}
	n = n->next;
	if (n->next == head) {
	    vga_printf("%x\n", n);
	    return;
	}
    }
}
/* is_empty
 *  DESCRIPTION : used to indicate whether list is empty (ie head is not set)
 *  INPUTS      : none
 *  OUTPUTS     : none
 *  RETURN VALUE: 0 if head is set. 1 if head is NULL.
 *  SIDE EFFECTS: none
 */
int32_t is_empty() {
    return (head == NULL);
}
/* length
 *  DESCRIPTION : used to indicate length of linked list (# nodes between 
 *                head and last, inclusive).
 *  INPUTS      : none
 *  OUTPUTS     : none
 *  RETURN VALUE: # nodes between head and last nodes of list, inclusive. 
 *                0 if empty.
 *  SIDE EFFECTS: none
 */
int32_t length() {
    int32_t len = 0;
    struct list_head* curr;
    for (curr = head; curr->next != head; curr = curr->next) {
	len++;
    }
    return len;
}
#endif
