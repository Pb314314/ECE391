#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "idt.h"
#include "keyboard.h"
#include "i8259.h"
#include "RTC.h"
#include "paging.h"
#include "Terminal.h"
#include "filesystem.h"
#include "syscall.h"
#include "tasks.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

int next;
int exception = -1;

/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
		if (!idt[i].present){
		    assertion_failure();
            return FAIL;
		}
	}
	if ((idt[KEYBOARD_VECTOR].offset_15_00 == NULL) &&
            (idt[KEYBOARD_VECTOR].offset_31_16 == NULL) ){
	    assertion_failure();
        return FAIL;
	}
	if ((idt[System_Call_Vector].offset_15_00 == NULL) &&
            (idt[System_Call_Vector].offset_31_16 == NULL)){
	    assertion_failure();
        return FAIL;
	}
	if ((idt[RTC_VECTOR].offset_15_00 == NULL) &&
	    (idt[RTC_VECTOR].offset_31_16 == NULL) ){
	    assertion_failure();
        return FAIL;
	}

	return result;
}

// add more tests here

/*
 *  exception test
 *      DESCRIPTION: test exceptions with keyboard numpad.
 *      INPUT: None.
 *      OUTPUT: System Interrupts.
 *      RETURN: None.
 *      SIDE EFFECT: GENERATE a blue screen.
 */
int exception_test() {
    next = 0;
    while(!next);
    clear();

    TEST_HEADER;

    int a;
    int val;
    int *pointer;
    exception = -1;

    printf(" USE num pad to select exception from 0 to 9.\n ");
    printf(" Current Key board Type: %d \n",KEYBOARD_TYPE);

    if (test) {
        while (exception == -1);
    }
    switch (exception) {
        case 0x00:
            val = 0;                                // create divide error.
            a = 2 / val;
            break;
        case 0x01:
            asm volatile ("int $0x01");
            break;
        case 0x02:
            asm volatile ("int $0x02");
            break;
        case 0x03:
            asm volatile ("int $0x03");
            break;
        case 0x04:
            asm volatile ("int $0x04");
            break;
        case 0x05:
            asm volatile ("int $0x05");
            break;
        case 0x06:
            asm volatile ("int $0x06");
            break;
        case 0x07:
            asm volatile ("int $0x07");
            break;
        case 0x08:
            asm volatile ("int $0x08");
            break;
        case 0x09:
            asm volatile ("int $0x09");
            break;
        case 0x0A:
            asm volatile ("int $0x0A");
            break;
        case 0x0B:
            pointer = NULL;
            a = *pointer;
            break;
        default:
            break;
    }
    return FAIL;
}

/*
 *  test_rtc
 *      DESCRIPTION: test RTC interrupt by calling dump function.
 *      INPUT: None.
 *      OUTPUT: Video memory will add one each time the interrupt hits.
 *      RETURN: None.
 *      SIDE EFFECT: modify video memory.
 */
void test_rtc(){
    next = 0;
    while(!next);
    clear();
    printf("\n");
    TEST_HEADER;
    next = 0;
    printf(" PRESS Left CTRL to start Dump Chars. Press TAB to clear and continue. \n");
    while(!next);
    clear();
    next = 0;
    printf("\n[TEST test_rtc] FINISHED. \n");
    return;
}

void cp2_rtc_test(){
    int32_t freq;
    int i;
    term_ptr->status = 0;
    while(!term_ptr->status);
    clear();
    printf("\n");
    TEST_HEADER;

    rtc_open(0);

    printf("test rtc_read, read 8 times and wait for 4 sec. \n");
    for (i=0;i<8;i++){
        rtc_read(0,0,0);
    }

    printf("now testing RTC interrupt with rtc read and write. \n");
    printf("default rate. press enter for loop test. \n");
    test = 1;

    term_ptr->status = 0;
    while(!term_ptr->status);

    for (freq = 4;freq<1025;freq*=2) {
        printf("%d HZ: ", freq);
        rtc_write(0, &freq, 4);
        for (i = 0; i < freq * 3; i++) {
            rtc_read(0, 0, 0);
        }
        printf("\n");
    }

    test = 0;
    rtc_close(0);
    printf("\n[TEST test_rtc] FINISHED. \n");
    return;
}
/*
 *  test_keyboard
 *      DESCRIPTION: Create a space for user to type.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: PASS if the typing is correct.
 *      SIDE EFFECT: the video memory may be modified if the key is being typed.
 */
