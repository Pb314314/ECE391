//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/25.
//

#include "Terminal.h"

#define SUCCESS 0
#define FAILURE -1

void clear_buffer();
void enable_cursor(uint8_t start, uint8_t end);
void moving_cursor(int pos_x, int pos_y);

terminal_t terminal_list[3];

// current displaying terminal pointer.
volatile terminal_t* term_ptr = NULL;

// current displaying terminal id.
volatile int cur_terminal_id;

// current handle terminal pointer.
volatile terminal_t* handle_term = NULL;


/*
 *  terminal_init
 *      DESCRIPTION: initialize a terminal and show it on screen after test.
 *      INPUT:       None.
 *      OUTPUT:      None.
 *      RETURN:      None.
 *      SIDE EFFECT: Modify the video memory.
 */
void terminal_init(){
    int i;
    // terminal init:
    for (i = 0;i<3;i++){
        terminal_list[i].isem = 0;
        terminal_list[i].status = 0;
        terminal_list[i].terminalID = i;
        memset(terminal_list[i].input_buffer,0,BUFFER_SIZE);    // clean the input buffer,
        terminal_list[i].cursor_x = 0;                             // init the cursor location.
        terminal_list[i].cursor_y = 0;
        terminal_list[i].video_ptr = (uint8_t*)(VIDEO_ADDR+(i+1)*SIZE_4KB);
        terminal_list[i].cur_pid = -1;                              // the pid that currently running in the terminal.
        terminal_list[i].next_tid = (i+1)%3;                        // the next terminal id in the rr loop.
        set_terminal_pages(i,1);                              // init the paging for the terminals.
    }
    clear();
    term_ptr = terminal_list;                                       // open the first terminal.
    handle_term = terminal_list;                                    // only one terminal in the RR loop.
    cur_terminal_id = 0;                                            // set the current terminal id to 0;
    set_terminal_pages(i,1);                                  // set the first terminal page as global.
    enable_cursor(0,SCREEN_HEIGHT);                            // set the cursor location.
    term_ptr->cursor_x = screen_x;
    term_ptr->cursor_y = screen_y;
    moving_cursor(screen_x,screen_y);
}

//used when alt F1/2/3 is pressed. About the terminal showing switching, not the execute purpose.
/*
 *  terminal_switch
 *      DESCRIPTION: switch terminal to the desire one. Enable terminals when it is not activated.
 *      INPUT:  int term_id: the given terminal id.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: The vid map of the terminal is changed.
 */
void terminal_switch(int term_id){
    if (term_id == cur_terminal_id){
        return;
    }

    cli();
    terminal_t* next_terminal = get_terminal(term_id);


    // 1. save current status into cur terminal.
    term_ptr->cursor_x = screen_x;
    term_ptr->cursor_y = screen_y;

    // 2. load status from the new terminal.
    screen_x = next_terminal->cursor_x;
    screen_y = next_terminal->cursor_y;

    // 3. Save old video memory && load new memory.
    memcpy((void*)term_ptr->video_ptr,(void*)VIDEO_ADDR, SIZE_4KB);//save ole one
    memcpy((void*)VIDEO_ADDR, (void*)next_terminal->video_ptr,SIZE_4KB);//load new one
    // 4. update local variables && cursor.
    term_ptr = next_terminal;//term_ptr always point to the terminal showing
    cur_terminal_id = term_ptr->terminalID;

    moving_cursor(screen_x,screen_y);
    // 5. remap the video memory.
    // the terminal has running programs on it. check whether it is being handled.

    // if the terminal is currently being handled:
    if (cur_terminal_id == handle_term->terminalID) {
        // 1. remap it to the video map.
        vid_remap((uint8_t*)VIDEO_ADDR);
    } else {
        // map the handled terminal to the buffer.
        vid_remap((uint8_t*)term_ptr->video_ptr);
    }
    sti();
    return;
}
/*
 *  terminal_open
 *      DESCRIPTION: Standard open system call. Do noting for terminal.
 *      INPUT/OUTPUT/RETURN: None.
 *      SIDE EFFECT: None.
 */
int terminal_open(const uint8_t* filename){
    // do noting.
    clear_buffer();
    return SUCCESS;
}

/*
 *  terminal_close
 *      DESCRIPTION: Standard close system call. Do nothing for terminal.
 *      INPUT/OUTPUT/RETURN: None.
 *      SIDE EFFECT: None.
 */
int terminal_close(int32_t fd){
    // do nothing.
    clear_buffer();
    return SUCCESS;
}


//use from syscall.c sys_read, every time read input characters to the terminal call terminal_read
/*
 *  terminal_read
 *      DESCRIPTION: Copy the content in the input buffer to the target buffer.
 *      INPUT: buffer: pointer to the target buffer.
 *      OUTPUT: number of the
 *      RETURN: Numbers of chars that being copied.
 *      SIDE EFFECT: clear the key board buffer.
 */
