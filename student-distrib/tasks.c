//
// Created by Jordan(qishenz2@illinois.edu) on 2022/4/10.
//
#include "tasks.h"
#include "types.h"
#include "paging.h"
#include "filesystem.h"
#include "syscall.h"
#include "x86_desc.h"
#include "lib.h"
#include "Terminal.h"

#define SUCCESS  0
#define FAILURE -1

volatile int pid_map[6] = {0,0,0,0,0,0};

volatile int cur_pid = -1;

volatile pcb_t* cur_pcb = NULL;

/*
 *  execute_task
 *      DESCRIPTION: execute the given command.
 *      INPUT:  command string that given by user.
 *              terminal: the target execute terminal.
 *      OUTPUT: None.
 *      RETURN: SUCCESS if No exception. or FAILURE.
 *      SIDE EFFECT: GO TO USER LEVEL after the end of the function.
 */
int execute_task(const uint8_t* cmd, int target_num){
    uint8_t filename[NAME_LENGTH+1];
    uint8_t argument[BUFFER_SIZE]={0};
    uint8_t magic_buffer[4];
    dentry_t current_dentry;
    pcb_t * last_pcb;
    terminal_t* execute_terminal;

    int parent_pid;
    int next_pid;

    // parse command before next check.

    cli();

    if (cmd == NULL){
        sti();
        return FAILURE;
    }
    if (target_num < 0 || target_num > 3){
        sti();
        return FAILURE;
    }
    if (parse_argument(cmd,filename,argument) == FAILURE){
        sti();
        return FAILURE;
    }

    // check for File's magic number.
    if(read_dentry_by_name(filename,&current_dentry)==FAILURE){
        sti();
        return FAILURE;
    }

    if (current_dentry.type != TYPE_FILE){
        sti();
        return FAILURE;
    }

    if (read_data(current_dentry.inode_num,0,magic_buffer,4)!=FAILURE){
        if (magic_buffer[0] != 0x7F || magic_buffer[1] != 0x45 ||
            magic_buffer[2] != 0x4C || magic_buffer[3] != 0x46){
            sti();
            return FAILURE;
        }
    } else{
        return FAILURE;
    }

    // try to allocate new PID.
    next_pid = create_new_pid();
    if (next_pid == FAILURE){
        sti();
        printf("Process Full. \n ");
        return FAILURE;
    }

    // create PCB for this pid.
    set_task_page(next_pid);

    // load the file into given user pages. PROGRAM_ADDR to the kernel space
    if (read_exe_file(filename)== FAILURE){
        sti();
        return FAILURE;
    }
    // load the pid into the given terminal.
    execute_terminal = get_terminal(target_num);
    parent_pid = execute_terminal->cur_pid;
    execute_terminal->cur_pid = next_pid;

    // INIT the PCB.
    cur_pcb = init_pcb(next_pid,parent_pid);

    strncpy((int8_t*)cur_pcb->arg, (int8_t*)argument, BUFFER_SIZE);

    // set the TSS: change to the stack pointer.

    tss.esp0 = cur_pcb->tss_esp0;
    tss.ss0 = KERNEL_DS;

    cur_pid = next_pid;

    // set the esp and ebp in the pcb.
    if (cur_pcb->parent_pid != -1) {
        last_pcb = get_pcb(cur_pcb->parent_pid);

        // move the esp and ebp into the pcb.
        asm volatile(
        "movl %%ebp, %0;"
        "movl %%esp, %1;"
        : "=r"(last_pcb->exec_ebp), "=r"(last_pcb->exec_esp)
        );
    }

    // go to ring 3.

    goto_user_level();

    return SUCCESS;
}

/*
 *  halt_task
 *      DESCRIPTION: halt the task with the given PID, return to parent process.
 *      INPUT: status: the address to be return.
 *      OUTPUT: None.
 *      RETURN: 0 for success.
 *      SIDE EFFECT: RETURN to parent process.
 */
int halt_task(uint8_t status){
    int32_t fd;

    file_des_t* fd_array;   /* current fd array. */

    cli();

    fd_array = get_fd_array();

    /* close related files, clear the fd array */
    fd = 0;

    while(fd < MAX_FD){
        if(fd_array[fd].flag)
            close(fd);
        fd++;
    }

    /* restart the base shell if halting it */
    if(cur_pcb->parent_pid == -1){
        sti();
        pid_map[cur_pid] = 0;
        cur_pid = -1;
        handle_term->cur_pid = -1;
        // the pit will restart the terminal.
        execute((uint8_t*)"shell");     /* restart the base shell */
    }
    else {
        /* restore parent paging */
        set_task_page(cur_pcb->parent_pid);

        /* restore tss's kernel stack registers */
        tss.ss0 = KERNEL_DS;
        tss.esp0 = KERNEL_BOTTOM - KERNEL_STACK_SIZE * cur_pcb->parent_pid - 4; /* 4-byte for int32_t */

        /* switch current process to parent process */
        // free the bitmap for process.
        pid_map[cur_pid] = 0;
        cur_pid = cur_pcb->parent_pid;
        cur_pcb = get_pcb(cur_pid);

        sti();
        /* restore parent's esp and ebp, ready to return back */
        asm volatile(
        "movl   %0, %%esp   ;"     /* restore %esp */
        "movl   %1, %%ebp   ;"     /* restore %ebp */
        "movl   $0, %%eax   ;"
        "movb   %2, %%al    ;"     /* return status */
        "leave              ;"
        "ret                ;"     /*need to jump to execute return */
        : /* no output */
        : "r" (cur_pcb->exec_esp), "r" (cur_pcb->exec_ebp), "r" (status)
        : "esp", "ebp", "eax"
        );
    }


    return SUCCESS;
}

