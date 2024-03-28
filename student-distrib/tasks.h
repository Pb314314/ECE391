//
// Created by Jordan(qishenz2@illinois.edu) on 2022/4/10.
//
#include "syscall.h"
#include "types.h"

#ifndef MP3_TASKS_H
#define MP3_TASKS_H

#define KERNEL_BOTTOM       0x00800000  // 8MB
#define KERNEL_STACK_SIZE   0x00002000  // 8KB
#define USER_STACK_SIZE     0x00400000  // 4MB

#define VIRTUAL_MAP_START   0x08000000  // 128MB
#define VIRTUAL_MAP_END     0x08400000  // 132MB
#define BUFFER_SIZE         128         // same for all buffer.

// program offsets.
#define PROGRAM_ADDR        0x08048000      // given entry point.
#define PROGRAM_START       PROGRAM_ADDR+24 // same entry for all program.

#define MAX_FILENAME_LENGTH 32
#define User_Level_Programs_Index 32        // 128MB/4MB = 32
// PCB struct from syscall.h

/* main execute function. */
int execute_task(const uint8_t* cmd,int target_num);
int halt_task(uint8_t status);
int task_switch(int next_term_id);

/* read arguments */
int parse_argument(const uint8_t* cmd, uint8_t* cmd_buffer, uint8_t* arg_buffer);

/* create pid when new execute function comes. */
int create_new_pid();

/* fill the pcb with init value. */
pcb_t* init_pcb(int next_pid, int parent_pid);

/* set up pages with 4MB for user program */
int set_task_page(int pid);

/* load execute file into kernel memory. */
int read_exe_file(const uint8_t* filename);

/* switch from R0 to R3. */
int goto_user_level();

/* helper function for allocate memory for pcb. */
pcb_t* get_pcb_block(uint32_t pid);

/* helper function to get current pid. */
int get_current_pid();

#endif //MP3_TASKS_H
