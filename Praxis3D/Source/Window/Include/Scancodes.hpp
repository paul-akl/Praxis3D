#pragma once
//#pragma warning (disable: 4003)

#include <sdl\SDL_events.h>
#include <sdl\SDL_mouse.h>
#include <sdl\SDL_scancode.h>

#include "Common/Include/EnumFactory.hpp"

// Definition of supported scancode enums
// Setting their value to the same as SDL scancodes eliminates 
// switch statements, when determining which key was pressed.
// Would require modifying, if the input API was changed.
#define SCANCODES(Code) \
Code(Key_Invalid, = SDL_SCANCODE_UNKNOWN) \
Code(Key_Reserved1,) \
Code(Key_Reserved2,) \
Code(Key_Reserved3,) \
Code(Key_A, = SDL_SCANCODE_A) \
Code(Key_B, = SDL_SCANCODE_B) \
Code(Key_C, = SDL_SCANCODE_C) \
Code(Key_D, = SDL_SCANCODE_D) \
Code(Key_E, = SDL_SCANCODE_E) \
Code(Key_F, = SDL_SCANCODE_F) \
Code(Key_G, = SDL_SCANCODE_G) \
Code(Key_H, = SDL_SCANCODE_H) \
Code(Key_I, = SDL_SCANCODE_I) \
Code(Key_J, = SDL_SCANCODE_J) \
Code(Key_K, = SDL_SCANCODE_K) \
Code(Key_L, = SDL_SCANCODE_L) \
Code(Key_M, = SDL_SCANCODE_M) \
Code(Key_N, = SDL_SCANCODE_N) \
Code(Key_O, = SDL_SCANCODE_O) \
Code(Key_P, = SDL_SCANCODE_P) \
Code(Key_R, = SDL_SCANCODE_R) \
Code(Key_S, = SDL_SCANCODE_S) \
Code(Key_T, = SDL_SCANCODE_T) \
Code(Key_U, = SDL_SCANCODE_U) \
Code(Key_V, = SDL_SCANCODE_V) \
Code(Key_W, = SDL_SCANCODE_W) \
Code(Key_X, = SDL_SCANCODE_X) \
Code(Key_Y, = SDL_SCANCODE_Y) \
Code(Key_Z, = SDL_SCANCODE_Z) \
Code(Key_1, = SDL_SCANCODE_1) \
Code(Key_2, = SDL_SCANCODE_2) \
Code(Key_3, = SDL_SCANCODE_3) \
Code(Key_4, = SDL_SCANCODE_4) \
Code(Key_5, = SDL_SCANCODE_5) \
Code(Key_6, = SDL_SCANCODE_6) \
Code(Key_7, = SDL_SCANCODE_7) \
Code(Key_8, = SDL_SCANCODE_8) \
Code(Key_9, = SDL_SCANCODE_9) \
Code(Key_0, = SDL_SCANCODE_0) \
Code(Key_enter, = SDL_SCANCODE_RETURN) \
Code(Key_esc, = SDL_SCANCODE_ESCAPE) \
Code(Key_backspace, = SDL_SCANCODE_BACKSPACE) \
Code(Key_tab, = SDL_SCANCODE_TAB) \
Code(Key_space, = SDL_SCANCODE_SPACE) \
Code(Key_minus, = SDL_SCANCODE_MINUS) \
Code(Key_equal, = SDL_SCANCODE_EQUALS) \
Code(Key_leftbracket, = SDL_SCANCODE_LEFTBRACKET) \
Code(Key_rightbracket, = SDL_SCANCODE_RIGHTBRACKET) \
Code(Key_backslash, = SDL_SCANCODE_BACKSLASH) \
Code(Key_semicolon, = SDL_SCANCODE_SEMICOLON) \
Code(Key_apostrophe, = SDL_SCANCODE_APOSTROPHE) \
Code(Key_tilde, = SDL_SCANCODE_GRAVE) \
Code(Key_comma, = SDL_SCANCODE_COMMA) \
Code(Key_period, = SDL_SCANCODE_PERIOD) \
Code(Key_slash, = SDL_SCANCODE_SLASH) \
Code(Key_capslock, = SDL_SCANCODE_CAPSLOCK) \
Code(Key_F1, = SDL_SCANCODE_F1) \
Code(Key_F2, = SDL_SCANCODE_F2) \
Code(Key_F3, = SDL_SCANCODE_F3) \
Code(Key_F4, = SDL_SCANCODE_F4) \
Code(Key_F5, = SDL_SCANCODE_F5) \
Code(Key_F6, = SDL_SCANCODE_F6) \
Code(Key_F7, = SDL_SCANCODE_F7) \
Code(Key_F8, = SDL_SCANCODE_F8) \
Code(Key_F9, = SDL_SCANCODE_F9) \
Code(Key_F10, = SDL_SCANCODE_F10) \
Code(Key_F11, = SDL_SCANCODE_F11) \
Code(Key_F12, = SDL_SCANCODE_F12) \
Code(Key_printscr, = SDL_SCANCODE_PRINTSCREEN) \
Code(Key_scrolllock, = SDL_SCANCODE_SCROLLLOCK) \
Code(Key_pause, = SDL_SCANCODE_PAUSE) \
Code(Key_insert, = SDL_SCANCODE_INSERT) \
Code(Key_home, = SDL_SCANCODE_HOME) \
Code(Key_pageup, = SDL_SCANCODE_PAGEUP) \
Code(Key_delete, = SDL_SCANCODE_DELETE) \
Code(Key_end, = SDL_SCANCODE_END) \
Code(Key_pagedown, = SDL_SCANCODE_PAGEDOWN) \
Code(Key_right, = SDL_SCANCODE_RIGHT) \
Code(Key_left, = SDL_SCANCODE_LEFT) \
Code(Key_down, = SDL_SCANCODE_DOWN) \
Code(Key_up, = SDL_SCANCODE_UP) \
Code(Key_numlock, = SDL_SCANCODE_NUMLOCKCLEAR) \
Code(Key_keypad_divide, = SDL_SCANCODE_KP_DIVIDE) \
Code(Key_keypad_multiply, = SDL_SCANCODE_KP_MULTIPLY) \
Code(Key_keypad_minus, = SDL_SCANCODE_KP_MINUS) \
Code(Key_keypad_plus, = SDL_SCANCODE_KP_PLUS) \
Code(Key_keypad_enter, = SDL_SCANCODE_KP_ENTER) \
Code(Key_keypad_1, = SDL_SCANCODE_KP_1) \
Code(Key_keypad_2, = SDL_SCANCODE_KP_2) \
Code(Key_keypad_3, = SDL_SCANCODE_KP_3) \
Code(Key_keypad_4, = SDL_SCANCODE_KP_4) \
Code(Key_keypad_5, = SDL_SCANCODE_KP_5) \
Code(Key_keypad_6, = SDL_SCANCODE_KP_6) \
Code(Key_keypad_7, = SDL_SCANCODE_KP_7) \
Code(Key_keypad_8, = SDL_SCANCODE_KP_8) \
Code(Key_keypad_9, = SDL_SCANCODE_KP_9) \
Code(Key_keypad_0, = SDL_SCANCODE_KP_0) \
Code(Key_keypad_period, = SDL_SCANCODE_KP_PERIOD) \
Code(Key_leftctrl, = SDL_SCANCODE_LCTRL) \
Code(Key_leftshift, = SDL_SCANCODE_LSHIFT) \
Code(Key_leftalt, = SDL_SCANCODE_LALT) \
Code(Key_leftwindows, = SDL_SCANCODE_LGUI) \
Code(Key_rightctrl, = SDL_SCANCODE_RCTRL) \
Code(Key_rightshift, = SDL_SCANCODE_RSHIFT) \
Code(Key_rightalt, = SDL_SCANCODE_RALT) \
Code(Key_rightwindows, = SDL_SCANCODE_RGUI) \
Code(MouseKeysOffset,) \
Code(Mouse_left, = MouseKeysOffset + SDL_BUTTON_LEFT) \
Code(Mouse_middle, = MouseKeysOffset + SDL_BUTTON_MIDDLE) \
Code(Mouse_right, = MouseKeysOffset + SDL_BUTTON_RIGHT) \
Code(Mouse_x1, = MouseKeysOffset + SDL_BUTTON_X1) \
Code(Mouse_x2, = MouseKeysOffset + SDL_BUTTON_X2) \
Code(Mouse_wheelup,) \
Code(Mouse_wheeldown,) \
Code(Mouse_wheelleft,) \
Code(Mouse_wheelright,) \
Code(NumberOfScancodes, )
DECLARE_ENUM(Scancode, SCANCODES)
