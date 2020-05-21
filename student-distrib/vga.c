#ifndef VGA_C
#define VGA_C
#include "include/sys_call.h"
#include "include/vga.h"
#include "include/memory.h"
#include "include/task.h"
#include "include/terminal.h"
#include "include/sched.h"
#include "include/pit.h"
#include "include/keyboard.h"
#define VGA_FAIL -1
#define VGA_SUCCESS 0
#define ATTRIBUTE (((BLACK) << 4) | ((WHITE) & 0xF))
#define REP_OUTSW(port, source, count)                              \
do {                                                                \
    asm volatile ("                                               \n\
        1: movw 0(%1), %%ax                                       \n\
        outw %%ax, (%w2)                                          \n\
        addl $2, %1                                               \n\
        decl %0                                                   \n\
        jne 1b                                                    \n\
        "                                                           \
        : /* no outputs */                                          \
        : "c"((count)), "S"((source)), "d"((port))                  \
        : "eax", "memory", "cc"                                     \
	);                                                          \
} while (0)

/* macro used to write two bytes to two consecutive ports */
#define OUTW(port, val)                                             \
do {                                                                \
    asm volatile ("outw %w1, (%w0)"                                 \
        : /* no outputs */                                          \
        : "d"((port)), "a"((val))                                   \
        : "memory", "cc"                                            \
    );                                                              \
} while (0);

uint32_t vga_mem_base = VIDEO_MEM_START;

uint8_t  foreground = LIGHT_BLUE;
uint8_t  background = BLACK;
int16_t x_pos = 0;
int16_t y_pos = 0;
//crtc registers needed to set the cursor
unsigned short qemu_crtc_reg[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5504, 0x8105, 0xBF06, 0x1F07,
    0x0008, 0x4F09, 0x0D0A, 0x0E0B, 0x000C, 0x000D, 0x000E, 0x010F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x1F14, 0x9615, 0xB916, 0xA317,
    0xFF18
};
vga_table_t vga_table;

/* init_vga
 *  DESCRIPTION : initialize VGA context and registers 
 *  INPUTS      : none
 *  OUTPUTS     : none
 *  RETURN VALUE: none
 *  SIDE EFFECT : initializes VGA registers to values defined in qemu_rtc_reg array
 */
int32_t init_vga()
{
    OUTW(VGA_IO_ADDR, PROTECTION_BIT_MASK);
    REP_OUTSW(VGA_IO_ADDR, qemu_crtc_reg, NUM_CRTC_REGS);
    x_pos = get_cursor_pos() % CHARS_PER_LINE;
    y_pos = get_cursor_pos() / CHARS_PER_LINE;
    //__map_page_directory(vga_mem_base, USER_VIDEO_MEM_ADDR, PRESENT | RW_EN | USER_EN | EXTENDED_PAGING);
    vga_table.bitmap = 0x00;
    return VGA_SUCCESS;
}

/* vga_putc_color
 *  DESCRIPTION : parameterized putc function
 *  INPUT       : c -- charachter to display
 *                fg -- vga foreground color 
 *                bg -- vga background color
 *                x  -- cursor x position
 *                y  -- cursor y position
 *  OUTPUT      : prints charachter c to screen with attribute specified by fg
 *                and bg and coordinates given by x and y.
 *  RETURN VALUE: returns 0 if successful
 */
int32_t vga_putc_color(uint8_t c, uint8_t fg, uint8_t bg, int32_t x, int32_t y)
{

    if (c == '\n' || c == '\r') {
	x = 0;
	y += 1;
    } else {
	uint16_t attr = (bg << 4) | (fg & 0xF); // bitshift 4 for the bacground and 0xF mask to get the bottom 4 bits
	volatile uint16_t* to;
	to = (volatile uint16_t*)vga_mem_base + VGA_OFFSET(x,y);
	*to = c | (attr << 8); // Bit shift 8 to set attributes
	x += 1;
	y = (y + (x / CHARS_PER_LINE)) % (LINES_PER_PANE + 1);
	x %= CHARS_PER_LINE;
    }
    if (y == LINES_PER_PANE) {
	vga_scroll_screen();
    }
    vga_update_cursor();
    return VGA_SUCCESS;
}

