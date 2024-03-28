//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/18.
//

#include "RTC.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"

#define SUCCESS  0
#define FAILURE -1

volatile static uint32_t rtc_intr_flag;
int rtc_freq_check(int freq);
// RTC Writable Registers
/*
 *  RA
 *  |7  |6 4|3 0|
 *  |UIP|DVC|RSC|
 *
 *  UIP: Flag, 1 shows that update in progress.
 *  DVC: divider bit for RTC operate Freq. (010 to set 32.768KHZ in default.)
 *  RSC: Rate selection bits define RTC PI rate.
 *
 *  RB
 *  |7  |6  |5  |4  |3   |2 |1    |0  |
 *  |SET|PIE|AIE|UIE|SQWE|DM|24/12|DSE|
 *
 *  PIE: PI enable. We should set this.
 *  SQWE: Generate Square Wave. (Can we turn this off?)
 */

/*
 *  rtc_init
 *      DESCRIPTION: Initialize rtc to default rate and enable irq according to the wiki.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: O for success. other for fail.
 *      SIDE EFFECT: The RTC interrupt will enable, Modify the RA and RB register.
 */
int rtc_init(){
    char prev_B, prev_A;
    // select Register B to set PIE.
    outb(RB, RTC_COMMAND);                  // SELECT Register B to read old data.
    prev_B = inb(RTC_DATA);
    outb(RB, RTC_COMMAND);                  // SELECT Register B to write new data.
    outb((prev_B | 0x40), RTC_DATA);   // change PIE to enable Periodic Interrupt.

    // SELECT Register A to set Freq.
    outb(RA, RTC_COMMAND);                  // SELECT Register A.
    prev_A = inb(RTC_DATA);
    outb(RA, RTC_COMMAND);                  // Mask the UIP and DVC. We only need to change RSC.
    outb((prev_A & 0xF0)|DEFAULT_RATE, RTC_DATA);
    test = 0;
    // enable irq 8 on PICs.
    enable_irq(RTC_IRQ);
    rtc_intr_flag = 0;
    return 0;
}

/*
 *  rtc_handler
 *      DESCRIPTION: Handle RTC interrupt and send EOI to the PIC.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: Output RC. Change Interrupt status.
 */
void rtc_handler(){
    /* USED ONLY IN CP1. */
    // TODO: Add Virtualize to RTC Later.
    if (test){
        test_interrupts();
    }
    rtc_intr_flag += 1;
    // RESET RC and end interrupt.
    outb(RC, RTC_COMMAND);
    inb(RTC_DATA);
    send_eoi(RTC_IRQ);
}

/*
 *  rtc_open
 *      DESCRIPTION: set the frequency to default frequency when called.
 *      INPUT/OUTPUT: None.
 *      RETURN: 0 for SUCCESS. No fail conditions.
 *      SIDE EFFECT: rtc freq change.
 */
int rtc_open(const uint8_t* filename){
    // set frequency for rtc -1024 hz.
    char prev_A;
    // SELECT Register A to set Freq.
    outb(RA, RTC_COMMAND);                  // SELECT Register A.
    prev_A = inb(RTC_DATA);
    outb(RA, RTC_COMMAND);                  // Mask the UIP and DVC. We only need to change RSC.
    outb((prev_A & 0xF0)|OPEN_RATE, RTC_DATA);
    printf("test rct open file");
    return SUCCESS;
}

/*
 *  rtc_close
 *      DESCRIPTION: close virtual rtc driver. Do nothing when no activated virtual RTC.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: virtual rtc closed.
 */
int rtc_close(int32_t fd){

    // restore the rate to 1024HZ.
    char prev_A;
    // SELECT Register A to set Freq.
    outb(RA, RTC_COMMAND);                  // SELECT Register A.
    prev_A = inb(RTC_DATA);
    outb(RA, RTC_COMMAND);                  // Mask the UIP and DVC. We only need to change RSC.
    outb((prev_A & 0xF0)|DEFAULT_RATE, RTC_DATA);
    return SUCCESS;
}

/*
 *  rtc_read
 *      DESCRIPTION: return success when interrupt happened.
 *      INPUT: None. (not used. )
 *      OUTPUT: None.
 *      RETURN: always success.
 *      SIDE EFFECT: none,
 */
int rtc_read(int32_t fd, void* buf, int32_t nbytes){

    rtc_intr_flag = 0;                      // clear the RTC flag.
    while(!rtc_intr_flag);                  // wait for interrupt to set the flag.
    return SUCCESS;
}

/*
 *  rtc_write
 *      DESCRIPTION: read frequency in HZ that written as uint32_t from the buffer. change the
 *                   rtc frequency according to the input.
 *      INPUT:  fd: file directory that unused.
 *              buf: buffer pointer contains the freq.
 *              nbytes: the length of the buffer. should be 4.
 *      OUTPUT: NONE.
 *      RETURN : SUCCESS for success. FAILURE for failure.
 *      SIDE EFFECT: None.
 */
int rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    int freq,rate;
    char prev_A;
    // check for valid input.
    if (buf == NULL || nbytes != 4){
        return FAILURE;
    }

    freq = *(int*)buf;                  // copy the value in the buffer.
    if (freq > 1024){
        return FAILURE;
    }
    rate = rtc_freq_check(freq);        // convert the freq into rate.
    if (rate == FAILURE){
        return FAILURE;
    }
    // SELECT Register A to set Freq.
    outb(RA, RTC_COMMAND);                  // SELECT Register A.
    prev_A = inb(RTC_DATA);
    outb(RA, RTC_COMMAND);                  // Mask the UIP and DVC. We only need to change RSC.
    outb((prev_A & 0xF0)|rate, RTC_DATA);
    return SUCCESS;
}


/*
 *  rtc_freq_check
 *      DESCRIPTION: CHECK if the given freq in HZ is valid. then convert it to
 *                   corresponding rate for RTC.
 *      INPUT:  Rates in HZ,
 *      OUTPUT: NOne.
 *      RETURN: calculated corresponding rate.
 *              FAILURE for fail convection.
 *      SIDE EFFECT: None.
 */
int rtc_freq_check(int freq){
    // directly return freq... since there are not too much.

    // one line: (freq-1)&freq == 0.
    // use shift to log 2.
    // but i think this can be faster.
    switch (freq) {
        case 2:
            return 0x0F;
        case 4:
            return 0x0E;
        case 8:
            return 0x0D;
        case 16:
            return 0x0C;
        case 32:
            return 0x0B;
        case 64:
            return 0x0A;
        case 128:
            return 0x09;
        case 256:
            return 0x08;
        case 512:
            return 0x07;
        case 1024:
            return 0x06;
        default:
            return FAILURE;
    }
}