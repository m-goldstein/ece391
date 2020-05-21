#ifndef SHELL_H
#define SHELL_H
#include "sys_call.h"
#include "task.h"
#include "sys.h"
#include "sched.h"
#include "memory.h"
#include "terminal.h"
extern void* init_shell(); /* initialize a shell instance */
extern void* attach_shell(uint32_t term_id); /* initialize a shell instance and attach it a tty session */
//extern queue_t* current_queue;
#endif
