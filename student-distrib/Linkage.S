#define ASM 1
#include "Linkage.h"

# General linkage.

# RTC linkage

.global rtc_intr_linkage
rtc_intr_linkage:
    pushal
    pushfl
    call    rtc_handler
    popfl
    popal
    iret

# Key board Linkage.
.global key_intr_linkage
key_intr_linkage:
    pushal
    pushfl
    call    key_board_handler
    popfl
    popal
    iret

# pit interrupt Linkage
.global pit_intr_linkage
pit_intr_linkage:
    pushal
    pushfl
    call    pit_handler
    popfl
    popal
    iret

/* SB16 interrupt linkage code */
.global SB16_intr_linkage();
SB16_intr_linkage:
    pushal
    pushfl
    call    sb16_handler
    popfl
    popal
    iret


# exception handler linkage.
.global idt_0, idt_1, idt_2, idt_3, idt_4, idt_5
.global idt_6, idt_7, idt_8, idt_9, idt_10, idt_11
.global idt_12, idt_13, idt_14, idt_15, idt_16, idt_17
.global idt_18, idt_19

idt_0:
    pushal
    call     divide_error
    popal
    iret

idt_1:
    pushal
    call     debug
    popal
    iret

idt_2:
    pushal
    call    nmi
    popal
    iret

idt_3:
    pushal
    call    breakpoint
    popal
    iret

idt_4:
    pushal
    call    overflow
    popal
    iret

idt_5:
    pushal
    call    bounds
    popal
    iret

idt_6:
    pushal
    call    invalid_op
    popal
    iret

idt_7:
    pushal
    call    device_not_available
    popal
    iret

idt_8:
    pushal
    call    doublefault_fn
    popal
    iret

idt_9:
    pushal
    call    coprocessor_segment_overrun
    popal
    iret

idt_10:
    pushal
    call    invalid_TSS
    popal
    iret

idt_11:
    pushal
    call    segment_not_present
    popal
    iret

idt_12:
    pushal
    call    stack_segment
    popal
    iret

idt_13:
    pushal
    call    general_protection
    popal
    iret

idt_14:
    pushal
    call    page_fault
    popal
    iret

idt_15:
    pushal
    call    intel_reserved
    popal
    iret

idt_16:
    pushal
    call    coprocessor_error
    popal
    iret

idt_17:
    pushal
    call    alignment_check
    popal
    iret

idt_18:
    pushal
    call    machine_check
    popal
    iret

idt_19:
    pushal
    call    simd_coprocessor_error
    popal
    iret







