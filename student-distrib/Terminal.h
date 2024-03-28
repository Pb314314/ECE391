//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/25.
//

#ifndef MP3_TERMINAL_H
#define MP3_TERMINAL_H


#include "paging.h"
#include "keyboard.h"
#include "lib.h"
#include "syscall.h"


#define BUFFER_SIZE     128         // one page can contain 128 line.
#define OUTPUT_SIZE     512         // one page can contain 512 bytes.
#define OBMINSP         64          // not being used.

#define CURSOR_COMMAND  0x3D4
#define CURSOR_DATA     0x3D5

#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25

// Terminal structure
typedef struct terminal_t{
    int  terminalID;                // terminal id.
    int  cur_pid;                  // terminal current process id.
    int  next_tid;                  // next terminal id. (the RR loop.)
    uint8_t* video_ptr;             // video mapper location.
    char input_buffer[BUFFER_SIZE]; // terminal input buffer.
    int  isem;                      // terminal input buffer location.
    int  status;                    // terminal status for terminal read.
    int  cursor_x;                  // cursor location for current terminal.
    int  cursor_y;

}terminal_t;

// current displaying terminal pointer.
extern volatile terminal_t* term_ptr;

// currently handling terminal ptr.
extern volatile terminal_t* handle_term;

// currently displaying terminal ptr.
extern volatile int cur_terminal_id;

/* initialize a terminal. */
void terminal_init();

/* switch the terminals. */
void terminal_switch(int term_id);

/* system call handle for terminal. */
int terminal_open(const uint8_t* filename);
int terminal_close(int32_t fd);
int terminal_read(int32_t fd, void* buffer, int32_t nbytes);
int terminal_write(int32_t fd, const void* buffer, int32_t nbytes);

/* cursor handle for terminal in text mode. */
void enable_cursor(uint8_t start, uint8_t end);
void disable_cursor();
void moving_cursor(int pos_x, int pos_y);

/* helper function to clean the terminal buffer. */
void clear_buffer();

void set_terminal_pages(int terminal_num, int global);

terminal_t* get_terminal(int term_id);

#endif //MP3_TERMINAL_H