/* vga_putc
 *  DESCRIPTION : putc function
 *  INPUT       : c -- charachter to display
 *  OUTPUT      : prints charachter c to screen with default attribute and at
 *                and at current cursor positon 
 *  RETURN VALUE: returns 0 if successful
 */
int32_t vga_putc(uint8_t c)
{
    if (c == '\n' || c == '\r') {
	x_pos = 0;
	y_pos += 1;
    } else {
	    uint16_t attr = (background << 4) | (foreground & 0xF); // bitshift 4 for the bacground and 0xF mask to get the bottom 4 bits
	    volatile uint16_t* to;
	    to = (volatile uint16_t*)vga_mem_base + VGA_OFFSET(x_pos,y_pos);
	    *to = c | (attr << 8); // Bit shift 8 to set attributes
	    x_pos += 1;
	    y_pos = (y_pos + (x_pos / CHARS_PER_LINE)) % (LINES_PER_PANE+1);
	    x_pos %= CHARS_PER_LINE;
    }
    if (y_pos == LINES_PER_PANE) {
	vga_scroll_screen();
    }
    if(current_proc->terminal_id == current_session || keyboard_write == 1)
      vga_update_cursor();
    return VGA_SUCCESS;
}

/* vga_tuple_putc
 *  DESCRIPTION : parameterized putc function
 *  INPUT       : c -- charachter to display
 *                x  -- cursor x position
 *                y  -- cursor y position
 *  OUTPUT      : prints charachter c to screen with default attribute values
 *                and coordinates given by x and y.
 *  RETURN VALUE: returns 0 if successful
 */
int32_t vga_tuple_putc(uint8_t c, int32_t x, int32_t y)
{
    if (c == '\n' || c == '\r') {
	x = 0;
	y += 1;
    } else {
	uint16_t attr = (background << 4) | (foreground & 0xF); // bitshift 4 for the bacground and 0xF mask to get the bottom 4 bits
	volatile uint16_t* to;
	to = (volatile uint16_t*)vga_mem_base + VGA_OFFSET(x,y);
	*to = c | (attr << 8); // Bit shift 8 to set attributes
	x += 1;
	y = (y + (x / CHARS_PER_LINE)) % (LINES_PER_PANE + 1);
	x %= CHARS_PER_LINE;
    }
    if (y == LINES_PER_PANE) {
	vga_scroll_screen();
    }
    vga_update_cursor();


    return VGA_SUCCESS;
}

/* vga_puts
 *  DESCRIPTION : Makes consecutive calls to vga_putc as long as current charachter
 *                is not NULL or null-terminator
 *  INPUT       : s -- charachter buffer (pointer)
 *  OUTPUT      : displays charachters in buffer sequentally
 *  RETURN VALUE: returns number of charachters printed
 *  SIDE EFFFECT: updates cursor position
 */
int32_t vga_puts(int8_t* s)
{
    register int32_t index = 0;
    while (s[index] != '\0' && s != NULL) {
	vga_putc(s[index]);
	index++;
    }
    return index;
}

/* void vga_backspace(void);
 * Inputs: void
 * Return Value: none
 * Function: echos the backspace key from the keyboard 
 */
void vga_backspace()
{
  if(x_pos == 0 && y_pos == 0) // If there are no characters don't backspace
    return;
  x_pos--;
  if(x_pos < 0) // If it's at the left edge wrap around to the row above
  {
    x_pos = CHARS_PER_LINE - 1;
    y_pos--;
  }
  // Update that space with a blank
  *(uint8_t *)(vga_mem_base + ((VGA_OFFSET(x_pos,y_pos)) << 1)) = ' ';
  *(uint8_t *)(vga_mem_base + ((VGA_OFFSET(x_pos,y_pos) << 1) + 1)) = ((background << 4) | (foreground & 0xF)); // bitshift by four to set background and mask bottom 4 bits of foregorund for foreground
  vga_update_cursor();
}

/* void vga_update_cursor(void);
 * Inputs: void
 * Return Value: none
 * Function: updates the cursor's location */
void vga_update_cursor()
{
  qemu_crtc_reg[VGA_CURSOR_LOC_LOW] = VGA_CURSOR_LOC_LOW + (((VGA_OFFSET(x_pos, y_pos)&0xFF) << 8)); // 0xFF bit mask for lower 8 bits
  qemu_crtc_reg[VGA_CURSOR_LOC_HIGH] = VGA_CURSOR_LOC_HIGH + ((VGA_OFFSET(x_pos,y_pos)&0xFF00)); // 0xFF bit mask for upper 8 bits
  /* clear protection bit to enable write access to first few registers */
  OUTW(VGA_IO_ADDR, PROTECTION_BIT_MASK);
  REP_OUTSW(VGA_IO_ADDR, qemu_crtc_reg, NUM_CRTC_REGS);
  return;
}

