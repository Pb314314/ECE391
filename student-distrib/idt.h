//
// Created by Jordan (qishenz2@illinois.edu) on 2022/3/17.
//
//  Implement the Interrupt Description Table in c.
//  Header file.

#ifndef MP3_IDT_H
#define MP3_IDT_H

// Define some Constant.
#define idt_size 255       // 0x00 to 0xFF
#define Exception_range 20  // 0x14

// Define common vector.
#define System_Call_Vector  0x80
#define PIT_VECTOR          0x20
#define KEYBOARD_VECTOR     0x21
#define SB16_VECTOR         0x25
#define RTC_VECTOR          0x28
#define MOUSE_VECTOR        0x2C

// Define IDT function.
void init_idt(void);

// handler extern
void divide_error();
void debug();
void nmi();
void breakpoint();
void overflow();
void bounds();
void invalid_op();
void device_not_available();
void doublefault_fn();
void coprocessor_segment_overrun();
void invalid_TSS();
void segment_not_present();
void stack_segment();
void general_protection();
void page_fault();
void intel_reserved();
void coprocessor_error();
void alignment_check();
void machine_check();
void simd_coprocessor_error();

#endif //MP3_IDT_H
