//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/20.
//

#include "keyboard.h"
#include "i8259.h"
#include "types.h"
#include "lib.h"
#include "tests.h"
#include "Terminal.h"

int test;
void set_buffer(terminal_t* ptr, uint8_t value);
void handle_function_key(int keycode);

// key board buffer
char* kb_buffer;

// SPECIAL KEYs
volatile int numlock =   0;
volatile int capslock =  0;
volatile int shiftlock = 0;
volatile int ctrl =      0;
volatile int alt  =      0;

// scan code list.

/*
 *  Key Board Scancode
 *  Based on  atakbd.c, which is
 *  Copyright (c) 2005 Michael Schmitz
 *  Referred from kernel version v5.17
 */
unsigned char scancode_list[58][2] =
        {
            [KEY_EMPTY]={   0,0   } ,
            [KEY_ESC]={   0,0   } ,
            [KEY_1]={ '1','!' } ,
            [KEY_2]={ '2','@' } ,
            [KEY_3]={ '3','#' } ,
            [KEY_4]={ '4','$' } ,
            [KEY_5]={ '5','%' } ,
            [KEY_6]={ '6','^' } ,
            [KEY_7]={ '7','&' } ,
            [KEY_8]={ '8','*' } ,
            [KEY_9]={ '9','(' } ,
            [KEY_0]={ '0',')' } ,
            [KEY_MINUS]={ '-','_' } ,
            [KEY_EQUAL]={ '=','+' } ,
            [KEY_BACKSPACE]={   8,8   } ,        //Backspace
            [KEY_TAB]={   '\t','\t'   } ,
            [KEY_Q]={ 'q','Q' } ,
            [KEY_W]={ 'w','W' } ,
            [KEY_E]={ 'e','E' } ,
            [KEY_R]={ 'r','R' } ,
            [KEY_T]={ 't','T' } ,
            [KEY_Y]={ 'y','Y' } ,
            [KEY_U]={ 'u','U' } ,
            [KEY_I]={ 'i','I' } ,
            [KEY_O]={ 'o','O' } ,
            [KEY_P]={ 'p','P' } ,
            [KEY_LEFTBRACE]={ '[','{' } ,
            [KEY_RIGHTBRACE]={ ']','}' } ,
            [KEY_ENTER]={'\n','\n'} ,
            [KEY_LEFTCTRL]={   0,0   } ,
            [KEY_A]={ 'a','A' } ,
            [KEY_S]={ 's','S' } ,
            [KEY_D]={ 'd','D' } ,
            [KEY_F]={ 'f','F' } ,
            [KEY_G]={ 'g','G' } ,
            [KEY_H]={ 'h','H' } ,
            [KEY_J]={ 'j','J' } ,
            [KEY_K]={ 'k','K' } ,
            [KEY_L]={ 'l','L' } ,
            [KEY_SEMICOLON]={ ';',':' } ,
            [KEY_APOSTROPHE]={  39, 34 } ,
            [KEY_GRAVE]={ '`','~' } ,
            [KEY_LEFTSHIFT]={   0,0   } ,
            [KEY_BACKSLASH]={ '\\','|'} ,
            [KEY_Z]={ 'z','Z' } ,
            [KEY_X]={ 'x','X' } ,
            [KEY_C]={ 'c','C' } ,
            [KEY_V]={ 'v','V' } ,
            [KEY_B]={ 'b','B' } ,
            [KEY_N]={ 'n','N' } ,
            [KEY_M]={ 'm','M' } ,
            [KEY_COMMA]={ ',','<' } ,
            [KEY_DOT]={ '.','>' } ,
            [KEY_SLASH]={ '/','?' } ,
            [KEY_RIGHTSHIFT]={   0,0   } ,
            [KEY_MULTIPL]={   0,0   } ,
            [KEY_LEFTALT]={   0,0   } ,
            [KEY_SPACE]={ ' ',' ' } ,
        };

// handle special keys here.


/*
 *  key_board_init
 *      DESCRIPTION: Enable the interrupt on keyboard.
 *      INPUT/OUTPUT/RETURN: None.
 *      SIDE EFFECT: the PIC status modified.
 */
void key_board_init(){
    enable_irq(KB_IRQ);
}

//use by linkage when keyboard interrupt occur
/*
 *  key_board_handler
 *      DESCRIPTION: do the keyboard handler
 *      INPUT/OUTPUT/RETURN: None.
 *      SIDE EFFECT: do the key press check
 */