int test_keyboard(){
    printf("\n");
    TEST_HEADER;

    next = 0;
    printf("Now press keys and press tab to end test. \n ");
    while(!next);
    return PASS;
}

/*
 *  test_garbage
 *      DESCRIPTION: test system call, error input and interrupt handler.
 *      INPUT: None.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: the rtc test may dump in video memory.
 */
int test_garbage(){
    TEST_HEADER;

    // First test disable irq.
    enable_irq(20);
    disable_irq(20);
    send_eoi(20);

    // test disable irq.
    printf("TEST INTERRUPT ENABLE AND DISABLE. irq 8 now disabled. Tab to enable.\n ");
    disable_irq(RTC_IRQ);
    test = 1;

    next = 0;
    while(!next);
    enable_irq(RTC_IRQ);
    next = 0;
    while (!next);
    test = 0;
    clear();

    printf("\n[TEST test_garbage] TEST SYSTEM CALL. tab to continue. \n ");
    next = 0;
    while(!next);
    // test system call.
    asm volatile ("int $0x80");
    return PASS;
}

/* paging_test
 *
 * Test if the paging settings are valid and if all assigned page could work and edge case
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging_init()
 * Files: paging.c/h
 */
int paging_test(){
    clear();
    TEST_HEADER;

    int i;           /*loop index*/

    /* page directory setting check */
    for (i = 0; i < TOTAL_SIZE; i++){
        if (i == 0 || i == 1){
            if(!page_directory[i].P)
                return FAIL;
        }
        else if(page_directory[i].P)
            return FAIL;
    }

    /* page table setting check */
    for(i = 0; i < TOTAL_SIZE; i++) {
        if(i == VIDEO_ADDR / SIZE_4KB) {
            if(!page_table[i].P)
                return FAIL;
        }
        else if(page_table[i].P)
            return FAIL;
    }

    /* R/W ability check */
    char buff;
    for(i = 0; i < SIZE_4KB; i++) {
        buff = *(char*)(VIDEO_ADDR + i);  // check read ability
        *(char*)(VIDEO_ADDR + i) = buff;  // check write ability
    }

    for(i = 0; i < TOTAL_SIZE * SIZE_4KB; i++) {
        buff = *(char*)(KERNEL_ADDR + i);  // check read ability
        *(char*)(KERNEL_ADDR + i) = buff;  // check write ability
    }

    /* check edge cases (page fault will not raise) */
    buff = *(char*)VIDEO_ADDR;   // the start of video
    buff = *(char*)KERNEL_ADDR;   // the start of kernel
    buff = *(char*)0x7FFFFF;   // the end of kernel

    // uncomment this if you want to check page fault
    /* check edge cases (page fault will raise) */
    //buff = *(char*)0x3FFFFF;   // the end of first page (page fault)
    //buff = *(char*)0x800000;   // the start of third page (page fault)
    return PASS;
}

/* Checkpoint 2 tests */

/*
 *  test_terminal
 */
void test_terminal(){
    char buffer[BUFFER_SIZE] = {'Q'};
    int i;
    int value;
    terminal_open(0);
    printf("TEST TERMINAL HERE. \n");
    printf("step1: huge output. press enter to continue, \n");
    term_ptr->status = 0;
    while(!term_ptr->status);
    terminal_write(0, buffer,BUFFER_SIZE);
    printf("TRY USING CTRL and L to clean the screen. \n");
    term_ptr->status = 0;
    while(!term_ptr->status);
    printf("TRY terminal read for 5 times. (for loop)\n");
    for (i=0;i<5;i++) {
        printf("TRIAL %d \n", i);
        memset(buffer, 0, BUFFER_SIZE);
        value = terminal_read(0, buffer, BUFFER_SIZE);
        printf("read %d chars. \n",value);
        value = terminal_write(0, buffer, value);
        printf("print %d chars. \n",value);
    }
    printf("Now test end. free to type on terminal.\n ");

    return;

}

/*
 *  simple test on file system.
 *  try to read along the file system and test several
 *  types of file and output with terminal write.
 */
