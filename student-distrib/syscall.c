#include "syscall.h"
#include "x86_desc.h"
#include "filesystem.h"
#include "RTC.h"
#include "tasks.h"
#include "Terminal.h"


/*
 *  sys_halt(const uint8_t status)
 *      Decsription: terminate a process, return to parent process
 *      Inputs: status: halt status
 *      Return: return status
 *      Side-Effect: terminate process, restore parent data, paging and clear file descriptor
 */
int32_t halt(uint8_t status) {
    int ret = halt_task(status);
    return ret;
}

/*
 *  sys_execute(const uint8_t* command)
 *      Description: execute commands
 *      Inputs:  the command to execute
 *      Outputs: -1 on failure, 0 on success
 */
int32_t execute(const uint8_t* command) {
//    while(cur_terminal_id != handle_term->terminalID);
    int ret = execute_task(command, term_ptr->terminalID);
    if (ret == -1){
        printf("ERROR EXECUTE! \n");
    }
    return ret;
}

/*
 *  sys_read(int32_t fd, void* buf, int32_t nbytes)
 *      Description: system read
 *      Inputs: fd - file descriptor, buf - buffer, nbytes - max size to read
 *      Outputs: -1 on failure, number of read bytes on success
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    //printf("enter sysread!!\n");
    if(fd < 0 || fd > MAX_FD || buf == NULL || nbytes < 0|| fd==1){
        printf("The sysread input is wrong!\n");
        return -1;
    }
    pcb_t* current_pcb = get_pcb(get_current_pid());
    uint32_t whether_use = current_pcb->file_des_array[fd].flag;
    if(whether_use == NOT_USE){
        printf("file not exist\n");
        return -1;
    }
    int32_t read_num = current_pcb->file_des_array[fd].file_op_table_ptr->read(fd,buf,nbytes);
    //printf("return from sys_read!!!return value = %d\n", read_num);
    // reset the file position when read fail.
    // FIXME: now moved to the directory read and file read.

    return read_num;
}

/*
 *  sys_write(int32_t fd, void* buf, int32_t nbytes)
 *      Description: system write
 *      Inputs: fd - file descriptor, buf - buffer, nbytes - max size to read
 *      Outputs: -1 on failure, written file
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    //printf("enter write\n");
    //printf("fd = :%d, nbyte  = :%d\n", fd, nbytes);
    if(fd < 0 || fd > MAX_FD || buf == NULL || nbytes < 0 || fd==0){
        printf("input error!!!!6868666\n");
        return -1;
    }
    pcb_t* current_pcb = get_pcb(get_current_pid());
    uint32_t whether_use = current_pcb->file_des_array[fd].flag;
    if(whether_use == NOT_USE){
        printf("File not exist\n");
        return -1;
    }
    int32_t write_num = current_pcb->file_des_array[fd].file_op_table_ptr->write(fd,buf,nbytes);
    return write_num;
}

/*
 *  sys_open(const uint8_t* filename)
 *      Description: system open
 *      Inputs: filename - file to open
 *      Outputs: -1 on failure, 1 on success
 */
//put the file operation table pointer,inode,file position and flag in the FD array return the fd index
int32_t open(const uint8_t* filename) {

    int i;
    dentry_t dentry_found;
    int find_file = read_dentry_by_name(filename, &dentry_found);
    if(find_file == -1){  //file not exist
        printf("File not exist\n");
        return -1;
    }
    //printf("current pid is:%d\n",get_current_pid());
    pcb_t* current_pcb = get_pcb(get_current_pid());

    for(i=2;i<8;i++){
        //uint32_t flagg = current_pcb->file_des_array[i].flag;
        if(current_pcb->file_des_array[i].flag == 0){
            // find a free FD
            current_pcb->file_des_array[i].flag = 1;

            current_pcb->file_des_array[i].file_pos = 0;

            if(dentry_found.type == TYPE_RTC){                /* rtc*/
                //printf("Open RTC ...\n");
                current_pcb->file_des_array[i].inode_num = 0;
                current_pcb->file_des_array[i].file_op_table_ptr = &RTC_Op_table;
            }else if (dentry_found.type == TYPE_DIR){         /* directory */
                //printf("Open directory...\n");
                current_pcb->file_des_array[i].inode_num = 0;
                current_pcb->file_des_array[i].file_op_table_ptr = &Directory_Op_table;
            }else if (dentry_found.type == TYPE_FILE){         /* regular file */
                //printf("Open regular file...\n");
                current_pcb->file_des_array[i].inode_num = dentry_found.inode_num;
                current_pcb->file_des_array[i].file_op_table_ptr = &File_Op_table;
            }
            return i;
        }
    }
    printf("No free FD\n");
    return -1;
}

