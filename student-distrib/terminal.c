#ifndef TERMINAL_C
#define TERMINAL_C
#include "include/terminal.h"
#include "include/keyboard.h"
#include "include/lib.h"
#include "include/sched.h"
#include "include/memory.h"
terminal_session_t sessions[MAX_NUM_TERMINALS];
uint8_t session_buffers[MAX_NUM_TERMINALS][TERMINAL_BUF_SIZE];
uint32_t current_session = 0;
int session_counter = 0;
uint8_t terminal_reading = 0;
uint8_t shell_showing = 1;
static uint8_t session_colors[] = { (BLACK <<4) + WHITE,
 														        (GREEN << 4) + BLACK,
													          (WHITE << 4) + RED
													 }; // Offset by 4 for fg and bg colors
/*
 * clear_terminal_buffer
 *   DESCRIPTION: clears a terminal sessions buffer
 *   INPUTS: index - pointer to the index to reset
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: actually just moves the index back to 0
 */
void clear_terminal_buffer(int32_t id)
{
    if (id < 0 || id > 8) //Boundary check for the id max of 8 terminals and can't have negative terminal
	return;

    if (&sessions[id] == NULL)
	return;

    vga_ctrl_L();
    sessions[id].index = 0;
}

/*
 * terminal_write
 *   DESCRIPTION: writes to terminal buffer and screen when this function is called
 *   INPUTS: fd - pointer to the file descriptor
 *           buffer - pointer to the buffer to print
 *           length - the number of characters to print to screen
 *   OUTPUTS: none
 *   RETURN VALUE: returns the number of bytes written
 *                 returns -1 if the buffer pointer is NULL
 *   SIDE EFFECTS: writes to screen length number of characters from buffer
 */
int32_t terminal_write(int32_t fd, uint8_t* buffer, uint32_t length)
{
  cli();
  int i;
  int num_char_2_screen= 0;
  // pid_t* hpid = get_current_htable_entry();
  // proc_t* current_pcb = &hpid->pcb;
  if(current_proc->terminal_id == current_session)
    vga_mem_base = VIDEO_MEM_START;
  else
    vga_mem_base = (uint32_t)sessions[current_proc->terminal_id].vga.screen_start;
  if (current_proc == NULL)
      return -1;
  if (sessions[current_proc->terminal_id].en == 0)
      return -1;
  if (CHECK_MSB(length)) // Check that it is negative
      return -1;
  if(buffer == NULL)
      return -1;
  for(i = 0; i<length;i++)
  {
      if(*(buffer+i) != NULL) // Do not print NULL characters
      {
	  vga_putc( *(buffer+i));
	  //vga_putc(*(buffer+i));
	  num_char_2_screen++;
      }
  }
  sti();
  return num_char_2_screen;
}
/*
 * terminal_open
 *   DESCRIPTION: opens terminal
 *   INPUTS: filename - not used
 *   OUTPUTS: 0 on success
 *   RETURN VALUE: returns 0 on success
 *   SIDE EFFECTS: opens terminal with number fd
 */
int32_t terminal_open(int32_t id, uint8_t* filename, uint32_t length)
{
    // sessions[0].index = 0;
    // sessions[0].en = 1;
    // sessions[0].enter = 0;
    // sessions[0].buffer = session_buffers[0];
    //current_session = 0;

    return 0;
}

/*
 * terminal_close
 *   DESCRIPTION: closes terminal 1
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: closes terminal 1
 */
int32_t terminal_close(int32_t id, uint8_t* filename, uint32_t length)
{
    sessions[0].en  = 0;
    // sessions[0].vga = NULL;
    return 0;
}

/*
 * terminal_read
 *   DESCRIPTION: read terminal 1
 *   INPUTS: fd - pointer to the file descriptor
 *           buffer - pointer to the buffer to print to
 *           length - not used
 *   OUTPUTS: none
 *   RETURN VALUE: returns the number of bytes written
 *                 returns -1 if the buffer pointer is invalid
 *   SIDE EFFECTS: reads terminal 1
 */
proc_t* current_pcb;
pid_t* hpid;
int32_t terminal_read(int32_t id, uint8_t* buffer, uint32_t length)
{
    uint32_t num_to_copy;

    if(buffer == NULL || CHECK_MSB(length))
	   return -1;

    if (current_proc == NULL)
	return -1;
    if (sessions[current_proc->terminal_id].en == 0)
	return -1;
    terminal_reading = 1;
    while(sessions[current_proc->terminal_id].enter == 0);
    cli();
    if(length < sessions[current_proc->terminal_id].index)
	num_to_copy = length;
    else
	num_to_copy = sessions[current_proc->terminal_id].index;

    sessions[current_proc->terminal_id].index = 0;
    sessions[current_proc->terminal_id].enter = 0;

    // Copy all data from the terminal keyboard buffer
    memcpy((void*)buffer, sessions[current_proc->terminal_id].buffer, num_to_copy);
    if (num_to_copy > 0) {
	buffer[num_to_copy-1] = '\n'; //Ensure that you always end on a '\n'
    } else if (num_to_copy == 0) {
	buffer[num_to_copy] = '\n';
    }
    terminal_reading = 0;
    sti();
    return num_to_copy;
}

