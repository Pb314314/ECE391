//
// Created by Jordan(qishenz2@illinois.edu) on 2022/3/20.
//

#ifndef MP3_KEYBOARD_H
#define MP3_KEYBOARD_H
#include "types.h"
/*
 *  KEY_BOARD_TYPE
 *  When testing on several machines, we find that the full keyboard
 *  returns different keycode in some condition.
 *
 *  0: Atari KEYBOARD.
 *  1: ANSI104 KEYBOARD.
 *  2. MY mobility keyboard. // not activated.
 */
#define KEYBOARD_TYPE           1

#define KB_IRQ                  1
#define KB_COMMAND              0x64
#define KB_DATA                 0x60

// KEY RELEASE SCANCODE.
#define KEY_LSHIFT_RELEASE          0xAA
#define KEY_RSHIFT_RELEASE          0xB6
#define KEY_ALT_RELEASE             0xB8
#define KEY_CAPSLOCK_RELEASE        0xBA
#define KEY_CTRL_RELEASE            0x9D


extern int test;

/* enable the interrupt for keyboard. */
void key_board_init();

/* Handle Key board interrupt. */
void key_board_handler();

/* read data from the key board buffer. */
int key_print(int key_ascii);

/* helper function to handle keypad. */
 uint8_t handle_keypad(int keycode);

/*
 *  Key Board Scancode
 *  Based on  atakbd.c, which is
 *  Copyright (c) 2005 Michael Schmitz
 *  Referred from kernel version v5.17
 */
#if (KEYBOARD_TYPE==0)
enum keyboard{
    KEY_EMPTY,      //0
    KEY_ESC,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,          // 10
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,          // 20
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LEFTBRACE,
    KEY_RIGHTBRACE,
    KEY_ENTER,
    KEY_LEFTCTRL,
    KEY_A,          // 30
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_APOSTROPHE, // 40
    KEY_GRAVE,
    KEY_LEFTSHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,          // 50
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_RIGHTSHIFT,
    KEY_MULTIPL,
    KEY_LEFTALT,
    KEY_SPACE,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,         // 60
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_HOME,
    KEY_UP,         // 70
    KEY_KPMINUS,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_KPPLUS,
    KEY_DOWN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_102ND,
    KEY_UNDO,
    KEY_HELP,       // 80
    KEY_KPLEFTPAREN,
    KEY_KPRIGHTPAREN,
    KEY_KPSLASH,
    KEY_KPASTERISK_1,
    KEY_KP7,
    KEY_KP8,
    KEY_KP9,
    KEY_KP4,
    KEY_KP5,
    KEY_KP6,        // 90
    KEY_KP1,
    KEY_KP2,
    KEY_KP3,
    KEY_KP0,
    KEY_KPDOT,
    KEY_KPENTER,
}keys;

#elif (KEYBOARD_TYPE == 1)
enum keyboard{
    KEY_EMPTY,      //0
    KEY_ESC,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,          // 10
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,          // 20
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LEFTBRACE,
    KEY_RIGHTBRACE,
    KEY_ENTER,
    KEY_LEFTCTRL,
    KEY_A,          // 30
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_APOSTROPHE, // 40
    KEY_GRAVE,
    KEY_LEFTSHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,          // 50
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_RIGHTSHIFT,
    KEY_MULTIPL,
    KEY_LEFTALT,
    KEY_SPACE,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,         // 60
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_NUMLOCK,
    KEY_SCROLL,         // 70
    KEY_HOME,
    KEY_UP,
    KEY_PRIOR,
    KEY_KPMINUS,
    KEY_LEFT,
    KEY_CLEAR,
    KEY_RIGHT,
    KEY_KPADD,
    KEY_END,
    KEY_DOWN,       // 80
    KEY_NEXT,
    KEY_INSERT,
    KEY_DELETE,
    KEY_SNAPSH,
    PL1,
    PL2,
    PL3,
    PL4,
    PL5,
    PL6,        // 90
    KEY_LWIN,
    KEY_RWIN,       // use this place to add numpad.
    KEY_KP7,
    KEY_KP8,
    KEY_KP9,
    PL7,
    KEY_KP4,
    KEY_KP5,
    KEY_KP6,
    PL8,
    KEY_KP1,
    KEY_KP2,
    KEY_KP3,
    KEY_KP0,
    KEY_KPDOT
}keys;
#endif


#endif //MP3_KEYBOARD_H