/*
 *  sys_close(int32_t fd)
 *      Description: system open
 *      Inputs: fd - file descriptor
 *      Outputs: -1 on failure, file close
 */
int32_t close(int32_t fd) {
    //printf("enter sysclose!!\n");
    //printf("close function input fd is: %d", fd);
     if(fd < MIN_FD || fd >= MAX_FD ){
        //printf("input fd out of range\n");
        return -1;
    }
    pcb_t* current_pcb = get_pcb(get_current_pid());
    uint32_t whether_use = current_pcb->file_des_array[fd].flag;
    if(whether_use == NOT_USE){
        printf("File not exist\n");
        return -1;
    }
    // fix: all the close return -1 for failure...
    if(current_pcb->file_des_array[fd].file_op_table_ptr->close(fd) == -1){
        return -1;
    }
    current_pcb->file_des_array[fd].flag = NOT_USE;
    return 0;
    }

/*
 *  int getargs
 *      DESCRIPTION: copy the args into the buffer.
 *      INPUT:  buf: pointer to the buffer.
 *              nbytes: number of bytes to be copied.
 *      OUTPUT: None.
 *      RETURN: number of char read, or -1 for FAIL.
 *      SIDE EFFECT: None.
 */
int32_t getargs(const void* buf, int32_t nbytes){

    uint8_t* bufptr;
    pcb_t*  cur_pcb = get_pcb(get_current_pid());
//    int i;
//    int num_print = 0;

    // check if the input is valid.
    if (buf == NULL || nbytes == 0){
        return -1;
    }
    bufptr = (uint8_t* )buf;
/*
    for (i = 0;i<nbytes;i++){
        bufptr[i] = cur_pcb->arg[i];
        num_print++;
    }
    */
   //printf("the args copy is: %d\n", cur_pcb->arg);
    strncpy((int8_t*)buf, (int8_t*)(cur_pcb->arg), nbytes);
    return 0;
}

/*
 *  vid_map
 *      DESCRIPTION: Map the video memory into user space started at 132MB-136MB. Size 4KB pages.
 *      INPUT:  screen_start: start pointer of the video memory.
 *      OUTPUT: None.
 *      RETURN: 0 for Success, 1 for FAIL.
 *      SIDE EFFECT: enable a new paging map to the physical VM and make the screen_start point to that vitual memory.
 */
int32_t vidmap(uint32_t** screen_start){
    //printf("enter vidmap!!!!!!!!!!!\n");
    if (!screen_start || (uint32_t)screen_start < USER_ADDR || (uint32_t)screen_start >= (USER_ADDR+SIZE_4MB))
        return -1;
    //0x8800000(program paging(to modify the physical VM))
    // set up at 136 MB USER_ADDR
    page_directory[VIDEO_MEMORY_INDEX].P = 1;
    page_directory[VIDEO_MEMORY_INDEX].PS = 0;  //4KB
    page_directory[VIDEO_MEMORY_INDEX].US = 1;
    page_directory[VIDEO_MEMORY_INDEX].Page_addr =((uint32_t)page_table_vidmap)/SIZE_4KB;
    page_directory[VIDEO_MEMORY_INDEX].RW = 1;

    page_table_vidmap[0].P = 1;//why page_table_vidmap[0]? because 0x88000000 is the first 4kb paging in the 132MB-136MB  
    page_table_vidmap[0].US = 1;
    page_table_vidmap[0].Page_addr = VIDEO_ADDR/SIZE_4KB;//map the program paging(DPL=3)?? to the Physicle VM
    page_table_vidmap[0].RW = 1;//read and write

    tlb_flush();

    *screen_start = (uint32_t*)(VIDEO_MM);  //0x8800000/4096 = 34816, 34816/1024 = 34 (the 34th 4MB(132MB-136MB))
    return 0;
}

/*
 *  vid_remap
 *      DESCRIPTION: map the program paging to the terminal buffer described by the address.
 *      INPUT:  the termminal buffer. input is the start address of the terminal buffer.
 *      OUTPUT: None.
 *      RETURN: 0 for Success, 1 for FAIL.
 *      SIDE EFFECT: .
 */
