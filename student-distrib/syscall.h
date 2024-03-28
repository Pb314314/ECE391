#ifndef _SYSCALL_H
#define _SYSCALL_H


#include "types.h"

#define MAX_FD 8
#define MIN_FD 2
#define MAX_FILE_NAME 32
#define BUFFER_SIZE   128

#define _Eight_MB_ 0x800000
#define _Eight_KB_ 0x2000
#define NOT_USE 0
#define IN_USE 1
#define INIT_FILE_POS 0
#define VIDEO_MEMORY_INDEX 34

#define VIDEO_MM 0x8800000

typedef struct file_operation{
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
} file_operation_table_t;


/* file descriptor*/
typedef struct file_des{
    file_operation_table_t* file_op_table_ptr;
    uint32_t inode_num;
    uint32_t file_pos;
    uint32_t flag;
} file_des_t;


/* pcb */
typedef struct pcb{ 
    file_des_t file_des_array[MAX_FD];      // The fd_array for each process
    uint32_t pid;                           // pid of each process
    uint32_t parent_pid;                    // parent id. When process halt, execute parent process
    uint32_t exec_esp;                      // user stack esp
    uint32_t exec_ebp;                      // user stack ebp
    uint32_t tss_esp0;                      // ker
    uint32_t user_eip;
    uint32_t user_esp;

    uint8_t arg[BUFFER_SIZE];
} pcb_t;

file_operation_table_t File_Op_table;
file_operation_table_t RTC_Op_table;
file_operation_table_t Directory_Op_table;
file_operation_table_t Terminal_table;            // for stdio use.

extern void syscall_handler();

// halt
int32_t halt(uint8_t status);

// execute
int32_t execute(const uint8_t* command);

// read
int32_t read(int32_t fd, void* buf, int32_t nbytes);

// write
int32_t write(int32_t fd, const void* buf, int32_t nbytes);

// open
int32_t open(const uint8_t* filename);

// close
int32_t close(int32_t fd);

// get arguments
int32_t getargs(const void* buf, int32_t nbytes);

// map video
int32_t vidmap(uint32_t** screen_start); // ????????int32 or int8????
int vid_remap(uint8_t* address);

// set handler
int32_t set_handler(int32_t signum, void* handler_address);

// sigreturn
int32_t sigreturn(void);

/*-----------------helper functions---------------*/
pcb_t* get_pcb(int32_t pid);

void init_File_operations_table();

void init_Rtc_operations_table();

void init_Directory_operations_table();

void init_Terminal_table();

int32_t init_test_PCB();

int init_fd_array(file_des_t* fd_array);

void clear_fd_array();

// get fd from the current fd array.
file_des_t* get_fd_array();

// init all FOP at start of kernel.
int init_fop_table();

#endif // SYSCALLS_H