/*
 *  task_switch
 *      DESCRIPTION: switch the task to given pid.
 *      INPUT: next_term_id, the terminal will execute in this function.
 *      OUTPUT: None.
 *      RETURN: 0 FOR success, -1 for FAIL.
 *      SIDE EFFECT: process switch. the video memory will not affected.
 *      REMINDER: the video mapping was not affected in this program.
 */
int task_switch(int next_term_id){
    pcb_t* next_pcb;
    terminal_t* next_terminal;
    // check if the current status is valid.
    // wait for first execute to activate the task switch.
    if (cur_pid == -1||cur_pcb == NULL){
        return FAILURE;
    }
    if (next_term_id == -1|| next_term_id == handle_term->terminalID){
        // no next process. do nothing.
        return SUCCESS;
    }
    // store the current terminal info.
    handle_term->cur_pid = cur_pid;//store the pid information of zhiqian yunxingde terminal (save to the list(will restore in the next execute))
    next_terminal = get_terminal(next_term_id);//find the terminal will execute now
    next_pcb = get_pcb(next_terminal->cur_pid);//find the pcb of the terminal execute now

    // store current stack info.
    asm volatile(
    "movl %%ebp, %0;"
    "movl %%esp, %1;"
    : "=r"(cur_pcb->exec_ebp), "=r"(cur_pcb->exec_esp)
    );
    cur_pcb->tss_esp0 = tss.esp0;

    // update local variables
    handle_term = next_terminal;

    // check if the terminal has valid process running.
    if (handle_term->cur_pid < 0 || handle_term->cur_pid > 5){
        if (handle_term->cur_pid == -1){
            // run new shell on the terminal, but not chain it into the terminal chain.
            printf("Current located in TTY %d. \n",handle_term->terminalID+1);
            execute_task((uint8_t*)"shell",handle_term->terminalID);
        } else {
            return FAILURE;
        }
    } else{
        cur_pid = handle_term->cur_pid;//update the global variable(to execute the current terminal)
        cur_pcb = next_pcb;
        // load next stack info.
        asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        :
        : "r"(cur_pcb->exec_esp), "r"(cur_pcb->exec_ebp)
        : "esp", "ebp"
        );
        tss.esp0 = cur_pcb->tss_esp0;
        tss.ss0 = KERNEL_DS;

    }

    // set page for next process.
    set_task_page(handle_term->cur_pid);

    // actually this part is not necessary because the return works the same thing.
    // just use it to make the logic more clear.
    asm volatile(
    "leave ;"
    "ret   ;"
    );

    return SUCCESS;
}


/*
 *  parse_argument
 *      DESCRIPTION: Parse the given argument into pcb.
 *      INPUT: cmd: the given command from input.
 *             cmd_buffer: buffer to load the command divided from cmd.
 *             arg_buffer: buffer to store the args separated from cmd.
 *      OUTPUT: None.
 *      RETURN: SUCCESS for success, FAIL for FAILURE.
 *      SIDE EFFECT: None.
 */
int parse_argument(const uint8_t* cmd, uint8_t* cmd_buffer, uint8_t* arg_buffer){
    int command_len = strlen((int8_t*)cmd);

    int argument_len = 0;
    int i = 0;

    while (cmd[i] != ' ' && cmd[i]!= '\0'){
        /* check whether the file name is too long */
        if (i >= MAX_FILENAME_LENGTH){
            return FAILURE;
        }
        /* copy file name */
        cmd_buffer[i] = cmd[i];
        i++;
    }
    cmd_buffer[i] = '\0';

    /* skip the space between file name and argument */
    while (cmd[i] == ' ' && i < command_len) {i++;}

    /* copy argument */
    while (i < command_len){
        arg_buffer[argument_len] = cmd[i];
        i++;
        argument_len++;
    }
    arg_buffer[argument_len] = '\0';
    
    return SUCCESS;
}

/*
 *  set_task_page
 *      DESCRIPTION: set a 4MB page for process to run.
 *      INPUT:  pid: the process id.
 *      OUTPUT: None.
 *      RETURN: SUCCESS for success, FAIL for failure.
 *      SIDE EFFECT: the memory map was changed.
 */
