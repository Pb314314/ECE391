//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/27.
//

#include "filesystem.h"
#include "syscall.h"

// why dont call it SSE FS?
// SSE is for SuperSimpleExt.

// local parameter updater
int next_dir();

/*
 *  the image was encodes the location of the core image using a block list format.
 *
 *  block list:
 *  |Boot block |FIRST INODE|......|FIRST BLOCK|.....|64
 *  |SSE_POINTER|+1         |......|+inodes num|.....|
 */

#define SUCCESS 0
#define FAILURE -1

/* Struct pointers. */
void*       SSE_pointer;        // start address backup.
boot_t*     boot_ptr;           // equal to the start address.
inode_t*    inode_start;        // block 2 start addr.
block_t*    block_start;        // data block pointer.

volatile static int         dir_loc  = -1;       // current directory count. point to the root block for default.
volatile static dentry_t*   location = NULL;    // test.
                dentry_t    temp;               // allocate somewhere instead of put it on stack.
/*
 *  filesystem_init
 *      DESCRIPTION: Load the pointer given by the kernel to the file struct.
 *      INPUT:  start_addr: Mount point of the file system.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: None.
 */
void filesystem_init(void * start_addr){
    SSE_pointer = start_addr;
    boot_ptr = (boot_t*)start_addr;
    inode_start    = (inode_t*)(boot_ptr+1);                        // the pointer will automatically pass the whole structure.
    block_start   = (block_t*)(inode_start+boot_ptr->inode_count);  // so just use correct structure pointer to add offsets.
}

/*
 *  read_entry_by_name
 *      DESCRIPTION: linear search the whole entry by look up names.
 *                   then copy the dentry to the given position.
 *      INPUT:  fname: char pointer to the file name.
 *              dentry: the dentry we should copy to.
 *      OUTPUT: None.
 *      RETURN: SUCCESS for find and copied.
 *              FAILURE for otherwise.
 *      SIDE EFFECT: modify the given memory location.
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    // local var allocation.
    int i;
    int length;
    int cmpflag = 1;
    dentry_t* temp_ptr;
    if (fname == NULL){
        return FAILURE;
    }
    // set the buffer to all NULL in case that the char length is 32.
    // then the char can be null-terminated.
    char filename[NAME_LENGTH+1] = {0};
    char dirname[NAME_LENGTH+1] = {0};
    // first check for valid input:
    length = strlen((char*)fname);
    if (length>NAME_LENGTH || dentry == NULL){
        return FAILURE;
    }

    // load the file name to local buffers.
    strcpy(filename,(int8_t *)fname);

    // note that the name in the dir is not terminated by NULL.
    for(i=0;i < boot_ptr->dir_count; i++){
        temp_ptr = &boot_ptr->dentries[i];

        if (strlen(temp_ptr->filename) == length){
            cmpflag = strncmp((int8_t*)temp_ptr->filename,filename,length);
        }
        else{
            if (strlen(temp_ptr->filename) > NAME_LENGTH){
                // copy the name out to the buffer to make it nul-terminated.
                strncpy(dirname, (char *) temp_ptr->filename, NAME_LENGTH);
                cmpflag = strncmp(dirname, filename,NAME_LENGTH);
            }
        }
        if (!cmpflag){
            // load the pointer with dentry structure,
            *dentry = boot_ptr->dentries[i];
            // update the current location to the global var.
            dir_loc = i;
            return SUCCESS;
        }
    }
    return FAILURE;
}

/*
 *  read_dentry_by_index
 *      DESCRIPTION: linear search the whole entry by index look up.
 *                   link the pointer to the target entry if possible.
 *      INPUT:  idx: the index of the entry.
 *              dentry: the pointer to be assigned.
 *      OUTPUT: None,
 *      RETURN: None.
 *      SIDE EFFECT: the memory will be modified at target address.
 */
int32_t read_dentry_by_index(uint32_t idx, dentry_t* dentry){

    // check for valid input.
    if (idx >= boot_ptr->dir_count || dentry == NULL){
        return FAILURE;
    }
    // load the pointer with dentry structure.
    *dentry = boot_ptr->dentries[idx];
    dir_loc = idx;
    return SUCCESS;
}

/*
 *  read_data
 *      DESCRIPTION: read the data from the given inodes in to the buffer and return
 *                   the number of bytes read.
 *      INPUT:  index_idx: the index of the node.
 *              offset: byte offset from the start of the files.
 *              buff:   the target buffer pointer.
 *              nbytes: number of bytes should be copied.
 *      OUTPUT: None.
 *      RETURN: Number of bytes being copied. FAILURE for error.
 */