/* void vga_clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void vga_clear(void)
{
    int32_t i;
    for (i = 0; i < VGA_FRAME_SIZE; i++) {
        *(uint8_t *)(vga_mem_base + (i << 1)) = ' ';
        *(uint8_t *)(vga_mem_base + (i << 1) + 1) = ((background << 4) | (foreground & 0xF)); // bitshift 4 for the bacground and 0xF mask to get the bottom 4 bits
    }
}

/* void vga_bg_fg_reset(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void bg_fg_reset(void)
{
    int32_t i;
    for (i = 0; i < VGA_FRAME_SIZE; i++) {
        *(uint8_t *)(VIDEO_MEM_START + (i << 1) + 1) = ((background << 4) | (foreground & 0xF)); // bitshift 4 for the bacground and 0xF mask to get the bottom 4 bits
    }
}

/* vga_ctrl_L
 *  DESCRIPTION : ctrl-L implementation for vga sessions
 *  INPUT       : none
 *  OUTPUTS     : clears screen and displays shell prompt
 *  RETURN VALUE: none
 *  SIDE EFFECTS: updates cursor position
 */
void vga_ctrl_L()
{
    vga_clear();
    x_pos = 0;
    y_pos = 0;
    vga_update_cursor();
    if( PIT_tick >= 1 && shell_showing == 1) // Special ctrl-L for shell
      vga_printf("391OS> ");
}

/* VGA printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t vga_printf(int8_t *format, ...)
{
    if (format == NULL)
	return -1;
    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
	switch (*buf) {
	    case '%':
		{
		    int32_t alternate = 0;
		    buf++;

format_char_switch:
		    /* Conversion specifiers */
		    switch (*buf) {
			/* Print a literal '%' character */
			case '%':
			    vga_putc('%');
			    break;

			    /* Use alternate formatting */
			case '#':
			    alternate = 1;
			    buf++;
			    /* Yes, I know gotos are bad.  This is the
			     * most elegant and general way to do this,
			     * IMHO. */
			    goto format_char_switch;

			    /* Print a number in hexadecimal form */
			case 'x':
			    {
				int8_t conv_buf[64];
				if (alternate == 0) {
				    itoa(*((uint32_t *)esp), conv_buf, 16);
				    vga_puts(conv_buf);
				} else {
				    int32_t starting_index;
				    int32_t i;
				    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
				    i = starting_index = strlen(&conv_buf[8]);
				    while(i < 8) {
					conv_buf[i] = '0';
					i++;
				    }
				    vga_puts(&conv_buf[starting_index]);
				}
				esp++;
			    }
			    break;

			    /* Print a number in unsigned int form */
			case 'u':
			    {
				int8_t conv_buf[36];
				itoa(*((uint32_t *)esp), conv_buf, 10);
				vga_puts(conv_buf);
				esp++;
			    }
			    break;

			    /* Print a number in signed int form */
			case 'd':
			    {
				int8_t conv_buf[36];
				int32_t value = *((int32_t *)esp);
				if(value < 0) {
				    conv_buf[0] = '-';
				    itoa(-value, &conv_buf[1], 10);
				} else {
				    itoa(value, conv_buf, 10);
				}
				vga_puts(conv_buf);
				esp++;
			    }
			    break;

			    /* Print a single character */
			case 'c':
			    vga_putc((uint8_t) *((int32_t *)esp));
			    esp++;
			    break;

			    /* Print a NULL-terminated string */
			case 's':
			    vga_puts(*((int8_t **)esp));
			    esp++;
			    break;

			default:
			    break;
		    }

		}
		break;

	    default:
		vga_putc(*buf);
		break;
	}
	buf++;
    }
    return (buf - format);
}

/* vga_scroll_screen(void);
 * Inputs: void
 * Return Value: none
 * Function: scrolls the terminal upwards
 */
