#include "paging.h"


/* paging_init
 *  Description: Initialize the paging directory and paging table and mapping the video memory
 *  Input:  none    
 *  Output: none
 *  Side Effect: set the hardware ready for pagings
 */
void paging_init(){
    int i;     /*index*/

    /* Initialize the page directory */
    for (i = 0; i < TOTAL_SIZE; i++){
        if(i==0){//0-4MB, video memory
            page_directory[i].P = 1;         /* Present */
            page_directory[i].RW = 1;        /* RW enable */
            page_directory[i].US = 0;        /* For kernel */
            page_directory[i].PWT = 0;       /* Always write back policy */
            page_directory[i].PCD = 0;       /* 1 for code and data */
            page_directory[i].A = 0;         /* Set to 1 by processor */

            page_directory[i].D_diff = 0;    /* 4KB */
            page_directory[i].PS = 0;        /* 4KB */
            page_directory[i].G = 0;         /* Ignored for 4KB page*/
            page_directory[i].Avail = 0;     /* Not used */
            
            /* setting address */
            page_directory[i].Page_addr = ((int)page_table)/SIZE_4KB;     
        }
        else if(i==KERNEL_LOC){//4-8MB kernel memory
            page_directory[i].P = 1;         /* Present */
            page_directory[i].RW = 1;        /* RW enable */
            page_directory[i].US = 0;        /* For kernel */
            page_directory[i].PWT = 0;       /* Always write back policy */
            page_directory[i].PCD = 0;       /* 1 for code and data */
            page_directory[i].A = 0;         /* Set to 1 by processor */

            page_directory[i].D_diff = 0;    /* Set to 0 as Dirty for 4MB */
            page_directory[i].PS = 1;        /* 4MB */
            page_directory[i].G = 1;         /* Only for the kernel page */
            page_directory[i].Avail = 0;     /* Not used */
            page_directory[i].Page_addr = KERNEL_ADDR/SIZE_4KB;     
        }
        else{  //8-4GB program , not exit yet
            page_directory[i].P = 0;         /* Not present */
            page_directory[i].RW = 1;        /* RW enable */
            page_directory[i].US = 0;        /* For kernel */
            page_directory[i].PWT = 0;       /* Always write back policy */
            page_directory[i].PCD = 1;       /* 1 for code and data */
            page_directory[i].A = 0;         /* Set to 1 by processor */

            page_directory[i].D_diff = 0;    /* Set to 0 as Dirty for 4MB */
            page_directory[i].PS = 0;        /* 4MB */
            page_directory[i].G = 0;         
            page_directory[i].Avail = 0;     /* Not used */
            page_directory[i].Page_addr = 0;    
        }
    }
    
    /* Initialize the page table for 0-4 MB as well as the Video Memory 4KB page */
    for (i = 0; i < TOTAL_SIZE; i++){

        if (i == (VIDEO_ADDR/SIZE_4KB)){                //the VM is the 184 paging in the first 4MB(184 * 4KB = 0xB8000)
            page_table[i].P = 1;                        /*set video momory page to 1*/
        }
        else{
            page_table[i].P = 0;
        }
        page_table[i].RW = 1;                           /* Read/Write enable */                           
        page_table[i].US = 0;                           /* Kernel */
        page_table[i].PWT = 0;
        page_table[i].PCD = 0;
        page_table[i].A = 0;
        page_table[i].D = 0;                            /* Set by processor */
        page_table[i].PAT = 0;                          /* Not used */
        page_table[i].G = 0;                            /* Kernel */
        page_table[i].Avail = 0;                        /* Not used */
        page_table[i].Page_addr = i;                    /* Physical Address MSB 20bits */
    }


    /* for vidmap page init */
    for(i = 0; i < TOTAL_SIZE; i++){
        page_table_vidmap[i].P = 0;
        page_table_vidmap[i].RW = 1;                           /* Read/Write enable */                           
        page_table_vidmap[i].US = 0;                           /* Kernel */
        page_table_vidmap[i].PWT = 0;
        page_table_vidmap[i].PCD = 0;
        page_table_vidmap[i].A = 0;
        page_table_vidmap[i].D = 0;                            /* Set by processor */
        page_table_vidmap[i].PAT = 0;                          /* Not used */
        page_table_vidmap[i].G = 0;                            /* Kernel */
        page_table_vidmap[i].Avail = 0;                        /* Not used */
        page_table_vidmap[i].Page_addr = i;                    /* Physical Address MSB 20bits */
    }


    /* Enable paging mode in hardware */
    asm volatile (
        "movl $page_directory, %%eax   /* Load paging directory */      ;"
        "movl %%eax, %%cr3                                              ;"

        "movl %%cr4, %%eax             /* Enable PSE */                 ;"
        "orl  $0x00000010, %%eax                                        ;"
        "movl %%eax, %%cr4                                              ;"

        "movl %%cr0, %%eax             /* Set paging bit */             ;"
        "orl  $0x80000000, %%eax                                        ;"
        "movl %%eax, %%cr0                                              ;"
        : /* no output */
        : /* no input */
        : "eax"
    );
}

/*
 *  tlb_flush
 *      DESCRIPTION: Flush the TBS when necessary.
 *      INPUT/OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: The TLBs without global flag will bu modified.
 */
void tlb_flush() {
    asm volatile(
    "movl %%cr3,%%eax     ;" /* Flush the TLB through read and write back the CR3 reg. */
    "movl %%eax,%%cr3     ;"
    : /* No output. */
    : /* No input*/
    : "eax", "cc"
    );
}

/*  not used yet
 *  set_vm_page
 *      DESCRIPTION: set a paging given the index.
 *      INPUT/OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: enable a paging by setting the page table(0-4MB....).
 */
void set_vm_page(int index){
    int entry = index/SIZE_4KB;
    page_table[entry].P = 1;

}