int test_file(){
    TEST_HEADER;
    // simple test file read.
    int i;
    int ret = 1;
    char buffer[200];
    ret = directory_open((uint8_t*)".");
    if (ret){
        return FAIL;
    }
    // test directory reading.
    while(ret != -1){
        ret = directory_read(0,buffer,NAME_LENGTH);
        if(ret == -1){
            break;
        }
        printf((char*)buffer);
        for (i=0;i<60-ret;i++){
            printf(" ");
        }
        printf("T: %d",get_dir_type());
        printf(" S: %d\n",get_file_length(NULL));
        ret = next_dir();

        memset(buffer,0,200);
    }
    directory_close(0);

    // test text file reading.
    printf("File Name: frame0.txt\n");
    term_ptr->status = 0;
    while (!term_ptr->status);
    file_open((uint8_t*)"frame0.txt");
    ret = get_file_length(NULL);
    ret = file_read(0, buffer, ret);
    if (ret == -1){
        return FAIL;
    }
    printf("read count: %d \n",ret);
    file_close(0);
    terminal_write(0,buffer,ret);
    memset(buffer,0,200);

    // test executable reading.
    printf("File Name: grep\n");
    term_ptr->status = 0;
    while (!term_ptr->status);
    file_open((uint8_t*)"grep");
    ret = get_file_length(NULL);
    uint8_t exebuffer[ret+1];
    ret = file_read(0,exebuffer,ret);
    if (ret==-1){
        return FAIL;
    }
    printf("read count: %d\n",ret);
    file_close(0);
    terminal_write(0,exebuffer,ret);

    // test large file reading.
    printf("File Name: verylarge................\n");
    term_ptr->status = 0;
    while (!term_ptr->status);
    file_open((uint8_t*)"verylargetextwithverylongname.tx");
    ret = get_file_length(NULL);
    uint8_t largerbuffer[ret+1];
    ret = file_read(0,largerbuffer,ret-1);
    if (ret == -1){
        return FAIL;
    }
    printf("read count: %d\n",ret);
    file_close(0);
    terminal_write(0,largerbuffer,ret);

    // test impossible game.
    printf("File Name: GTA_VI.exe\n");
    term_ptr->status = 0;
    while (!term_ptr->status);
    ret = file_open((uint8_t*)"GTA_VI.exe");
    if (ret != -1){
        return FAIL;
    }
    printf("STILL NO GTA VI !!!\n");
    return PASS;
};

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */
int test_system_call(){
    TEST_HEADER;
    /*
    asm volatile ("                 \n\
            movl $3,%eax            \n\
            int  $0x80
            "
    );
    */
    __asm__("int    $0x80");
    return 0;
}

int test_syscall_open(){
    //init a PCB 
    int32_t pid = init_test_PCB();
    // open RTC file
    int32_t fd2 = open((uint8_t*)"frame0.txt");
    int32_t fd = open((uint8_t*)"rtc");
    int32_t fd3 = open((uint8_t*)".");

    pcb_t* current_pcb =  get_pcb(pid);

    
    /*------------------rtc file test ----------------------*/
    //run the file in the FD operation table;
    int32_t freq;
    int i;
    file_operation_table_t* OT1 = current_pcb->file_des_array[fd].file_op_table_ptr;
    printf("test_RTC file\n");
    test = 1;
    for (freq = 4;freq<1025;freq*=2) {
        printf("%d HZ: ", freq);
        OT1->write(fd, &freq, 4);
        for (i = 0; i < freq * 3; i++) {
            OT1->read(fd, 0, 0);
        }
        printf("\n");
    }
    test =0;
    clear();

    /*------------------regular file test ----------------------*/
    printf("test Regular file\n");
    int ret = 0;
    char buffer[200];
    file_operation_table_t* OT2 = current_pcb->file_des_array[fd2].file_op_table_ptr;
    printf("File Name: frame0.txt\n");
    
    int32_t fd4 = OT2->open((uint8_t*)"frame0.txt");
    ret = 4; //get_file_length();
    ret = OT2->read(fd4, buffer, ret);
    if (ret == -1){
        return FAIL;
    }
    printf("read count: %d \n",ret);
    OT2->close(fd2);
    terminal_write(fd4,buffer,ret);
    memset(buffer,0,200);
    
    /*------------------directory  test ----------------------*/
    printf("test directory file\n");
    file_operation_table_t* OT3 = current_pcb->file_des_array[fd3].file_op_table_ptr;
    OT3->read(0,buffer,NAME_LENGTH);
    
    return 0;
}