int terminal_read(int32_t fd, void* buf, int32_t nbytes){
    //printf("enter terminal read");
    // Check if the input is valid.
    int i;
    int num_read = 0;
    char* buffer = (char*)buf;

    if (buf == NULL || nbytes == 0){
        return FAILURE;
    }
    term_ptr->status = 0;
    // wait until the terminal is free.
    while(!handle_term->status){}
    handle_term->status = 0;
    for (i = 0; (i < nbytes)&&(i<BUFFER_SIZE);i++) {
        if (buffer[i] != '\n') {
            if (i < handle_term->isem) {
                num_read++;
                buffer[i] = handle_term->input_buffer[i];
            }
        }else {
                // read until the \n and then break the loop.
                buffer[i] = handle_term->input_buffer[i];
                num_read++;
                break;
        }
    }

    // fill the buffer fully with NULL.
    if (nbytes > handle_term->isem){
        for (;i<nbytes;i++){
            buffer[i] = 0;
        }
    }
    // clean key board buffer.
    clear_buffer();
    return num_read;
}



//use from syscall.c sys_write, every time write to the terminal call terminal_write
/*
 *  terminal_write
 *      DESCRIPTION: write the content in the given buffer in the terminal.
 *      INPUT:       buffer: the pointer to the given buffer.
 *                   count:  the number of chars that should be put.
 *      OUTPUT: the contend in the buffer.
 *      RETURN: number of char written.
 *      SIDE EFFECT: the video memory was modified.
 */
int terminal_write(int32_t fd, const void* buffer, int nbytes){
    //printf("enter terminal Write");
    int i;
    // help change the pointer type.
    cli();
    uint8_t* buffpointer;
    int num_print = 0;

    // check if the input is valid.
    if (buffer == NULL || nbytes == 0){
        return FAILURE;
    }
    buffpointer = (uint8_t* )buffer;

    for (i = 0; i<nbytes;i++) {
        if (buffpointer[i] != '\0') {            // Skip null
            terminal_putc(buffpointer[i]);      //new version of puts!(change VM when handle_term==showing term| change term_buffer when handle_term!=showing_term)
            num_print++;
        }
    }
    sti();
    return 0;
}

/*
 *  clear_buffer
 *      DESCRIPTION: clean the current terminal buffer of the given terminal.
 *      INPUT/OUTPUT: None.
 *      RETURN: None,
 *      SIDE EFFECT: clear the buffer for the terminal.
 */
void clear_buffer(){
    if (term_ptr == NULL){
        return;
    }
    memset((void*)term_ptr->input_buffer,0,BUFFER_SIZE);
    term_ptr->isem = 0;
}

/*
 *  get_terminal
 *      DESCRIPTION: helper function to return terminal pointer by the given pid.
 *      INPUT: int terminal id;
 *      OUTPUT: None.
 *      RETURN: terminal pointer or NULL for fail.
 *      SIDE EFFECT: NOne.
 */
terminal_t* get_terminal(int term_id){
    return (terminal_list+term_id);
}

/*
 *  set_terminal_pages
 *      DESCRIPTION: set the pages for the terminal.
 *      INPUT: terminal_id: the id of the terminal.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: the pages at 8 to 16 KB.
 */
void set_terminal_pages(int terminal_num, int global){
    int index;
    if (terminal_num > 3 || terminal_num < 0){
        return;
    }

    index = (uint32_t)terminal_list[terminal_num].video_ptr/SIZE_4KB;

    page_table[index].RW = 1;
    page_table[index].US = 0;
    page_table[index].A = 0;
    page_table[index].Avail = 0;
    page_table[index].D = 0;
    page_table[index].G = global;
    page_table[index].Page_addr = index;
    page_table[index].P = 1;
    page_table[index].PAT = 0;
}

/*
 *  cursor part referred from:
 *  Text mode cursor. Text Mode Cursor - OSDev Wiki. (n.d.).
 *  Retrieved March 26, 2022, from https://wiki.osdev.org/Text_Mode_Cursor
 */

/*
 *  enable_cursor
 *      DESCRIPTION: enable the cursor by setting the start and end scanline.
 *      INPUT:  start: start scanline of the cursor. usually set to 0.
*               end:    end scanline of the cursor. max is 15.
 *      OUTPUT: cursor in the text mode.
 *      RETURN: None.
 *      SIDE EFFECT: modify the cursor register.
 */
void enable_cursor(uint8_t start, uint8_t end){
    outb(0x0A,CURSOR_COMMAND);
    outb((inb(CURSOR_DATA)&0xC0)|start, CURSOR_DATA);

    outb(0x0B, CURSOR_COMMAND);
    outb((inb(CURSOR_DATA)&0xE0)|end, CURSOR_DATA);
}

/*
 *  disable_cursor
 *      DESCRIPTION: disable the cursor by set the cursor register.
 *      INPUT: None.
 *      OUTPUT: None,
 *      RETURN: None.
 *      SIDE EFFECT: disable the cursor in text mode.
 */
void disable_cursor(){
    outb(0x0A, CURSOR_COMMAND);     // select cursor shape register.
    outb(0x20, CURSOR_DATA);        // bit 5 will disable the cursor.
}

/*
 *  moving_cursor
 *      DESCRIPTION: update the cursor position with the given X and Y.
 *      INPUT:  pos_x, pos_y: the desire cursor location on the screen.
 *      OUTPUT: the cursor position update.
 *      RETURN: None.
 *      SIDE EFFECT: cursor position register was modified.
 */
void moving_cursor(int pos_x, int pos_y){

    uint16_t pos = pos_y*SCREEN_WIDTH + pos_x;

    outb(0x0F, CURSOR_COMMAND);
    outb((uint8_t)(pos&0xFF), CURSOR_DATA);
    outb(0x0E, CURSOR_COMMAND);
    outb((uint8_t)((pos>>8)&0xFF),CURSOR_DATA);
}

