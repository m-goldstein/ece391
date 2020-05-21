#ifndef SCHED_H
#define SCHED_H
#include "types.h"
#include "task.h"
#include "terminal.h"
#include "vga.h"
#include "lib.h"
#define NUM_REAL_TIME_P     100
#define NUM_REGULAR_P       40
#define MAX_NUM_P           NUM_REAL_TIME_P+NUM_REGULAR_P
/* runqueue structure */
struct runqueue {
    uint8_t  lock;       /* the lock   */
    uint32_t n_runnable; /* # runnable */
    uint32_t n_switches; /* # switches */
    uint32_t timestamp;
    proc_t* current_pcb;
    proc_t* idle_pcb;
    prio_array_t* active_array;  /* pointer to active array */
    prio_array_t* expired_array; /* pointer to expired array */
    prio_array_t  first_array;   /* actual data for first priority array */
    prio_array_t  second_array;  /* actual data for second priority array*/
};
typedef struct runqueue runqueue_t;
extern runqueue_t runqueue;
extern void init_runqueue(); /* initialize runqueue */
extern int32_t switch_task(list_head_t* node); /* perform task switching */
extern proc_t* current_proc;
#endif