int test_syscall_close(){
    init_test_PCB();
    // open RTC file
    int32_t fd2 = open((uint8_t*)"frame0.txt");
    int32_t fd = open((uint8_t*)"rtc");
    close(fd2);
    close(fd);
    int32_t fd3 = open((uint8_t*)"frame1.txt");
    printf("the new FD is: %d",fd3);
    return 0;
}

int test_syscall_read_write(){
    //init a PCB 
    int32_t pid = init_test_PCB();
    // open RTC file
    int32_t fd2 = open((uint8_t*)"frame0.txt");
    int32_t fd = open((uint8_t*)"rtc");
    int32_t fd3 = open((uint8_t*)".");

    pcb_t* current_pcb =  get_pcb(pid);

    
    /*------------------rtc file test ----------------------*/
    //run the file in the FD operation table;
    int32_t freq;
    int i;
    //file_operation_table_t* OT1 = current_pcb->file_des_array[fd].file_op_table_ptr;
    printf("test_RTC file\n");
    test = 1;
    for (freq = 4;freq<1025;freq*=2) {
        printf("%d HZ: ", freq);
        write(fd, &freq, 4);
        for (i = 0; i < freq * 3; i++) {
            read(fd, 0, 0);
        }
        printf("\n");
    }
    test =0;
    clear();

    /*------------------regular file test ----------------------*/
    printf("test Regular file\n");
    int ret = 0;
    char buffer[200];
    file_operation_table_t* OT2 = current_pcb->file_des_array[fd2].file_op_table_ptr;
    printf("File Name: frame0.txt\n");
    
    OT2->open((uint8_t*)"frame1.txt");
    ret = 4;//get_file_length();
    ret = OT2->read(0, buffer, ret);
    if (ret == -1){
        return FAIL;
    }
    printf("read count: %d \n",ret);
    OT2->close(fd2);
    terminal_write(0,buffer,ret);
    memset(buffer,0,200);
    
    /*------------------directory  test ----------------------*/
    printf("test directory file\n");
    file_operation_table_t* OT3 = current_pcb->file_des_array[fd3].file_op_table_ptr;
    OT3->read(0,buffer,NAME_LENGTH);
    
    return 0;
    
}

int test_syscall_read_write_new(){
    //init a PCB 
    init_test_PCB();
    // open RTC file
    //int ret = 10;
    char buffer[200];
    int32_t fd2 = open((uint8_t*)"frame0.txt");
    int32_t fd = open((uint8_t*)"rtc");
    int32_t fd3 = open((uint8_t*)".");
    //read(fd,buffer,0);
    read(fd2,buffer,0);
    read(fd3,buffer,0);
    int32_t fre = 512;
    write(fd,&fre,4);
    write(fd2,buffer,1);
    write(fd3,buffer,1);
    close(fd2);
    close(fd);
    close(fd3);
    return 0;
    
}

/*
 *  simple_execute
 *      TEST shell....
 */



/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test())
	//TEST_OUTPUT("EXCP_test", exception_test(0))
	// launch your tests here
    //TEST_OUTPUT("paging_test", paging_test())
    //TEST_OUTPUT("file read test",test_file())
    //cp2_rtc_test();
    //test_terminal();
 //    test_system_call();

    // test_syscall_open();
    //test_syscall_close();
    //test_syscall_read_write_new();

    //test_syscall_open();
   //test_syscall_close();
   // test_syscall_read_write_new();

}

/*
 *  Control Registers for CPU
 *
 *  Writable Registers
 *
 *  CR0
 *  |31|30|29 |...|18 |16 |...|5 |4 |3 |2 |1 |0 |
 *  |PG|CD|NWE|...|AME|WPE|...|NE|ET|TS|EM|MP|PE|
 *
 *  PG: Page Enable check 1. HINT: SET CR4 before enable CR0.
 *  PE: Page Enable check 2.
 *  CD : Cache Disable.
 *
 *  CR3
 *  |31 ---------  12|11    5|4  |3  |2     0|
 *  |Page dir Address|Ignored|PCD|PWT|ignored|
 *
 *  PCD:
 *  PWT:
 *
 *  CR4
 *  |...|7  |...|4  |...|1  |0  |
 *  |...|PGE|...|PSE|...|PVI|VME|
 *
 *  PGE: Page Global Enable.
 *  PSE: Page Size Extension, 1 for 4MB.
 */

