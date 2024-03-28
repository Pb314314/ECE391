//
// Created by jordan on 2022/3/27.
//

#ifndef MP3_FILESYSTEM_H
#define MP3_FILESYSTEM_H

#include "types.h"
#include "lib.h"

// Constant define by the definition of SSE FS.
#define BOOT_RESERVE    52
#define DENTRY_RESERVE  24
#define INODES_BLOCK    1023
#define NAME_LENGTH     32
#define BLOCK_SIZE      4096


// SSE FS structs.
typedef struct dentry{
    int8_t     filename[NAME_LENGTH];
    int32_t    type;
    int32_t    inode_num;
    char        fill[DENTRY_RESERVE];
}dentry_t;
// Each regular file is described by an index node. 
typedef struct inodes{
    int32_t    length;      // file size in byte
    int32_t    blocks_num[INODES_BLOCK];    // block_nums[i]: i th data_block_id 
}inode_t;

typedef struct boot{
    int32_t    dir_count;
    int32_t    inode_count;
    int32_t    block_count;
    char        fillout[BOOT_RESERVE];
    dentry_t    dentries[63];
}boot_t;

// FIXME:why should we define data block struct?

typedef struct block{
    uint8_t      data[BLOCK_SIZE];
}block_t;

/* SSE FS manage function */
void filesystem_init(void* start_addr);

/* read the directory entry functions. */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t idx, dentry_t* dentry);

/* read data block function. */
int32_t read_data(uint32_t inode_idx, uint32_t offset, uint8_t* buf, uint32_t nbytes);

/* directory system call function. */
int directory_open(const uint8_t* filename);
int directory_close(int32_t fd);
int directory_read(int32_t fd, void* buf, int32_t nbytes);
int directory_write(int32_t fd, const void* buf, int32_t nbytes);

/* file system call function. */
int file_open(const uint8_t* filename);
int file_close(int32_t fd);
int file_read(int32_t fd, void* buffer, int32_t nbytes);
int file_write(int32_t fd, const void* buffer, int32_t nbytes);

/* helper functions. */
int get_file_length(dentry_t* dptr);
int get_dir_type();
int next_dir();

#endif //MP3_FILESYSTEM_H
