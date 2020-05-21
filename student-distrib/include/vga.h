#ifndef VGA_H
#define VGA_H
#include "qemu_vga.h"
/* get_cursor_pos
 *  DESCRIPTION : returns linear offset of cursor positon on screen 
 *  INPUT       : none
 *  OUTPUT      : none
 *  RETURN VALUE: linear offset of cursor on display
 *  SIDE EFFECTS: none 
 */
static inline uint16_t get_cursor_pos()
{
    uint16_t pos = 0;
    OUTB(VGA_IO_ADDR, VGA_CURSOR_LOC_LOW);
    pos |= inb(VGA_IO_DATA);
    OUTB(VGA_IO_ADDR, VGA_CURSOR_LOC_HIGH);
    pos |= ((uint16_t)inb(VGA_IO_DATA)) << 8; // Bit shift 8 to set correctly the attrib
    return pos;
}
/* set_cursor_pos
 *  DESCRIPTION : sets linear offset of cursor positon on screen from (x,y) coords
 *  INPUT       : x -- x coordinate of cursor 
 *                y -- y coordinate of cursor
 *  OUTPUT      : none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: changes cursor position to (x,y) specified as inputs 
 */
static inline void set_cursor_pos(uint16_t x, uint16_t y)
{
    uint16_t pos = VGA_OFFSET(x,y);
    OUTB(VGA_IO_ADDR, VGA_CURSOR_LOC_LOW);
    OUTB(VGA_IO_DATA, (uint8_t)(pos & 0xFF));
    OUTB(VGA_IO_ADDR, VGA_CURSOR_LOC_HIGH);
    OUTB(VGA_IO_DATA, (uint8_t)((pos >> 8) & 0xFF));
    return;
}
extern uint32_t vga_mem_base; /* pointer to base of video memory */
extern void vga_backspace();  /* backspace implementation */
extern void vga_update_cursor(); /* update cursor implementation */
extern void vga_clear(void);     /* clear screen implementation  */
extern void vga_ctrl_L();        /* ctrl-L implementation        */
extern void bg_fg_reset();       /* reset VGA color attribute values */
extern int32_t vga_printf(int8_t *format, ...); /* printf implementation */
extern void vga_scroll_screen();       /* scroll screen implementation */
extern int32_t restore_vga_state_NO_MEMORY(vga_t* vga); /* restore vga state without adjusting vga_mem_base pointer */
extern int32_t save_vga_state_NO_MEMORY(vga_t* vga); /* save vga state without adjusting vga mem base pointer */
#endif
