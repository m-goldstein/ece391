#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "include/vga.h"
#include "include/keyboard.h"
#include "include/terminal.h"
#include "include/sched.h"

// https://www.win.tue.nl/~aeb/linux/kbd/scancodes-11.html
// http://www.philipstorr.id.au/pcbook/book3/scancode.htm

// Use to signify a key on the keyboard that is unshowable on the screen
static char unshow[] = {0xFF,0xFF,0xFF,0xFF};

// 0xFF is a key un-representable on the screen
// Converts the scan code in the keyboard register to ASCII characters
static char* key_press[] =
{
    unshow, unshow, "!1!1", "@2@2",  "#3#3",  "$4$4",  "%5%5",  "^6^6",  "&7&7",  "*8*8",  "(9(9",  ")0)0",  "_-_-",  "+=+=",  unshow, "    ",
    "qQQq",  "wWWw",  "eEEe",  "rRRr",  "tTTt",  "yYYy",  "uUUu",  "iIIi",  "oOOo",  "pPPp",  "{[{[",  "}]}]",  "\n\n\n\n", unshow, "aAAa",  "sSSs" ,
    "dDDd",  "fFFf",  "gGGg",  "hHHh",  "jJJj",  "kKKk",  "lLLl",  ":;:;",  "\"\'\"\'", "~`~`", unshow,  "|\\|\\",  "zZZz",  "xXXx", "cCCc", "vVVv" ,
    "bBBb",  "nNNn",  "mMMm",  "<,<,",  ">.>.",  "?/?/", unshow, unshow, unshow, "    ",  unshow, unshow, unshow, unshow, unshow, unshow,
    unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow,
    unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow, unshow
};

// Flags for some special keys
uint32_t caps_flag              = 0;
uint32_t shift_flag             = 0;
uint32_t ctrl_flag              = 0;
uint32_t alt_flag               = 0;
uint32_t activate_inter_flag    = 0;
uint32_t keyboard_write         = 0;
volatile uint32_t f2_key_flag   = 0;
volatile uint32_t f3_key_flag   = 0;
/*
 * keyboard_init
 *   DESCRIPTION: initializes the keyboard with the IDT table and enables IRQ for keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes the keyboard
 */
void keyboard_init()
{
    uint32_t cliflags;
    cli_and_save(cliflags);
    unsigned short seg_size = KERNEL_CS; // The segment is KERNEL_CS bc this stuff is in kernel
    unsigned short flags = 0x8E00; // This flag corresponds to a Interrupt gate with 0 DPL,
    // present flag to 1, and gate size 1 (32-bits)

    // Reset all flags
    caps_flag  = 0;
    shift_flag = 0;
    ctrl_flag   = 0;
    alt_flag = 0;
    activate_inter_flag = 0;
    f2_key_flag = 0;
    f3_key_flag = 0;

    fill_interrupt(KEYBOARD_INTERRUPT_VEC,(uint32_t*)keyboard_linkage,seg_size,flags);
    enable_irq(KEYBOARD_IRQ);
    restore_flags(cliflags);

    return;
}

/*
 * keyboard_handler
 *   DESCRIPTION: reads the scan code in the keyboard register and deals with it
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: deals with a keyboard interrupt
 */

