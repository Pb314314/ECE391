//
// Created by Jordan(qishenz2@illinois.edu) on 2022/4/23.
//

#include "scheduler.h"
#include "tasks.h"
#include "Terminal.h"
#include "lib.h"
#include "i8259.h"

/*
 *  init_pit
 *      DESCRIPTION: initiate the PIT, enable the irq and set freq to set value.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: the PIT irq enabled.
 */
void PIT_init(){

    uint16_t freq_division = (PIT_MAX_FREQ/2)/PIT_FREQ;
    // select mode 3, channel 0;
    outb(PIT_MODE_SELECT, PIT_COMMAND);
    // load the freq into the PIT.
    outb((uint8_t)freq_division && 0xFF, PIT_DATA);
    outb((uint8_t)freq_division >> 8,PIT_DATA);

    // set the i8259.
    enable_irq(PIT_IRQ);
    return;
}

/*
 *  pit_handler
 *      DESCRIPTION: handle the PIT interrupt and switch the process. working as scheduler.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: switch the process.
 */
void pit_handler(){
    terminal_t* next_term;
    // ending interrupt.
    send_eoi(PIT_IRQ);
    // remap the video memory if any process on the terminal.
    if (handle_term->cur_pid != -1){
        next_term = get_terminal(handle_term->next_tid);

        if (next_term->terminalID == cur_terminal_id){//if the next terminal to execute == current displaying terminal id
            //example: handle term2(already executed), next is terminal 0. current show terminal0. then should map the user video memory to the video address.
            vid_remap((uint8_t*)VIDEO_ADDR);//map the program paging to the physical VM
        } else{
            // map to the buffers for each terminal.
            vid_remap((uint8_t*)next_term->video_ptr);//map the program paging to the next terminal buffer
        }
        tlb_flush();
    }
    // switch process.
    task_switch(handle_term->next_tid);//execute the next terminal.
}
