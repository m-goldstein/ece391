/* References
 * https://www.wiki.osdev.org/Text_UI/
 * https://www.wiki.osdev.org/Text_Mode_Cursor/
 * https://www.wiki.osdev.org/VGA_Hardware/
 */
#ifndef QEMU_VGA_H
#define QEMU_VGA_H
#include "types.h"
#include "lib.h"
#define MAX_PANES	            0x00000008
#define VIDEO_MEM_START_FENCE 0x000B7000
#define VIDEO_MEM_START	      0x000B8000
#define VIDEO_MEM_END	        0x000C0000
#define VIDEO_MEM_END_FENCE   0x000C1000
#define CHARS_PER_LINE		            80
#define LINES_PER_PANE	              25
#define VGA_FRAME_SIZE	      (CHARS_PER_LINE)*(LINES_PER_PANE)*2
#define OUTB(port, val)			 \
    do {				 \
	asm volatile("outb %b1, (%w0)"	 \
		:			 \
		:"d"((port)), "a"((val)) \
		:"memory", "cc"          \
		);			 \
    } while (0)

#define VGA_OFFSET(x,y)		              \
    ((y)*(CHARS_PER_LINE) + (x))
/* assign colors to macros */
enum QEMU_VGA_TEXT_COLOR_PALETTE {
	BLACK	      = 0x0,
	BLUE	      = 0x1,
	GREEN	      = 0x2,
	CYAN	      = 0x3,
	RED	      = 0x4,
	MAGENTA	      = 0x5,
	BROWN	      = 0x6,
	LIGHT_GREY    = 0x7,
	DARK_GREY     = 0x8,
	LIGHT_BLUE    = 0x9,
	LIGHT_GREEN   = 0xA,
	LIGHT_CYAN    = 0xB,
	LIGHT_RED     = 0xC,
	LIGHT_MAGENTA = 0xD,
	YELLOW        = 0xE,
	WHITE	      = 0xF
};
/* assign register values to macros for convenience */
enum QEMU_VGA_REGISTERS {
     VGA_IO_ADDR          = 0x03D4, // address
     VGA_IO_DATA          = 0x03D5, // data
     PROTECTION_BIT_MASK  = 0x0011, // to turn off protection bit
     VGA_MISC_OUTPUT_RD   = 0x03CC,
     VGA_MISC_OUTPUT_WR   = 0x03C2,
     VGA_IOAS_MASK        = 0x0001,
     VGA_RAM_EN_MASK      = 0x0002,
     VGA_CURSOR_START_POS = 0x000A,
     VGA_CURSOR_END_POS   = 0x000B,
     VGA_CURSOR_LOC_HIGH  = 0x000E,
     VGA_CURSOR_LOC_LOW   = 0x000F,
     VGA_READ_REQ         = 0x0000,
     VGA_WRITE_REQ        = 0x0001
};
/* structure to store vga context for switching between tty sessions */
struct vga_context {
    uint8_t  fg;    /* foreground color */
    uint8_t  bg;    /* background color */
    int16_t  x_pos; /* cursor x pos     */
    int16_t  y_pos; /* cursor y pos     */
    uint8_t* screen_start; /* offset in video memory */
};
typedef struct vga_context vga_t;
struct vga_context_table {
    vga_t panes[MAX_PANES];
    uint8_t bitmap;
};
typedef struct vga_context_table vga_table_t;
extern vga_table_t vga_table;
extern int32_t init_vga(); /* initialize vga support */
extern int32_t vga_putc_color(uint8_t c, uint8_t fg, uint8_t bg, int32_t x, int32_t y); /* parameterized putc */
extern int32_t vga_tuple_putc(uint8_t c, int32_t x, int32_t y); /* parameterized putc */
extern int32_t vga_putc(uint8_t c); /* regular putc */
extern int32_t vga_puts(int8_t* s); /* regular puts */
extern int32_t vga_printf(int8_t* format, ...); /* printf */
extern int32_t save_vga_state(vga_t* vga); /* save state */
extern int32_t restore_vga_state(vga_t* vga); /* restore state */
extern int32_t init_term_vga(uint32_t id); /* initialize a vga instance */
extern uint8_t foreground;                 /* foreground color attribute */
extern uint8_t background;                 /* background color attribute */
extern int16_t y_pos;                      /* cursor y position          */
extern int16_t x_pos;                      /* cursor x position          */
#endif