terminal_session_t* current_term;
void keyboard_handler()
{
    cli();
    uint8_t keycode;
    uint8_t ascii_conversion;
    uint32_t i;
    key_t* entry;
    keycode = inb(KEYBOARD_CMD_STAT_PORT);
    entry = (key_t*)key_press[keycode];
    if(keycode == ALTKEY)
	alt_flag = 1;
    // If a key is unpressed make sure that the flags are set accordingly
    if((int8_t) keycode < 0) //Unpress the key
    {
	keycode &= (uint8_t)UNPRESSMASK;

	if(keycode == LEFTSHIFT || keycode == RIGHTSHIFT)
	    shift_flag = 0;
	else if(keycode == LEFTCTRL || keycode == RIGHTCTRL)
	    ctrl_flag  = 0;
	else if(keycode == ALTKEY)
	    alt_flag = 0;
    }
    else if(keycode == CAPSLOCK) // For caps lock its another press to change the state
	caps_flag ^= 0x01;
    else if(keycode == F1KEY){ // Press F1 activates/deactivates the RTC interupt
	if(alt_flag == 1) {
	    term_switch(0); // Swtich to terminal 0
	}
	else
	    activate_inter_flag ^= 0x01;
    }
    else if(keycode == F2KEY){ // Press F2 activates/deactivates something
	if(alt_flag == 1) {
	    term_switch(1); // Swtich to terminal 1
	}
	else
	    f2_key_flag ^= 0x01;
    }
    else if(keycode == F3KEY){ // Press F3 to print the buffer
	if(alt_flag == 1) {
	    term_switch(2); // Swtich to terminal 2
	}
	else {
	    f3_key_flag ^= 0x01;
	    //vga_printf("\nKeeb Buffer: %d\n", getCurrentSession()->index); code used to count how many char were in the current buffer
	}
    }
    else if(keycode == LEFTSHIFT || keycode == RIGHTSHIFT)
	shift_flag = 1;
    else if(keycode == LEFTCTRL || keycode == RIGHTCTRL)
	ctrl_flag  = 1;
    else if(keycode == CAPSLOCK)
	caps_flag  = 1;


    else // This now deals with any key that is showable on the screen
    {
	if(keycode == ENTERKEY)
	    ascii_conversion = '\n';
	else if(keycode>MAX_KEY_PRESS)
	{
	    send_eoi(KEYBOARD_IRQ);
	    sti();
	    return;
	}
	else if(*(uint32_t*)entry == *(uint32_t*)unshow  && (keycode != BACKSPACEKEY)){
	    send_eoi(KEYBOARD_IRQ);
	    sti();
	    return;
	}
	else if(shift_flag == 1 && caps_flag == 1)
	    ascii_conversion = entry->shiftCapsChar;
	else if(shift_flag == 1)
	    ascii_conversion = entry->upperChar;
	else if(caps_flag == 1)
	    ascii_conversion = entry->capsChar;
	else
	    ascii_conversion = entry->lowerChar;

	current_term = getCurrentSession();
	pid_t* hpid = get_current_htable_entry();
	proc_t* current_pcb = &hpid->pcb;
	if(current_term->en == 1)
	{
	    save_vga_state_NO_MEMORY(&sessions[current_pcb->terminal_id].vga);
	    restore_vga_state_NO_MEMORY(&sessions[current_session].vga);
	    vga_mem_base = VIDEO_MEM_START;
	    keyboard_write = 1;
	    if((keycode == LKEY) && ctrl_flag) // Clear screen bc of ctrl-L
	    {
		if(terminal_reading == 0)
		{
		    vga_ctrl_L();
		    current_term->index = 0;
		}
		else
		{
		    vga_ctrl_L();
		    for(i = 0; i<current_term->index;i++)
			vga_putc(*((uint8_t*)(current_term->buffer+i)));
		}
	    }
	    else if(keycode == TABKEY)
	    {
		ascii_conversion = ' ';
		if(TERMINAL_BUF_SIZE-5 < current_term->index){} // Checks if there is enough room for a tab(tab size is 4 + 1 for /n)
		else
		{
		    for(i=0;i<TAB_SIZE;i++)
		    {
    			current_term->buffer[(current_term->index)++] = ascii_conversion;
    			vga_putc(ascii_conversion);
		    }
		}
	    }
	    else if(keycode == BACKSPACEKEY && current_term->index != 0) // You can backspace as long as there are characters
	    {
		(current_term->index)--;
		vga_backspace();
	    }
	    else if(keycode == ENTERKEY && current_term->enter != 1) // If enter key or one line before the max terminal size go to next line
	    {
    		ascii_conversion = '\n';
    		current_term->enter = 1;
    		current_term->buffer[(current_term->index)++] = ascii_conversion;
    		vga_putc(ascii_conversion);
	    }
	    else if(current_term->enter == 1){}
	    else if(current_term->index != TERMINAL_BUF_SIZE-1 && keycode != BACKSPACEKEY) // As long there is room you can put characters in the buffer
	    {
    		current_term->buffer[(current_term->index)++] = ascii_conversion;
    		vga_putc(ascii_conversion);
	    }
	}

    	keyboard_write = 0;
    	save_vga_state_NO_MEMORY(&sessions[current_session].vga);
    	restore_vga_state_NO_MEMORY(&sessions[current_pcb->terminal_id].vga);

      vga_mem_base =(uint32_t) sessions[current_pcb->terminal_id].vga.screen_start;

    }
    // Send EOI to the keyboard
    send_eoi(KEYBOARD_IRQ);
    sti();
    return;
}
#endif
