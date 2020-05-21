#ifndef TERMINAL_H
#define TERMINAL_H

#include "lib.h"
#include "x86_desc.h"
#include "vga.h"
#include "sys_call.h"
#include "sys.h"
#define TERMINAL_BUF_SIZE     128
#define MAX_NUM_TERMINALS       3
#define ASCII_BACKSPACE      0x08
#define ASCII_ENTER          0x0A   // This is actually newline ASCII
#define ASCII_L_UPPER        0x4C
#define ASCII_L_LOWER        0x6C
#define TERMINAL_IO_OPS      0x04
#define BLACK	     					  0x0
#define BLUE	      				  0x1
#define GREEN	      				  0x2
#define CYAN	                0x3
#define RED	       						0x4
#define MAGENTA	       				0x5
#define BROWN	       					0x6
#define LIGHT_GREY     				0x7
#define DARK_GREY      				0x8
#define LIGHT_BLUE    				0x9
#define LIGHT_GREEN    				0xA
#define LIGHT_CYAN     				0xB
#define LIGHT_RED      				0xC
#define LIGHT_MAGENTA  				0xD
#define YELLOW         				0xE
#define WHITE	       					0xF
#define PHYS_ADDR_START_TERM(x)	    ((VIDEO_MEM_START)+ 2*__4KB__+(x)*(__4KB__)) // An extra 4KB room to distance from video memory

/* Enumeration to assign integer values to temrinal I/O */
enum term_io_operations {
	TERM_OPEN     = 0,
	TERM_CLOSE    = 1,
	TERM_READ     = 2,
	TERM_WRITE    = 3
};
// Struct for a terminal session
struct terminal_session {
    uint8_t*    buffer;    // Buffer of that terminal
    uint8_t     en    ;    // Whether or not that terminal is enabled
    uint32_t    index ;    // Current index of the terminal buffer
    uint32_t    enter ;    // Whether or not enter was pressed
    uint32_t    id    ;
    vga_t	vga   ;
    queue_t*    queue ;
    io_table_t* op_ptr;
} __attribute__((packed));

typedef struct terminal_session terminal_session_t;

/* On startup you should clear all terminals*/
extern void clear_all_terminals();
extern void clear_terminal_buffer(int32_t index);
/*Choose to init one terminal*/
extern void init_terminal(int32_t number);

/*Switch to a different terminal*/
extern void switch_terminals(int32_t number);

/*Variable for what you're current session is*/
extern uint32_t current_session;

/*Whether or not the terminal is reading*/
extern uint8_t terminal_reading;

/*get whatever is the current terminal session*/
extern terminal_session_t* getCurrentSession();

/*session struct*/
extern terminal_session_t sessions[MAX_NUM_TERMINALS];
extern uint8_t session_buffers[MAX_NUM_TERMINALS][TERMINAL_BUF_SIZE];

extern int32_t save_term_vga_state();
extern int32_t restore_term_vga_state();

extern int32_t terminal_write(int32_t fd, uint8_t* buffer, uint32_t length);
extern int32_t terminal_read(int32_t fd, uint8_t* buffer, uint32_t length);
extern int32_t terminal_open(int32_t fd, uint8_t* filename, uint32_t length);
extern int32_t terminal_close(int32_t fd, uint8_t* filename, uint32_t length);
extern void term_switch(uint32_t term_to_switch);
extern uint8_t parse_user_input(uint8_t* input, int length); /* helper function to convert charachters in buffer to integers */

extern int session_counter;
extern uint8_t shell_showing;
#endif
