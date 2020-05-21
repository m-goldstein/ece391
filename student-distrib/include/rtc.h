// RTC.h - holds information needed for rtc.h
#ifndef RTC_H
#define RTC_H

#include "sys_call.h"
#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"

#define RTC_INTERRUPT_VEC 0x28
#define RTC_INDEX_REG     0x70
#define RTC_RW_REG        0x71
#define RTC_IRQ           0x08
#define RTC_REG_C         0x0C
#define DEFAULT_RATE      0x06
#define MAX_FREQ          32768
#define BASE_FREQ         512
#define STARTING_FREQ     2
#define RTC_REG_A         0x8A // Also disables NIM
#define RTC_REG_B         0x8B
#define DEF_FREQ          7
#define UPPER_RATE        15
#define NUM_RTC_OPS	      4
struct rtc {
    uint32_t id;
    uint32_t frequency;
};
typedef struct rtc rtc_t;
/*Initializes the RTC*/
extern void rtc_init();

/*Handles and rtc interrupt*/
extern void rtc_handler();

/*Assemby linkage for the RTC interrupt*/
extern void rtc_linkage();

extern int get_rtc_intr();

/*Sets the frequency of the RTC*/
extern void set_rtc_freq_default(uint32_t rate);

/*Sets the frequency of the RTC*/
extern void set_rtc_freq(uint32_t rate);

/*opens the RTC file*/
extern int rtc_open(const uint8_t* filename);
extern int32_t rtc_open_wrapper(int32_t fd, uint8_t* filename, uint32_t nbytes);
extern int32_t rtc_close_wrapper(int32_t fd, uint8_t* filename, uint32_t nbytes);
extern int32_t rtc_read_wrapper(int32_t fd, uint8_t* buffer, uint32_t nbytes);
extern int32_t rtc_write_wrapper(int32_t fd, uint8_t* buffer, uint32_t nbytes);
/*closes the RTC file*/
extern int rtc_close(int32_t fd);

/*writes to the RTC and changes the frequecy*/
extern int rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/*blocks until enough user interrupts have been received*/
extern int rtc_read(int32_t fd, void* buf, int32_t nbytes);
#endif
