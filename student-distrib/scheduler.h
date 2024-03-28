//
// Created by Jordan(qishenz2@illinois.edu) on 2022/4/23.
//

#ifndef MP3_SCHEDULER_H
#define MP3_SCHEDULER_H

#endif //MP3_SCHEDULER_H

#define PIT_COMMAND     0x40
#define PIT_DATA        0x43

#define PIT_MODE_SELECT 0x37        // USE mode 3, Channel 0.

#define PIT_FREQ        100
#define PIT_MAX_FREQ    1193180     // all in HZ.

#define PIT_IRQ         0

void PIT_init();

extern void pit_handler();
