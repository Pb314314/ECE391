/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
/*
 * pic_init
 *      DESCRIPTION: initiate the intel 8259 PIC according to
 *                   the instruction.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: initiate the pics, use ports to transfer bits.
 */
void i8259_init(void) {
    master_mask = 0xFF;
    slave_mask = 0xFF;
    outb(master_mask,PIC1_DATA);            // Mask all of Master PIC.
    outb(slave_mask,PIC2_DATA);             // Mask all of Slave PIC.

    // init Master PIC.
    outb(ICW1,PIC1_COMMAND);                // ICW1: Sending ICW1 to Master PIC.
    outb(ICW2_MASTER, PIC1_DATA);           // ICW2: Mapping IRQ0-7 on port 0x20 to 0x27.
    outb(ICW3_MASTER, PIC1_DATA);           // ICW3: Set secondary PIC on IRQ2 of Master.
    outb(ICW4, PIC1_DATA);                  // ICW4: Not Auto EOI mode. ICW4 is 0x01.

    // init Slave PIC.
    outb(ICW1, PIC2_COMMAND);                // ICW1: initiate Slave PIC.
    outb(ICW2_SLAVE, PIC2_DATA);            // ICW2: Mapping IRQ0-7 to port 0x28-0x2F.
    outb(ICW3_SLAVE, PIC2_DATA);              // ICW3: Tell Slave PIC it is on IRQ2 of Master.
    outb(ICW4, PIC2_DATA);              // ICW4: Slave PIC will not affected by Auto EOI.

    // Load Cache value.
    outb(master_mask, PIC1_DATA);           // mask the Master PIC again and wait for device install.
    outb(slave_mask, PIC2_DATA);            // mask the Slave PIC again and wait for device install
    enable_irq(2);
}

/* Enable (unmask) the specified IRQ */
/*
 *  enable_irq
 *      DESCRIPTIONS: enable the given specific irq when device are loaded to the system
 *      INPUT:  irq: the interrupt num from 0 to 15.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: Enable one specific interrupt port on PIC.
 */
void enable_irq(uint32_t irq_num) {
    unsigned int port,value;
    // check if the number is valid.
    if (irq_num > Max_Device || irq_num< 0){
        return;
    }

    // Check for Master and Slave port.
    if (irq_num  < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }

    value = inb(port) & ~(1 << irq_num);
    outb(value, port);
    return;
}


/* Disable (mask) the specified IRQ */
/*
 *  disable_irq
 *      DESCRIPTION: disable the given specific irq by masking it.
 *      INPUT: urq: the given irq port.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: Disable specific port n PIC.
 */
void disable_irq(uint32_t irq_num) {

    unsigned int port, value;
    // check if the irq value is valid.
    if (irq_num > Max_Device || irq_num<0){
        return;
    }

    // check for master and slave port.
    if (irq_num < 8){
        port = PIC1_DATA;
    } else{
        port = PIC2_DATA;
        irq_num -= 8;
    }

    value = inb(port) | (1 << irq_num);
    outb(value, port);
    return;
}

/* Send end-of-interrupt signal for the specified IRQ */

/*
 *  send_eoi
 *      DESCRIPTION: send EOI signals to Port to end and interruption.
 *      INPUT:  irq: the given irq port.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: Send EOI signal to the bus, the interrupt will end.
 */
void send_eoi(uint32_t irq_num) {
    // check if the irq value is valid.
    if (irq_num > Max_Device || irq_num<0){
        return;
    }

    if (irq_num < 8){
        // send EOI to master PIC only.
        outb(EOI|irq_num, PIC1_COMMAND);
    } else{
        irq_num -= 8;
        // send EOI to Slave PIC, and the IQR2 of the Master PIC.
        outb(EOI|irq_num, PIC2_COMMAND);
        outb(EOI|0x02, PIC1_COMMAND);
    }
}