int32_t read_data(uint32_t inode_idx, uint32_t offset, uint8_t* buff, uint32_t nbytes){
    int i,j;
    int read_count = 0;
    int data_block_start,data_block_end,data_block_id;
    int left_bytes;
    inode_t* target_node;
    block_t* target_block;

    // check if the input is valid.
    if (buff == NULL || nbytes == 0||inode_idx> boot_ptr->inode_count){
        return FAILURE;
    }
    // look up the target inodes.
    target_node = (inode_t*)(inode_start+inode_idx);

    if (offset+nbytes > target_node->length){

        //printf("TOO MANY CHARS: %d \n", offset+nbytes);
        //return 0;
        nbytes = target_node->length - offset;
        if(nbytes ==0){
            return 0; // no need to read
        }
    };

    // find out the start block we should read.
    data_block_start = offset/(BLOCK_SIZE);
    data_block_end   = (offset+nbytes)/(BLOCK_SIZE);

    // check if the start point and end point is out of the target node.
    
    left_bytes = nbytes;//all need to copy nbyte characters

    // read all the data block.
    //printf("the data copy from the file is:");
    for (i = data_block_start; i <= data_block_end; i++){

        data_block_id = target_node->blocks_num[i];
        target_block = (block_start+data_block_id);

        if (/*left_bytes > (BLOCK_SIZE) ||*/ ((offset+ read_count)/(BLOCK_SIZE) < (offset+nbytes)/(BLOCK_SIZE)  ) ){
            // current block should copy all the bytes into the buffer.
            
            if (i == data_block_start) {
                // calculate the start offset.
                j = offset % (BLOCK_SIZE);//j in the start index of the data block
            } else{
                j = 0;
            }
            // copy in progress.

            for (;j<BLOCK_SIZE;j++) {
                //printf("%d",target_block->data[j]);
                buff[read_count]= target_block->data[j];
                read_count++;
            }
            left_bytes  = nbytes - read_count;
        } else{
            // current at the end of the data block.
            if (i == data_block_start)  {
                // calculate the start offset.
                j = offset % (BLOCK_SIZE);
                for(;j<(offset%BLOCK_SIZE) + left_bytes;j++){
                //printf("%d",target_block->data[j]);
                    buff[read_count] = target_block->data[j];
                    read_count++;
                } 
                left_bytes  = nbytes - read_count;
            }
            else{
                j = 0;
                for(;j<left_bytes;j++){
                //printf("%d",target_block->data[j]);
                    buff[read_count] = target_block->data[j];
                    read_count++;
                }
                left_bytes  = nbytes - read_count;
            }
            // copy the data.
            
        }
    }

    return read_count;
}



// directory system call function.

/*
 *  directory_open
 *      DESCRIPTION: open a directory. do nothing now.
 *      INPUT:  filename: the name of the file.
 *      OUTPUT: None.
 *      RETURN: 0 for SUCCESS, -1 for FAILURE
 *      SIDE EFFECT: None.
 */
int directory_open(const uint8_t* filename){

    dentry_t to_be_open;
    if (!read_dentry_by_name(filename, &to_be_open)){
        if (to_be_open.type != 1){
            return FAILURE;
        }
        location = &to_be_open;
        return SUCCESS;
    }
    location = NULL;
    dir_loc = -1;
    return FAILURE;
}

/*
 *  directory_close
 *      DESCRIPTION: close an Directory and do noting right now.
 *      INPUT:  fd: not used because the syscall does all for it.
 *      OUTPUT: None.
 *      RETURN: SUCCESS
 *      SIDE EFFECT: None.
 */
int directory_close(int32_t fd){

    if (fd < 2){
        return FAILURE;
    }

    location = NULL;
    dir_loc = -1;
    return SUCCESS;
}

/*
 *  directory_read
 *      DESCRIPTION: read current directory and print its name
 *                   on the buffer.
 *      INOUT:  fd: Not used.
 *              buf: buffer pointer. copy target.
 *              nbytes: the number of bytes that should be read.
 *      OUTPUT: None.
 *      RETURN: Numbers of char that being read. FAILURE for error.
 *      SIDE EFFECT: the buffer will be filled.
 */
int directory_read(int32_t fd, void* buf, int32_t nbytes){
    //printf("enter directory read!!\n");
    int8_t buffer[NAME_LENGTH+1] = {0};
    int i;
    int length;

    file_des_t* cur_fd_array;
    cur_fd_array = get_fd_array();

    // check if the input is valid.
    if (buf == NULL|| nbytes == 0){
        printf("wrong input\n");
        return FAILURE;
    }

    if (cur_fd_array[fd].flag == 0){
       printf("directory file flag is zero\n");
       return FAILURE;
    }
    //printf("print :file_pos%d, dir_count: %d\n",cur_fd_array[fd].file_pos, boot_ptr->dir_count);
    if (cur_fd_array[fd].file_pos > boot_ptr->dir_count){
        // reset the file pos when it read to the end.
        cur_fd_array[fd].file_pos = 0;
        // update local parameters.
        read_dentry_by_index(0,&temp);
        location = &temp;
        dir_loc = 0;
        printf("wrong3 beyond the end of length\n");
        return FAILURE;
    }


    for (i = 0;i<NAME_LENGTH;i++){
        buffer[i] = boot_ptr->dentries[cur_fd_array[fd].file_pos].filename[i];
    }
    strcpy((int8_t*)buf, buffer);

    length = strlen(buffer);

    // update position
    cur_fd_array[fd].file_pos += 1;

    // update local storage.
    next_dir();

    return length;
}

