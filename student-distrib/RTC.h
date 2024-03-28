//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/18.
//

#include "types.h"

#ifndef MP3_RTC_H
#define MP3_RTC_H
#define TEST 1                 // 1 for rtc interrupt test USED IN CP1.

#define RTC     0x70
#define CMOS    0x71

#define RTC_COMMAND RTC
#define RTC_DATA    CMOS

#define RA      0x8A
#define RB      0x8B
#define RC      0x8C

#define RTC_IRQ 8
#define BASE_RATE 15
#define DEFAULT_RATE 0x06
#define OPEN_RATE    0x0F

extern int rtc_init();
extern void rtc_handler();

extern int rtc_open(const uint8_t* filename);
extern int rtc_close(int32_t fd);
extern int rtc_write(int32_t fd, const void* buf, int32_t nbytes);
extern int rtc_read(int32_t fd, void* buf, int32_t nbytes);

#endif //MP3_RTC_H