int set_task_page(int pid){
    // set pages.
    uint32_t phys_addr = (2 + pid)*SIZE_4MB;                                     //start at 8MB-12MB, one process one page
    page_directory[User_Level_Programs_Index].Page_addr = phys_addr/SIZE_4KB;   // >> 12 bits.(map the virtual memory 128MB-132MB to the current program physicle paging.)
    page_directory[User_Level_Programs_Index].PS = 1;
    page_directory[User_Level_Programs_Index].P  = 1;
    page_directory[User_Level_Programs_Index].US = 1;
    page_directory[User_Level_Programs_Index].PWT = 0;
    page_directory[User_Level_Programs_Index].PCD = 0;
    page_directory[User_Level_Programs_Index].G = 0;
    tlb_flush();
    return SUCCESS;
}

/*
 *  create_new_pid
 *      DESCRIPTION: TRY TO allocate a new pid.
 *      INPUT: None.
 *      OUTPUT: NOne.
 *      RETURN: pid for new pid, FAILURE for not allocated.
 *      SIDE EFFECT: modify the pid bit map array.
 */
int create_new_pid(){

    int next_pid;

    if(cur_pid == -1){
        pid_map[0] = 1;
        return 0;

    } else{
        for(next_pid=0;next_pid < 6;next_pid++){
            if (pid_map[next_pid] == 0){
                pid_map[next_pid] = 1;
                return next_pid;
            }
        }

        printf("Too many process. Try again later.\n");
        return FAILURE;
    }
}

/*
 *  init_pcb
 *      DESCRIPTION: init the PCB and the according structure FD
 *      INPUT: next_pid: given pid to init.
 *      OUTPUT: set the current pointers to the created PCB.
 *      RETURN: 0 for SUCCESS, -1 for FAIL
 *      SIDE EFFECT: None.
 */
pcb_t* init_pcb(int next_pid, int parent_pid){
    // get the process struct for the given pid.
    pcb_t* pcb;
    pcb = get_pcb_block(next_pid);
    pcb->pid = next_pid;

    // add terminals here.
    // the parent pid can be -1.
    pcb->parent_pid =  parent_pid;

    // set the tss.
    pcb->tss_esp0 = KERNEL_BOTTOM - KERNEL_STACK_SIZE*next_pid;

    // init the fd array.
    init_fd_array(pcb->file_des_array);

    // map the user address into virtual address: 132MB!
    pcb->user_esp = USER_ADDR+USER_STACK_SIZE-sizeof(uint32_t);

    return pcb;
}

/*
 *  read_exe_file
 *      DESCRIPTION: read executable file and copy it to the allocated memory.
 *      INPUT: filename: name of the file that need to be execute.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: copy the file from user level to kernel level.
 */
int read_exe_file(const uint8_t* filename){

    dentry_t temp_dentry;
    int length;
    int ret;
    // validation check performed before the function called.
    read_dentry_by_name(filename, &temp_dentry);

    length = get_file_length(&temp_dentry);

    // copy the program to the kernel space.
    ret = read_data(temp_dentry.inode_num, 0,(uint8_t*)PROGRAM_ADDR,length);

    if (ret == -1){
        return -1;
    }
    return ret;
}

/*
 *  goto_user_level
 *      DESCRIPTION: set up stacks and eip and then jump to user level by IRET.
 *      INPUT:  None.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: GO TO Ring 3.
 */
int goto_user_level(){

    // Load into Register.
    uint32_t ESP = VIRTUAL_MAP_END;
    uint32_t EIP = *(uint32_t*)((uint8_t*)PROGRAM_ADDR + 24);
    cur_pcb->user_eip = EIP;
    sti();

    asm volatile(
    "movw  %%ax, %%ds;"
    "pushl %%eax;"
    "pushl %%ebx;"
    "pushfl     ;"
    "pushl %%ecx;"
    "pushl %%edx;"
    "iret"
    :
    : "a"(USER_DS), "b"(ESP), "c"(USER_CS), "d"(EIP)
    : "cc", "memory"
    );

    return SUCCESS;
}

/*
 *  get_pcb_block
 *      DESCRIPTION: fetch current PCB block pointer.
 *      INPUT: NOne.
 *      OUTPUT: NOne.
 *      RETURN: Pointer to current pcb. None if not found.
 *      SIDE EFFECT: NOne.
 */
pcb_t* get_pcb_block(uint32_t pid){
     return (pcb_t*)(KERNEL_BOTTOM - KERNEL_STACK_SIZE*(pid+1));
}

 /*
  *  get_current_pid
  *     DESCRIPTION: return the current pid.
  *     INPUT: None.
  *     OUTPUT: None.
  *     RETURN: cur_pid recorded in this file.
  *     SIDE EFFECT:None.
  */
 int get_current_pid(){
     return cur_pid;
 }

