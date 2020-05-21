/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "include/i8259.h"
#include "include/lib.h"
#define MASTER_LOW  0x0
#define MASTER_HIGH 0x7
#define SLAVE_LOW	0x8
#define SLAVE_HIGH	0xF
/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xff; /* IRQs 0-7  */
uint8_t slave_mask = 0xff;  /* IRQs 8-15 */

/* i8259_init
 *  DESCRIPTION: initializes the master/slave pics in the IDT table and enables IRQ for both.
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: initializes the pic
 */
void i8259_init(void)
{
	unsigned long flags;
	cli_and_save(flags);

	/* Initialize master PIC */
	outb(ICW1, MASTER_8259_CMD);
	outb(ICW2_MASTER, MASTER_8259_DATA);
	outb(ICW3_MASTER, MASTER_8259_DATA);
	outb(ICW4, MASTER_8259_DATA);
	/* End master PIC init */

	/* Initialize slave PIC */
	outb(ICW1, SLAVE_8259_CMD);
	outb(ICW2_SLAVE, SLAVE_8259_DATA);
	outb(ICW3_SLAVE,SLAVE_8259_DATA);
	outb(ICW4, SLAVE_8259_DATA);
	/* End slave PIC init */

	// Restore flag
	outb(master_mask, MASTER_8259_DATA);
	outb(slave_mask, SLAVE_8259_DATA);
	enable_irq(SLAVE_IRQ); // Enable the slave pic

	restore_flags(flags);
}

/* enable_irq
 *  DESCRIPTION: Enable (unmask) the specified IRQ
 *  INPUTS: irq_num to be enabled
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
 */
void enable_irq(uint32_t irq_num)
{
	unsigned long flags;
	cli_and_save(flags);
  //Master
  if(irq_num < 8){ // Max of 8 IRQ lines per PIC
    master_mask = master_mask & ~(0x01 << irq_num);
    outb(master_mask, MASTER_8259_DATA);
  }
  //Slave
  else{
    slave_mask = slave_mask & ~(0x01 << (irq_num-MASTER_PORT_COUNT));
    outb(slave_mask, SLAVE_8259_DATA);
  }
	restore_flags(flags);
}

/* disable_irq
 *  DESCRIPTION: Disable (mask) the specified IRQ
 *  INPUTS: irq_num to be disabled
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
 */
void disable_irq(uint32_t irq_num)
{
	unsigned long flags;
	cli_and_save(flags);
  //Master
  if(irq_num < 8){ // Max of 8 IRQ lines per PIC
    master_mask = master_mask | (0x01 << irq_num);
    outb(master_mask, MASTER_8259_DATA);
  }
  //Slave
  else{
    slave_mask = slave_mask | (0x01 << (irq_num-MASTER_PORT_COUNT));
    outb(slave_mask, SLAVE_8259_DATA);
  }
	restore_flags(flags);
}

/* send_eoi
 *  DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *  INPUTS: irq_num to recieve eoi signal
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
 */
void send_eoi(uint32_t irq_num)
{
  if(irq_num > 7) // Max of 8 IRQ lines per PIC (base 0)
	{
    outb(EOI | (irq_num-MASTER_PORT_COUNT),SLAVE_8259_CMD);
		send_eoi(SLAVE_IRQ);
	}
	else
	{
  	outb((EOI | irq_num),MASTER_8259_CMD);
	}
}
