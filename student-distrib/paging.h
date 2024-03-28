#ifndef _paging_h_
#define _paging_h_

#ifndef ASM
#include "types.h"

#define TOTAL_SIZE      1024         // both directory and table have 1024 entries 
#define VIDEO_ADDR      0xB8000         // from lib.c, addr of video memory
#define SIZE_4KB        4096
#define SIZE_4MB        0x400000
#define KERNEL_ADDR     0x400000
#define USER_ADDR       0x08000000
#define KERNEL_LOC      1


void tlb_flush();

/* Structure for page dict and page table */
/* page table only used for 4KB Page */
typedef struct __attribute__((packed)) directory_entry {
    uint32_t P                  :1; /* present */   
    uint32_t RW                 :1; /* read/write */
    uint32_t US                 :1; /* user/supervisor */
    uint32_t PWT                :1; /* write-through */
    uint32_t PCD                :1; /* cache disabled */
    uint32_t A                  :1; /* accessed */
    /* D_diff:   4MB page - dirty 
                 4KB page - set to 0 */
    uint32_t D_diff             :1;
    uint32_t PS                 :1; /* Page size:  4MB page, 1 \ 4KB page, 0 */
    uint32_t G                  :1; /* Global page */
    uint32_t Avail              :3; /* available for system program's use */

    /* 4MB page: bit 31-22, addr; bit21-13, reserved; bit12, page table attr index */
    /* 4KB page: bit 31-12, addr */
    //uint32_t bit12              :1;
    //uint32_t bit21_13           :9; 
    //uint32_t bit31_22           :10;
    uint32_t Page_addr          :20;
} directory_entry_t;


/* Combined PDT struct for both 4KB and 4MB cases */
typedef struct __attribute__((packed)) table_entry {
    /* Common filed on bits 5:0 */
    uint32_t P                  :1; /* present */   
    uint32_t RW                 :1; /* read/write */
    uint32_t US                 :1; /* user/supervisor */
    uint32_t PWT                :1; /* write-through */
    uint32_t PCD                :1; /* cache disabled */
    uint32_t A                  :1; /* accessed */
    uint32_t D                  :1;
    uint32_t PAT                :1; 
    uint32_t G                  :1; /* Global page */
    uint8_t  Avail              :3; /* available for system program's use */
    uint32_t Page_addr          :20;

} table_entry_t;


directory_entry_t page_directory[TOTAL_SIZE] __attribute__((aligned (SIZE_4KB)));    /* Page Dict */
table_entry_t page_table[TOTAL_SIZE] __attribute__((aligned (SIZE_4KB)));          /* Page Table for first chunk */
table_entry_t page_table_vidmap[TOTAL_SIZE] __attribute__((aligned (SIZE_4KB)));

/* function prototype */
extern void paging_init(void);
//extern void paging_set_user_mapping(int32_t pid);
//extern void paging_set_for_vedio_mem(int32_t virtual_addr_for_vedio, int32_t phys_addr_for_vedio);
//extern void paging_restore_for_vedio_mem(int32_t virtual_addr_for_vedio);

#endif
#endif