void vga_scroll_screen()
{
    int i;
    memmove((void*)vga_mem_base, (const void*)vga_mem_base + (2*CHARS_PER_LINE),
	    (CHARS_PER_LINE * (LINES_PER_PANE-1))* 2);
    for(i = 0; i < (CHARS_PER_LINE * 2); i++)
    {
	if(i%2 == 0)
	    memset((uint8_t *)(vga_mem_base + i + (CHARS_PER_LINE * (LINES_PER_PANE-1) * 2)), (uint32_t)' ', 1);
	else {
	    memset((uint8_t *)(vga_mem_base + i + (CHARS_PER_LINE * (LINES_PER_PANE - 1) * 2)), (uint32_t)(((background << 4) | (foreground & 0xF))), 1);
	}
    }

    x_pos = 0;
    y_pos = LINES_PER_PANE - 1;
    return;
}

/* save_vga_state
 *  DESCRIPTION  : copies the contents of video memory to the address pointed to
 *                 by screen start field of vga struct
 *  INPUTS       : vga -- pointer to vga structure whose video memory to be saved
 *  OUTPUTS      : none
 *  RETURN VALUE : 0 on success, -1 on error
 *  SIDE EFFECTS : copies 4KB region from start of video memory to 
 *                 address pointed to by screen start field 
 */
int32_t save_vga_state(vga_t* vga)
{
    if (vga == NULL)
	     return -1;
    memcpy((uint8_t*)vga->screen_start, (uint8_t*)VIDEO_MEM_START, __4KB__);
    return 0;
}

/* restore_vga_state
 *  DESCRIPTION  : copies the contents of video memory starting at screen start
 *                 address in vga struct to the start of video memory.
 *  INPUTS       : vga -- pointer to vga structure whose video memory to restore
 *  OUTPUTS      : none
 *  RETURN VALUE : 0 on success, -1 on error
 *  SIDE EFFECTS : copies 4KB region from address pointed to by screen start field
 *                 to start of video memory. Also updates cursor positon on screen.
 */
int32_t restore_vga_state(vga_t* vga)
{
    if (vga == NULL)
	return -1;
    foreground = vga->fg;
    background = vga->bg;
    set_cursor_pos( vga->x_pos, vga->y_pos);
    memcpy((uint8_t*)VIDEO_MEM_START, (uint8_t*)vga->screen_start, __4KB__);
    bg_fg_reset();
    return 0;
}

/* restore_vga_state_NO_MEMORY
 *  DESCRIPTION : restores vga state without performing memory movement operations
 *  INPUTS      : vga -- pointer to vga context structure
 *  OUTPUTS     : none
 *  RETURN VALUE: -1 on error, 0 on success
 *  SIDE EFFECTS: modifies values of x_pos, y_pos, foreground, and background 
 *                variables. Does not explicitly change cursor position by
 *                modifying VGA cursor register
 */
int32_t restore_vga_state_NO_MEMORY(vga_t* vga)
{
    if (vga == NULL)
	   return -1;
    x_pos = vga->x_pos;
    y_pos = vga->y_pos;
    foreground = vga->fg;
    background = vga->bg;
    return 0;
}

/* save_vga_state_NO_MEMORY
 *  DESCRIPTION : saves vga state without performing memory movement operations
 *  INPUTS      : vga -- pointer to vga context structure
 *  OUTPUTS     : none
 *  RETURN VALUE: -1 on error, 0 on success
 *  SIDE EFFECTS: saves current (x,y) coordinats of cursor to x_pos, y_pos fields
 *                of pointer to vga structure passed as input
 */
int32_t save_vga_state_NO_MEMORY(vga_t* vga)
{
    if (vga == NULL)
	return -1;
    //uint16_t pos = get_cursor_pos();
    vga->x_pos        = x_pos; //pos % CHARS_PER_LINE;
    vga->y_pos        = y_pos; //pos / CHARS_PER_LINE;
    return 0;
}

/* init_term_vga
 *  DESCRIPTION: initializes vga structure corresponding to ID in vga table.
 *               This function is effectively NOT used.
 *  INPUTS:      id -- index of vga context in vga table
 *  OUTPUTS:     none
 *  RETURN VALUE : -1 on error, 0 on success
 *  SIDE EFFECTS : None
 */
int32_t init_term_vga(uint32_t id)
{
    if (CHECK_MSB(id))
	return -1;
    if (id > 8)
	return -1;
    return 0;
}

#endif
