//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/17.
//
// Implement the Interrupt Description Table in c.

#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "Linkage.h"
#include "syscall.h"
#include "tasks.h"
#include "sb16.h"

// programming used. TODO: delete it when ready compile.
//extern idt;

void raise_except_info(int num);

// *****  Checkpoint 1 only ***** //
// Define Chars to Print out When receive IRQ.
char* except_table[20]={
        "0x00: Divide By Zero Error.",
        "0x01: Debug Error. ",
        "0x02: NMI Exception",
        "0x03: ",
        "0x04: ",
        "0x05: ",
        "0x06: ",
        "0x07: ",
        "0x08: ",
        "0x09: ",
        "0x0A: ",
        "0x0B: Segment not present Error. ",
        "0x0C: ",
        "0x0D: ",
        "0x0E: Page Fault",
        "0x0F: ",
        "0x10: ",
        "0x11: ",
        "0x12: ",
        "0x13: "
        // TODO: Fill the text when free.
};

// ***** Check point 1 only ***** //
/*
 *  syscall_handle
 *      DESCRIPTION: print out sth when receive system call.
 *      INPUT: None.
 *      OUTPUT: Printout on the terminal.
 *      RETURN: None.
 *      SIDE EFFECT: None.
 */
void syscall_handle(){
    printf("System call proceeded. \n ");
    return;
}

/*
 *  raise_except_info
 *      DESCRIPTION: The exception handler for CP1. Raise Exception info
 *                  using while loop to freeze the machine.
 *      INPUT:  except_num: the exception vector. up to 0xFF.
 *      OUTPUT: PRINT OUT the exception information.
 *      RETURN: None.
 *      SIDE EFFECT: Freeze the system using while loop.
 */
void raise_except_info (int except_num){
    cli();
    printf("\n");
    printf(" EXCEPTION: %s", except_table[except_num]);
    printf("\n");

    // restore from errors.
    halt(1);
    sti();
}

/*
 *  init_idt
 *      DESCRIPTION:    Initialize the IDT with self-defined interrupt.
 *      INPUT :         VOID
 i*      OUTPUT:         NONE
 *      RETURN:         NONE
 *      SIDE EFFECT:    Fill the IDT array.
 */
void init_idt(void){

    // local var definition.
    int i;

    /*
     *  8 Bytes in one IDT structure
     *
     *  [0] offset 15 : 0
     *  [2] Seg Selector
     *  [4] PAD one byte.
     *  [5] Attributes of Descriptor
     *      |7|6    5|4 |3     0|
     *      |P|DPL(2)|SG|Gate(4)|
     *
     *      DPL:  Privilege Level. 0x0 for kernel and 0x3 for user space.
     *      Gate: 0x5 Task Gate | 0xE Intr Gate for Interrupt | 0xF Trap Gate for Exception
     *
     *  [6] offset 31: 16
     */

    // SET the idt to empty.
    for (i=0; i<idt_size; i++){
        idt[i].seg_selector = KERNEL_CS;
        idt[i].size = 1;
        idt[i].present = 0;
        idt[i].dpl = 0;         // privilege will be updated later.
        idt[i].reserved4 = 0;   // When using interrupt gate, the SG will set to 0.
        idt[i].reserved3 = 1;   // Gate 0x1111 for TRAP gate.
        idt[i].reserved2 = 1;   // TODO: REMEMBER NOW all the gate now are intr gate.
        idt[i].reserved1 = 1;   // iialize all the gate to TRAP gate.
        idt[i].reserved0 = 0;
    }

    // Fill the interrupt into the IDT.
    SET_IDT_ENTRY(idt[0], idt_0);
    SET_IDT_ENTRY(idt[1], idt_1);
    SET_IDT_ENTRY(idt[2], idt_2);
    SET_IDT_ENTRY(idt[3], idt_3);
    SET_IDT_ENTRY(idt[4], idt_4);
    SET_IDT_ENTRY(idt[5], idt_5);
    SET_IDT_ENTRY(idt[6], idt_6);
    SET_IDT_ENTRY(idt[7], idt_7);
    SET_IDT_ENTRY(idt[8], idt_8);
    SET_IDT_ENTRY(idt[9], idt_9);
    SET_IDT_ENTRY(idt[10], idt_10);
    SET_IDT_ENTRY(idt[11], idt_11);
    SET_IDT_ENTRY(idt[12], idt_12);
    SET_IDT_ENTRY(idt[13], idt_13);
    SET_IDT_ENTRY(idt[14], idt_14);
    SET_IDT_ENTRY(idt[15], idt_15);
    SET_IDT_ENTRY(idt[16], idt_16);
    SET_IDT_ENTRY(idt[17], idt_17);
    SET_IDT_ENTRY(idt[18], idt_18);
    SET_IDT_ENTRY(idt[19], idt_19);

    // system call handle.
    SET_IDT_ENTRY(idt[System_Call_Vector], syscall_handler);
    idt[System_Call_Vector].reserved0 = 0;              // set the gate to interrupt gate.
    idt[System_Call_Vector].dpl = 3;                    // Allow user space to use sys call.

    // key board handle.
    SET_IDT_ENTRY(idt[KEYBOARD_VECTOR],key_intr_linkage);
    idt[KEYBOARD_VECTOR].reserved0 = 0;                 // Set the gate into Interrupt gate.

    // RTC handle
    SET_IDT_ENTRY(idt[RTC_VECTOR],rtc_intr_linkage);
    idt[RTC_VECTOR].reserved0 = 0;                      // SET the gate into inr gate.
    idt[RTC_VECTOR].dpl = 3;

    // PIT handle
    SET_IDT_ENTRY(idt[PIT_VECTOR],pit_intr_linkage);
    idt[PIT_VECTOR].reserved0 = 0;
    idt[PIT_VECTOR].dpl = 3;

    SET_IDT_ENTRY(idt[SB16_VECTOR],SB16_intr_linkage);
    idt[SB16_VECTOR].reserved0 = 0;
    idt[SB16_VECTOR].dpl = 3;

}


// Fill the exception handler with specific exception.
void divide_error()         { raise_except_info(0x00);}
void debug()                { raise_except_info( 0x01);}
void nmi()                  { raise_except_info(0x02);}
void breakpoint()           { raise_except_info(0x03);}
void overflow()             { raise_except_info(0x04);}
void bounds()               { raise_except_info( 0x05);}
void invalid_op()           { raise_except_info(0x06);}
void device_not_available() { raise_except_info(0x07);}
void doublefault_fn()       { raise_except_info(0x08);}
void coprocessor_segment_overrun() { raise_except_info(0x09);}
void invalid_TSS()          { raise_except_info(0x0A);}
void segment_not_present()  { raise_except_info(0x0B);}
void stack_segment()        { raise_except_info(0x0C);}
void general_protection()   { raise_except_info(0x0D);}
void page_fault()           { raise_except_info(0x0E);}
void intel_reserved()       { raise_except_info(0x0F);}
void coprocessor_error()    { raise_except_info(0x10);}
void alignment_check()      { raise_except_info(0x11);}
void machine_check()        { raise_except_info(0x12);}
void simd_coprocessor_error() { raise_except_info(0x13);}
