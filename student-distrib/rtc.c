#ifndef RTC_C
#define RTC_C
#include "include/rtc.h"
#include "include/keyboard.h"
#include "include/sched.h"


// NOTE: default frequency is 1024 Hz used to keep proper time
// 1024/0x400 interrupts == 1 sec

uint32_t ticks;
uint32_t freq;
int interrupt_;
int intr_count = 0;
rtc_t rtc;
/*
 * rtc_init
 *   DESCRIPTION: initializes the RTC and fills the IDT table for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes the RTC
 */
void rtc_init()
{
  unsigned long cliflags;
  uint8_t regB;
  cli_and_save(cliflags);
  unsigned short segSize = KERNEL_CS; // The segment is KERNEL_CS bc this stuff is in kernel
  unsigned short flags = 0x8E00; // This flag corresponds to a Interrupt gate with 0 DPL,
                                 // present flag to 1, and gate size 1 (32-bits)

  fill_interrupt(RTC_INTERRUPT_VEC,(uint32_t*)rtc_linkage,segSize,flags);

  // Now to initialize the RTC This outb stuff is probably not needed
  outb(RTC_REG_B,RTC_INDEX_REG); // Select register B and diable NMI(non-maskable interupts)
  regB = inb(RTC_RW_REG);   // Read register B
  outb(RTC_REG_B,RTC_INDEX_REG);
  outb(regB|0x40,RTC_RW_REG);//0x40 is the freq for 2Hz

  ticks = 0;                // Initialize the number of RTC ticks
  freq = BASE_FREQ;             // Base frequecnt is 1024 HZ
  rtc.id = 0;
  rtc.frequency = freq;

 set_rtc_freq(DEF_FREQ);
  // Unmask the RTC_IRQ
  enable_irq(RTC_IRQ);
  restore_flags(cliflags);
  return;
}


/*
 * rtc_handler
 *   DESCRIPTION: handles an RTC interrupt by incrementing the ticks
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handles an RTC interrupt
 */
void rtc_handler()
{
  // Read register C for what interrupt happened
  cli();
    outb(RTC_REG_C,RTC_INDEX_REG);
  inb(RTC_RW_REG);      //Need to read register C for next interrupt to happen
  intr_count++;
  interrupt_ = 1;
  ticks++;              // Increment the number of tick
  // if(activate_inter_flag == 1) screen will flash if f1 is pressed
  //   test_interrupts();
  // // if(ticks%freq == 0)
  // //   printf("1Sec\n");
  send_eoi(RTC_IRQ);   // send eoi after servicing
  sti();
  return;
}
/*
 * get_rtc_intr
 *   DESCRIPTION: helper function to retrieve value
 *   INPUTS: none
 *   OUTPUTS: intr_count - the number of rtc interrupts
 *   RETURN VALUE: intr_count - the number of rtc interrupts
 *   SIDE EFFECTS: none
 */
int get_rtc_intr() {
    //printf("%d ", intr_count);
    return intr_count;
}

/*
 * set_rtc_freq_default
 *   DESCRIPTION: sets the frequency of the rtc
 *   INPUTS: rate - the rate of the RTC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets the frequency of the rtc
 */

void set_rtc_freq_default(uint32_t rate)
{
    // The rate value has to be between 1 and 15 but not 1 or 2
    if(rate > UPPER_RATE || rate < DEF_FREQ)
	return;
    // Calculation of frequency
    freq = MAX_FREQ >> (rate-1);
    rtc.frequency = freq;
    rate &= 0x0F;

    cli();
    outb(RTC_REG_A, RTC_INDEX_REG);
    char prev = inb(RTC_RW_REG);
    outb(RTC_REG_A, RTC_INDEX_REG);
    outb((prev & 0xF0)|DEF_FREQ, RTC_RW_REG); // 0xF0 preserve the top 4 bits
    sti();

    return;
}
/*
 * set_rtc_freq
 *   DESCRIPTION: sets the frequency of the rtc
 *   INPUTS: rate - the rate of the RTC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets the frequency of the rtc
 */