/*
 *  directory_write
 *      DESCRIPTION: Not used in a read only file system.
 *      INPUT:  None.
 *      OUTPUT: NOne.
 *      RETURN: SUCCESS.
 *      SIDE EFFECT: None.
 */
int directory_write(int32_t fd, const void* buf, int32_t nbytes){
    printf(" READ ONLY FS. \n");
    return FAILURE;
}

/*
 *  file_open
 *      DESCRIPTION: open a file with given filename.
 *                   FD will record the file... but not here!
 *      INPUT:  filename: the name of the file.
 *      OUTPUT: None.
 *      RETURN: SUCCESS for success, FAILURE for error.
 *      SIDE EFFECT: None.
 */
int file_open(const uint8_t* filename){

    dentry_t open_temp;
    if (!read_dentry_by_name(filename,&open_temp)){
        if (open_temp.type ==2){
            location = &open_temp;
            return SUCCESS;
        }
    }
    return FAILURE;
}

/*
 *  file_close
 *      DESCRIPTION: Do nothing until now.
 *      INPUT:  None.
 *      OUTPUT: None.
 *      RETURN: SUCCESS.
 *      SIDE EFFECT: None.
 */
int file_close(int32_t fd){
    // can not close stdin and stdout.
    if (fd < 2){
        return FAILURE;
    }
    dir_loc = -1;
    location = NULL;
    return SUCCESS;
}

/*
 *  file_read
 *      DESCRIPTION: read n bytes from the current opened file.
 *      INPUT:  fd: not used.
 *              buffer: the output buffer.
 *              nbytes: the number of the.
 *      OUTPUT: None.
 *      RETURN: numbers of bytes that read, or -1 for FAIL.
 *
 */
int file_read(int32_t fd, void* buffer, int32_t nbytes){

    int read_count;
    int offset;
    file_des_t* cur_fd_array;
    // check if the input is valid.
    if (buffer == NULL|| nbytes == 0){
        return FAILURE;
    }

    // load the file descriptor.
    cur_fd_array = get_fd_array();
    if (cur_fd_array[fd].flag == 0){
        return FAILURE;
    }

    offset = cur_fd_array[fd].file_pos;

    // check if the current location is valid.
    if (dir_loc > boot_ptr->dir_count){
        return FAILURE;
    }
    //printf("the input offset is:%d\n",offset);
    read_count = read_data(cur_fd_array[fd].inode_num,offset,buffer,nbytes);

    if (read_count != -1){
        cur_fd_array[fd].file_pos += read_count;
    }

    return read_count;
}

/*
 *  file_write
 *      DESCRIPTION: not used in read only fs.
 *      INPUT:  None.
 *      OUTPUT: None.
 *      RETURN: SUCCESS.
 *      SIDE EFFECT: None.
 */
int file_write(int32_t fd, const void* buffer, int32_t nbytes){
    printf("TEXT EDITOR IN PROGRESS.........\n");
    return FAILURE;
}

/*
 *  get_file_length
 *      DESCRIPTION: get the length of the current file.
 *      INPUT:  dptr: dentry pointer.
 *      OUTPUT: None.
 *      RETURN: File length of the current file.
 *              FAILURE for not file being opened.
 *      SIDE EFFECT: None.
 */
int get_file_length(dentry_t* dptr){
    if (dptr == NULL){
        return FAILURE;
    }
    int inode_num = dptr->inode_num;
    inode_t* target = (inode_t*)(inode_start+inode_num);
    return target->length;
}


/*
 *  get_directory_type
 *      DESCRIPTION: get the file type from the local global storage.
 *      INPUT:  None.
 *      OUTPUT: None.
 *      RETURN: DIR type varied from 0 to 2.
 *      SIDE EFFECT: None.
 */
int get_dir_type(){
    if (location == NULL||dir_loc == -1){
        return FAILURE;
    }
    return location->type;
}

/*
 *  next directory
 *      DESCRIPTION: Move the directory forward.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: SUCCESS for push forward, FAILURE for error.
 *      SIDE EFFECT: None.
 */
int next_dir(){

    if (!read_dentry_by_index(dir_loc + 1,&temp)){
        location = &temp;
        return SUCCESS;
    } else{
        dir_loc = -1;
        location = NULL;
        return FAILURE;
    }

}