void key_board_handler(){
    unsigned int scancode = 0;
    int i;
    uint8_t     key_trans;
    uint8_t     numpad;
    // Quit irq to let other interrupt on.
    send_eoi(KB_IRQ);

    // init key buffer.
    terminal_t* terminal= term_ptr;
    kb_buffer = terminal->input_buffer;

    // read scan code
    while(!inb(KB_DATA));
    scancode = inb(KB_DATA);
    // handle F1 to F3.
    handle_function_key(scancode);
    // handle special scancode.
    switch (scancode) {
        case KEY_CAPSLOCK:
            capslock = ~capslock;
            break;
        case KEY_LEFTSHIFT:
            if (!shiftlock){
                shiftlock = 1;
            }
            break;
        case KEY_RIGHTSHIFT:
            if (!shiftlock){
                shiftlock = 1;
            }
            break;
        case KEY_LEFTALT:
            alt = 1;
            break;
        case KEY_LEFTCTRL:
            ctrl = 1;
            break;
        case KEY_CTRL_RELEASE:
            ctrl = 0;
            break;
        case KEY_ALT_RELEASE:
            alt = 0;
            break;
        case KEY_LSHIFT_RELEASE:
            if (shiftlock){
                shiftlock = 0;
            }
            break;
        case KEY_RSHIFT_RELEASE:
            if (shiftlock){
                shiftlock = 0;
            }
            break;
        case KEY_BACKSPACE:
            if (terminal->isem == 0){                               // empty terminal buffer...
                break;
            }
            if (terminal->isem ==128){
                putc('\b');
            }
            else if (terminal->input_buffer[terminal->isem - 1] == '\t'){
                putc('\t');
            } else{
                putc('\b');
            }
            terminal->input_buffer[terminal->isem - 1] = 0;
            terminal->isem--;
            break;
        case KEY_NUMLOCK:
            numlock = ~numlock;
            printf("\nNUMLOCK STAT: %d\n", (0-numlock));
            break;
        case KEY_TAB:
            for (i=0;i<4;i++){
                putc(' ');
            }
            set_buffer(terminal, '\t');
            break;
        case KEY_ENTER:
            putc('\n');
            set_buffer(terminal,'\n');
            terminal->status = 1;// only the enter is pressed, the status is 1.
            break;
        case KEY_L:
            if (ctrl) {
                clear();
                break;
            }
        default:
            // we only add look up table until space.
            if (scancode > KEY_SPACE){
                // numpad may be found here.
                numpad = handle_keypad(scancode);
                if (numpad){
                    key_print(numpad);
                    set_buffer(terminal,numpad);
                }
                break;
            }
            if ((scancode >= KEY_Q && scancode <= KEY_P)||(scancode >= KEY_A && scancode <= KEY_L) ||
                    (scancode >= KEY_Z && scancode <= KEY_M)){
                // handle words use XOR gate.
                if (!capslock != !shiftlock){                   // since the words should consider differ from the symbols.
                    key_trans = scancode_list[scancode][1];
                } else{
                    key_trans = scancode_list[scancode][0];
                }
            } else{
                // handle symbols.
                if (shiftlock){
                    key_trans = scancode_list[scancode][1];
                } else{
                    key_trans = scancode_list[scancode][0];
                }
            }

            key_print(key_trans);
            set_buffer(terminal, key_trans);
            break;
    }
    moving_cursor(screen_x,screen_y);
    return;
}


/*
 *  key_print
 *      DESCRIPTION: PRINT KEY ASCII using printf given in the kernel lib.
 *      INPUT: scancode: key scancode. see keyboard.h for more info.
 *      OUTPUT: None.
 *      RETURN: 0 for success, 1 for out of range.
 *      SIDE EFFECT: Change video buffer.
 */
int key_print(int key_ascii){
    // no need to handle caps and shift in CP.
    putc(key_ascii);
    return 0;
}

/*
 *  set_buffer
 *      DESCRIPTION: set the keyboard buffer with the given value and
 *                   handle overflow in the buffer.
 *      INPUT:      ptr: pointer to the terminal itself.
 *                  value: the char value to be put in the buffer.
 *      OUTPUT:     None.
 *      RETURN:     None.
 *      SIDE EFFECT: None.
 */
void set_buffer(terminal_t* ptr, uint8_t value){
    terminal_t* terminal = ptr;

    if (terminal->isem <(BUFFER_SIZE-1)){
        terminal->input_buffer[terminal->isem]=value;
        terminal->isem++;
    }
    else{
        terminal->input_buffer[BUFFER_SIZE-1] = '\n';
        terminal->isem = BUFFER_SIZE;
    }
    return;
}

/*
 *  handle_keypad
 *      DESCRIPTION: TRY to handle keypad and use it to run functions.
 *      INPUT: keycode UNKNOWN scancode.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: modify some global variable related to test.
 */
uint8_t handle_keypad(int keycode){
    // trial.
    uint8_t num;
    if (numlock){
        // the num pad is enable.
        keycode += 22;
    }else{
        return 0;
    }

    switch (keycode) {
        case KEY_KP0:
            num = '0';
            break;
        case KEY_KP9:
            num = '9';
            break;
        case KEY_KP8:
            num = '8';
            break;
        case KEY_KP7:
            num = '7';
            break;
        case KEY_KP6:
            num = '6';
            break;
        case KEY_KP5:
            num = '5';
            break;
        case KEY_KP4:
            num = '4';
            break;
        case KEY_KP3:
            num = '3';
            break;
        case KEY_KP2:
            num = '2';
            break;
        case KEY_KP1:
            num = '1';
            break;
        case KEY_KPDOT:
            num = '.';
            break;
        default:
            num = 0;
            break;
    }
    if (test){
        exception = (int)num;
    }
    return num;
}

/*
 *  handle_function_key
 *      DESCRIPTION: switch terminals when ALT + Fn is pressed. in the GUI we should press ctrl, alt and Fn.
 *      INPUT: keycode.
 *      OUTPUT: None.
 *      RETURN: None.
 *      SIDE EFFECT: the displaying terminal will switch.
 */
void handle_function_key(int keycode){
    if (alt == 0){
        return;
    }
    switch (keycode) {
        case KEY_F1:
            terminal_switch(0);
            break;
        case KEY_F2:
            terminal_switch(1);
            break;
        case KEY_F3:
            terminal_switch(2);
            break;
        default:
            break;
    }
    return;
}





