#ifndef DRIVERS_H
#define DRIVERS_H

#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"

/* Ports that keyboard sist on*/
#define KEYBOARD_CMD_STAT_PORT         0x60
#define KEYBOARD_READ_WRITE_PORT       0x64
#define READ_KEYBOARD                  0x20
#define WRITE_KEYBOARD                 0x60
#define KEYBOARD_IRQ                   0x01
#define ENABLE_PORT                    0xAE
#define WRITE_CONFIG_BYTE              0x60
#define CONFIG_BYTE                    0x45
#define UNPRESSMASK                    0x7F // Use to analyze the keycode for an unpressed key
#define LEFTSHIFT                      0x2A // scan code of the the left shift
#define RIGHTSHIFT                     0x36 // scan code of the right shift
#define LEFTCTRL                       0x1D // scan code of the left control
#define RIGHTCTRL                      0xE0 // Not sure about this one scan code of the right control
#define CAPSLOCK                       0x3A // scan code of the caps lock
#define LKEY                           0x26 // scan code of the L key
#define F1KEY                          0x3B // scan code of the F1 key
#define F2KEY                          0x3C // scan code of the F2 key
#define F3KEY			                     0x3D // scan code of the F3 key
#define TABKEY                         0x0F // scan code of the TAB key
#define ENTERKEY                       0x1C // scan code of the enter key
#define BACKSPACEKEY                   0x0E // scan code of the backspace key
#define MAX_CHAR                         80 // Maximum number of characters per line
#define TAB_SIZE                          4 // Tab size
#define ALTKEY                         0x38
#define MAX_KEY_PRESS                    89

// structure used to easily access the upper, lower, caps, and shift caps characters
struct key {
  uint32_t shiftCapsChar  : 8;
  uint32_t capsChar       : 8;
  uint32_t upperChar      : 8;
  uint32_t lowerChar      : 8;
} __attribute__((packed));
typedef struct key key_t;

/*Assembly linkage for the keyboard handler*/
extern void keyboard_linkage();

/*Handler that reads the keyboard registers*/
extern void keyboard_handler();

/*Starts a keyboard session*/
extern void kb_session();

/*Initializes the keyboard*/
extern void keyboard_init();

/*Clears a terminal's buffer*/
extern void clear_buf();

/*Activates the RTC interrupt*/
extern uint32_t activate_inter_flag;

/*F2 key used to activate something*/
extern volatile uint32_t f2_key_flag;

/*F3 key used to print buffer*/
extern volatile uint32_t f3_key_flag;

extern uint32_t caps_flag;
extern uint32_t shift_flag;
extern uint32_t ctrl_flag;
extern uint32_t activate_inter_flag;
extern uint32_t terminal1Pos; // Position of the terminal 1 buffer
extern uint32_t keyboard_write;
#endif