/*
 * clear_all_terminals
 *   DESCRIPTION: clear all possible terminal variables
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: as DESCRIPTION
 */
void clear_all_terminals()
{
    current_session = 0;
    sessions[current_session].en = 0;
    sessions[current_session].enter = 0;
}
/*
 * init_terminal
 *   DESCRIPTION: initializes a terminal
 *   INPUTS: number - the number that determines what terminal to init
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes a terminal
 */
io_table_t term_ops_table = { &terminal_open, &terminal_close, &terminal_read, &terminal_write};
void init_terminal(int32_t number)
{
    if (number >= 0 && number < MAX_NUM_TERMINALS) {
	sessions[number].buffer = session_buffers[number];
	sessions[number].id     = number;
	sessions[number].op_ptr = &term_ops_table;
	sessions[number].en     = 1;
	sessions[number].enter  = 0;
	sessions[number].vga.screen_start = (uint8_t*)PHYS_ADDR_START_TERM(number);
	sessions[number].vga.fg = (session_colors[number] >> 4) & 0xF; // Top 4 bits if fg color
	sessions[number].vga.bg = session_colors[number] & 0xF; // Bottom 4 bits is bg color
	sessions[number].vga.x_pos = 0;
	sessions[number].vga.y_pos = 0;
	sessions[number].queue     = NULL;
	current_session = number;

	//save_term_vga_state();
	// init_term_vga(number);
	return;
    }
    else {
	printf("Session number %d not supported", number);
	return;
    }
    return;
}

/* FUNCTION DEPRECIATED
 * getCurrentSession
 *   DESCRIPTION: returns pointer to current session
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: returns a pointer to the current session
 *   SIDE EFFECTS: returns NULL is there are no sessions
 */
terminal_session_t* getCurrentSession()
{
    if(current_session < MAX_NUM_TERMINALS && 0 <= current_session)
	     return &sessions[current_session];
    return 0;
}

/*
 * parse_user_input
 *  DESCRIPTION: parses integers from a buffer
 *  INPUTS     : input   -- the bufer with the data to parse
 *               length  -- number of charachters in the buffer to parse
 *  OUTPUTS      : none
 *  RETURN VALUE : the decimal value of the input
 *  SIDE EFFECTS : none
 */
uint8_t parse_user_input(uint8_t* input, int length)
{
    if (input == NULL)
	return 0;
    uint8_t count = 0;
    uint8_t c;
    int i;
    for (i = 0; i < length; ++i) {
	if (input[i] == '\0' || input[i] == '\n')
	    return count;
	c = input[i];
	if (c >= '0' && c <= '9')
	    count = 10 * count + (c - '0');
    }
    return count;
}
/*
 * term_switch
 *  DESCRIPTION: switches the shown terminal that you selected and saves the context of the previous terminal
 *               NOTE: the terminals are base 0
 *  INPUTS     : term_to_switch   -- the terminal to be showen
 *  OUTPUTS      : none
 *  RETURN VALUE : none
 *  SIDE EFFECTS : changes the shown terminal to the one selected
 */
void term_switch(uint32_t term_to_switch)
{
    cli();
    if(term_to_switch == current_session || MAX_NUM_TERMINALS <= term_to_switch) // No switch if the terminal to switch to is the same
	     return;
    // Save the current screen of the current session
    terminal_session_t*  current_term = &sessions[current_session];
    terminal_session_t*  next_term    = &sessions[term_to_switch];
    save_vga_state(&(current_term->vga));

    // If the process you're switching to is vidmapped then you need to switch the virtual mem
    // address for the vidmap to point to real video memory
    if(current_proc->terminal_id == term_to_switch && current_proc->is_vidmapped)
    {
      __map_user_page(VIDEO_START_ADDR, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN);
      flush_tlb();
    }

    switch_terminals(term_to_switch);
    restore_vga_state(&(next_term->vga));

    //If the shell is showing then a special ctr_l is used in the vga
    uint8_t* showen_proc_command =  ((pid_t*)(next_term->queue->last->entry))->pcb.command;
    if(strncmp( (const int8_t*)showen_proc_command , "shell", strlen((int8_t*) showen_proc_command)) == 0)
      shell_showing = 1;
    else
      shell_showing = 0;
    sti();
    return;
}

/*
 * switch_terminals
 *   DESCRIPTION: switches to a terminal
 *   INPUTS: number - the terminal to switch to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: switches to a terminal
 */
void switch_terminals(int32_t number)
{
    // save_term_vga_state();
    current_session = number;
    // restore_term_vga_state();
}
#endif