void set_rtc_freq(uint32_t rate)
{
    // The rate value has to be between 1 and 15 but not 1 or 2
    if(rate > UPPER_RATE || rate < DEF_FREQ)
	return;
    // Calculation of frequency
    freq = MAX_FREQ >> (rate-1);
    //rtc.frequency = freq;
    rate &= 0x0F;

    cli();
    outb(RTC_REG_A, RTC_INDEX_REG);
    char prev = inb(RTC_RW_REG);
    outb(RTC_REG_A, RTC_INDEX_REG);
    outb((prev & 0xF0)|rate, RTC_RW_REG); // 0xF0 preserve the top 4 bits
    sti();

    return;
}
/*
 * rtc_open
 *   DESCRIPTION: opens the rtc file and sets
 *   frequency to 2 Hz
 *   INPUTS: filename - the rtc file
 *   OUTPUTS: int
 *   RETURN VALUE: '0' on success
 *   SIDE EFFECTS: sets the frequency of the rtc
 */

int rtc_open(const uint8_t* filename) {
    cli();
    pid_t* hpid = get_current_htable_entry();
    proc_t* current_pcb = &hpid->pcb;
    current_pcb->rtc_freq = STARTING_FREQ; // Base freq is 2 htz
    sti();
    return 0;
}

/*
 * rtc_close
 *   DESCRIPTION: closes the rtc file
 *   INPUTS: fd - the file descriptor for the rtc file
 *   OUTPUTS: int
 *   RETURN VALUE: '0' on success
 *   SIDE EFFECTS: none (for now)
 */
int rtc_close(int32_t fd) {
    cli();

    pid_t* hpid = get_current_htable_entry();
    proc_t* current_pcb = &hpid->pcb;
    current_pcb->rtc_freq = 0;
    sti();
    return 0;
}

/*
 * rtc_write
 *   DESCRIPTION: changes the frequency of the rtc
 *   INPUTS: fd - the file descriptor for the rtc file (for now not used)
 *   buf - the frequency value is passed in as a pointer
 *   nbytes - the number of bytes to write (for now not used)
 *   OUTPUTS: int
 *   RETURN VALUE: '0' on success, '-1' on invalid frequency
 *   SIDE EFFECTS: sets rtc frequency to any power of 2 within a range
 */
int rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    cli();
    if (CHECK_MSB(nbytes))
    {
       sti();
	     return -1;
    }
    int freq_ = *(int*)buf;
    uint32_t freq_t = freq_ - 1; /* check and make sure the */
    if((freq_ & freq_t) != 0) {  /* freq passed in is a valid power of 2 */
       sti();
	     return -1;
    }
    rtc.frequency = freq_;
    pid_t* hpid = get_current_htable_entry();
    proc_t* current_pcb = &hpid->pcb;
    current_pcb->rtc_freq = freq_;
    //set_rtc_freq(DEF_FREQ); // Make sure that the freq is set to max
    sti();
    return 0;
}

/*
 * rtc_read
 *   DESCRIPTION: blocks until the user freq has received enough interrupts for the default frequency rating
 *   INPUTS: fd - the file descriptor for the rtc file (for now not used)
 *   buf - the frequency value is passed in as a pointer
 *   nbytes - the number of bytes to read (for now not used)
 *   OUTPUTS: int
 *   RETURN VALUE: '0' on success
 *   SIDE EFFECTS: only returns once enough interrupts have been received to give the illusion
 *   the rtc is operationg at the user frequency setting
 */
int rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    //set_rtc_freq(6); /* set max frequency, 1024 Hz */
    if (CHECK_MSB(nbytes) || current_proc->rtc_freq <= 0)
	   return -1;
    int freq_ = current_proc->rtc_freq;
    int count = BASE_FREQ / freq_; /* interrupts needed for user frequency*/
    for(; count > 0; count--) { /* loop until enough interrupts have been received */
    	while(interrupt_ == 0) {
    	    continue;
    	}
    	interrupt_ = 0;
    }
    return 0;
}

int32_t rtc_open_wrapper(int32_t fd, uint8_t* filename, uint32_t nbytes)
{
    return (int32_t)rtc_open(filename);
}
int32_t rtc_close_wrapper(int32_t fd, uint8_t* filename, uint32_t nbytes)
{
    return (int32_t)rtc_close(fd);
}
int32_t rtc_read_wrapper(int32_t fd, uint8_t* buffer, uint32_t nbytes)
{
    return (int32_t)rtc_read(fd, (void*)buffer, (int32_t)nbytes);
}
int32_t rtc_write_wrapper(int32_t fd, uint8_t* buffer, uint32_t nbytes)
{
    return (int32_t)rtc_write(fd, (void*)buffer, (int32_t)nbytes);
}
io_table_t rtc_ops_table = { &rtc_open_wrapper, &rtc_close_wrapper, &rtc_read_wrapper, &rtc_write_wrapper };
#endif
