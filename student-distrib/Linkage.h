//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/19.
//

#ifndef MP3_LINKAGE_H
#define MP3_LINKAGE_H
#ifndef ASM

// Linkage function extern.
void rtc_intr_linkage();
void key_intr_linkage();
void pit_intr_linkage();
void SB16_intr_linkage();

// Exception function extern.
void idt_0();
void idt_1();
void idt_2();
void idt_3();
void idt_4();
void idt_5();
void idt_6();
void idt_7();
void idt_8();
void idt_9();
void idt_10();
void idt_11();
void idt_12();
void idt_13();
void idt_14();
void idt_15();
void idt_16();
void idt_17();
void idt_18();
void idt_19();

#endif
#endif //MP3_LINKAGE_H