int vid_remap(uint8_t* address){
    if (address == NULL){
        return -1;
    }

    // set up at 136 MBUSER_ADDR
    page_directory[VIDEO_MEMORY_INDEX].P = 1;
    page_directory[VIDEO_MEMORY_INDEX].PS = 0;  //4KB
    page_directory[VIDEO_MEMORY_INDEX].US = 1;
    page_directory[VIDEO_MEMORY_INDEX].Page_addr =((uint32_t)page_table_vidmap)/SIZE_4KB;
    page_directory[VIDEO_MEMORY_INDEX].RW = 1;

    page_table_vidmap[0].P = 1;     
    page_table_vidmap[0].US = 1;
    page_table_vidmap[0].Page_addr = ((uint32_t)address)/SIZE_4KB;  //map the first 4kb paging in 132MB-136MB to the terminal buffer(calculated by address)
    page_table_vidmap[0].RW = 1;

    tlb_flush();
    return 0;
}

int32_t set_handler(int32_t signum, void* handler_address){
    return 1;
}
int32_t sigreturn(void){
    return 1;
}
/*-----------------------------helper functions--------------------------*/
pcb_t* get_pcb(int32_t pid){
    return (pcb_t*)(_Eight_MB_ - _Eight_KB_ *(pid+1));
}

void init_File_operations_table(){
    File_Op_table.open = file_open;
    File_Op_table.read = file_read;
    File_Op_table.write = file_write;
    File_Op_table.close = file_close;
}

void init_Rtc_operations_table(){
    RTC_Op_table.open = rtc_open;
    RTC_Op_table.read = rtc_read;
    RTC_Op_table.write = rtc_write;
    RTC_Op_table.close = rtc_close;
}

void init_Directory_operations_table(){
    Directory_Op_table.open = directory_open;
    Directory_Op_table.read = directory_read;
    Directory_Op_table.write = directory_write;
    Directory_Op_table.close = directory_close;
}

void init_Terminal_table(){
    Terminal_table.open = terminal_open;
    Terminal_table.read = terminal_read;
    Terminal_table.write = terminal_write;
    Terminal_table.close = terminal_close;
}


int32_t init_test_PCB(){
    int cur_pid = get_current_pid();
    pcb_t* current_pcb = get_pcb(cur_pid);
    file_des_t* FD_array =  current_pcb->file_des_array;
    init_fd_array(FD_array);
    return cur_pid;
}

/*
 *  init_fd_array
 *      DESCRIPTION: initialize the fd array for further use.
 *      INPUT: fd_array: FD array pointer.
 *      OUTPUT: None.
 *      RETURN: 0 for success, -1 for failure.
 *      SIDE EFFECT: modify the given FD array.
 */
int init_fd_array(file_des_t* fd_array){
    int i;

    if (fd_array == NULL){
        return -1;
    }

    // FIXME: set the NULL to the terminal fop pointer.

    // stdin FD
    fd_array[0].file_op_table_ptr = &Terminal_table;
    fd_array[0].file_pos = 0;
    fd_array[0].flag = 1;
    fd_array[0].inode_num = 0;
    // stdout FD
    fd_array[1].file_op_table_ptr = &Terminal_table;
    fd_array[1].flag = 1;
    fd_array[1].inode_num = 0;
    fd_array[1].file_pos = 0;

    // clear the following FD array.
    for(i=2;i<MAX_FD;i++){
        fd_array[i].file_pos = 0;
        fd_array[i].inode_num = 0;
        fd_array[i].flag = 0;
        fd_array[i].file_op_table_ptr = NULL;
    }

    return 0;
}

/*
 *  clear_fd_array
 *      DESCRIPTION: clean the fd array after use.
 *      INPUT: fd_array: FD array pointer.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: clear the fd array with given value.
 */
void clear_fd_array(){
    // not used yet.
}

/*
 *  get_fd_array
 *      DESCRIPTION: get the file descriptor from current fd array.
 *      INPUT:  fd: the file descriptor idx.
 *      OUTPUT: NOne.
 *      RETURN: the file descriptor pointer.
 *      SIDE EFFECT: NONE.
 */
file_des_t* get_fd_array(){
    // directly get the current FD operating in FS.
    pcb_t* cur_pcb = get_pcb(get_current_pid());
    return cur_pcb->file_des_array;

}

/*
 *  init_fop_table
 *      DESCRIPTION: init the File Operation table when the kernel start up.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: Always 0.
 *      SIDE EFFECT: Set the global fop table.
 */
int init_fop_table(){
    // call all the init function defined above.
    init_Terminal_table();
    init_File_operations_table();
    init_Directory_operations_table();
    init_Rtc_operations_table();
    return 0;
}
