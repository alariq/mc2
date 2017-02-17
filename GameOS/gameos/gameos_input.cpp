#include "gameos.hpp"

#include <SDL2/SDL.h> // can avoid inluding this here if move MouseInfo struct to separate file
#include "gos_input.h"
#include "utils/vec.h"

#include<string.h>

extern input::MouseInfo g_mouse_info;
extern input::KeyboardInfo g_keyboard_info;

//
// The mouse position and buttons are read once a frame.
//
// Position returns values converted into position on window. Range is 0.0 to 1.0  (0 to 639 on a 640 wide screen)
// When running in a window this may not match the sum of the deltas, as the windows mouse cursor can have acceleration
//
//
// Deltas returns values as a percentage of screen size. Range is -1.0 to 1.0
//
// ie: 0.5 means half a screen to the right or down
//
// The wheel position is only available as a delta value, Range is -1.0 to 1.0 (999.0 means no wheel)
//
//
// ButtonsPressed are bitflags, 0=Left Button, 2=Right, 1=Middle, 3=X1, 4=X2 (X1 and X2 are on 5 button mice)
//
// If you want to make sure you do not miss mouse clicks, check for the mouse buttons with
//
// You may pass zero as any parameter you do not wish to query
//
////////////////////////////////////////////////////////////////////////////////
void __stdcall gos_GetMouseInfo( float* pXPosition, float* pYPosition, int* pXDelta, int* pYDelta, int* pWheelDelta, DWORD* pButtonsPressed )
{
    //const float w = (float)Environment.screenWidth;
    //const float h = (float)Environment.screenHeight;

    const float w = (float)Environment.drawableWidth;
    const float h = (float)Environment.drawableHeight;

    if(pXPosition)
        *pXPosition = g_mouse_info.x_ / w;
    if(pYPosition)
        *pYPosition = g_mouse_info.y_ / h;

	// as percentage of screen size
    
    // sebi: I do not like this percentage style and it does not work good, so
    /*
    if(pXDelta)
        *pXDelta = (int)(100.0f * clamp(g_mouse_info.rel_x_ / w, -1.0f, 1.0f));
    if(pYDelta)
        *pYDelta = (int)(100.0f * clamp(g_mouse_info.rel_y_ / h, -1.0f, 1.0f));
        */
    if(pXDelta)
        *pXDelta = (int)(clamp(g_mouse_info.rel_x_, -w, w));
    if(pYDelta)
        *pYDelta = (int)(clamp(g_mouse_info.rel_y_, -h, h));

    //if(pWheelDelta)
    //  *pWheelDelta = (int)(100.0f * (g_mouse_info.wheel_vert_ / h));
    if(pWheelDelta)
        *pWheelDelta = (int)(g_mouse_info.wheel_vert_);

    if(pButtonsPressed) {
        DWORD bs = 0;
        for(int i=0; i<input::MouseInfo::NUM_BUTTONS;++i)
        {
            bool down = g_mouse_info.button_state_[i] == input::KS_PRESSED;
            down |= g_mouse_info.button_state_[i] == input::KS_HELD;
            bs |= down ? (1<<i) : 0;
        }
        *pButtonsPressed = bs;
    }
}

////////////////////////////////////////////////////////////////////////////////
extern SDL_Window* g_sdl_window;
void __stdcall gos_SetMousePosition( float XPosition, float YPosition )
{
    if(g_sdl_window) {
        SDL_WarpMouseInWindow(g_sdl_window, (int)XPosition, (int)YPosition);
    }
}

// loks like KEY_* enum contains everything, from scancodes to localized keys, e.g. it has GRAVE and APOSTROPHE but those are usually on the same physical button and valuse depends on localization
// let's keep it like this for now and see later
gosEnum_KeyIndex remap_sdl_to_gos(const SDL_Scancode scancode) {

    if(scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
        return (gosEnum_KeyIndex)(scancode - SDL_SCANCODE_1 + KEY_1);
    if(scancode == SDL_SCANCODE_0) return KEY_0;

    if(scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
        return (gosEnum_KeyIndex)(scancode - SDL_SCANCODE_A + KEY_A);

    switch(scancode) {
        case SDL_SCANCODE_ESCAPE: return KEY_ESCAPE;
        case SDL_SCANCODE_MINUS: return KEY_MINUS;
        case SDL_SCANCODE_EQUALS: return KEY_EQUALS;
        case SDL_SCANCODE_BACKSPACE: return KEY_BACK;
        case SDL_SCANCODE_TAB: return KEY_TAB;
        case SDL_SCANCODE_LEFTBRACKET: return KEY_LBRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return KEY_RBRACKET;
        case SDL_SCANCODE_RETURN: return KEY_RETURN;
        case SDL_SCANCODE_SEMICOLON: return KEY_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE: return KEY_APOSTROPHE;
        case SDL_SCANCODE_GRAVE: return KEY_GRAVE;
        case SDL_SCANCODE_BACKSLASH: return KEY_BACKSLASH;
        case SDL_SCANCODE_COMMA: return KEY_COMMA;
        case SDL_SCANCODE_PERIOD: return KEY_PERIOD;
        case SDL_SCANCODE_SLASH: return KEY_SLASH;
        case SDL_SCANCODE_KP_MULTIPLY: return KEY_MULTIPLY;
        case SDL_SCANCODE_MENU: return KEY_LMENU;
        case SDL_SCANCODE_SPACE: return KEY_SPACE;
        case SDL_SCANCODE_CAPSLOCK: return KEY_CAPITAL;
        case SDL_SCANCODE_F1: return KEY_F1;
        case SDL_SCANCODE_F2: return KEY_F2;
        case SDL_SCANCODE_F3: return KEY_F3;
        case SDL_SCANCODE_F4: return KEY_F4;
        case SDL_SCANCODE_F5: return KEY_F5;
        case SDL_SCANCODE_F6: return KEY_F6;
        case SDL_SCANCODE_F7: return KEY_F7;
        case SDL_SCANCODE_F8: return KEY_F8;
        case SDL_SCANCODE_F9: return KEY_F9;
        case SDL_SCANCODE_F10: return KEY_F10;
        case SDL_SCANCODE_F11: return KEY_F11;
        case SDL_SCANCODE_F12: return KEY_F12;
        case SDL_SCANCODE_F13: return KEY_F13;
        case SDL_SCANCODE_F14: return KEY_F14;
        case SDL_SCANCODE_F15: return KEY_F15;
        case SDL_SCANCODE_KP_0: return KEY_NUMPAD0;
        case SDL_SCANCODE_KP_1: return KEY_NUMPAD1;
        case SDL_SCANCODE_KP_2: return KEY_NUMPAD2;
        case SDL_SCANCODE_KP_3: return KEY_NUMPAD3;
        case SDL_SCANCODE_KP_4: return KEY_NUMPAD4;
        case SDL_SCANCODE_KP_5: return KEY_NUMPAD5;
        case SDL_SCANCODE_KP_6: return KEY_NUMPAD6;
        case SDL_SCANCODE_KP_7: return KEY_NUMPAD7;
        case SDL_SCANCODE_KP_8: return KEY_NUMPAD8;
        case SDL_SCANCODE_KP_9: return KEY_NUMPAD9;
        case SDL_SCANCODE_KP_MINUS: return KEY_SUBTRACT;
        case SDL_SCANCODE_KP_PLUS: return KEY_ADD;
        case SDL_SCANCODE_KP_PERIOD: return KEY_DECIMAL;
        case SDL_SCANCODE_LANG3: return KEY_KANA;

        case SDL_SCANCODE_INTERNATIONAL3: return KEY_YEN;
        //case SDL_SCANCODE_GRAVE: return KEY_CIRCUMFLEX;
        case SDL_SCANCODE_KP_AT: return KEY_AT;
        case SDL_SCANCODE_KP_COLON: return KEY_COLON;
        case SDL_SCANCODE_STOP: return KEY_STOP;
        case SDL_SCANCODE_KP_EQUALS: return KEY_NUMPADEQUALS;
        case SDL_SCANCODE_LANG2: return KEY_KANJI; // is it correct?
        case SDL_SCANCODE_KP_ENTER: return KEY_NUMPADENTER;
        case SDL_SCANCODE_KP_COMMA: return KEY_NUMPADCOMMA;
        case SDL_SCANCODE_KP_DIVIDE: return KEY_DIVIDE;
        case SDL_SCANCODE_SYSREQ: return KEY_SYSRQ;
        case SDL_SCANCODE_HOME: return KEY_HOME;
        case SDL_SCANCODE_UP: return KEY_UP;
        case SDL_SCANCODE_PAGEUP: return KEY_PRIOR;
        case SDL_SCANCODE_LEFT: return KEY_LEFT;
        case SDL_SCANCODE_RIGHT: return KEY_RIGHT;
        case SDL_SCANCODE_END: return KEY_END;
        case SDL_SCANCODE_DOWN: return KEY_DOWN;
        case SDL_SCANCODE_PAGEDOWN: return KEY_NEXT;
        case SDL_SCANCODE_INSERT: return KEY_INSERT;
        case SDL_SCANCODE_DELETE: return KEY_DELETE;
        case SDL_SCANCODE_LGUI: return KEY_LWIN;
        case SDL_SCANCODE_RGUI: return KEY_RWIN;
        case SDL_SCANCODE_APPLICATION: return KEY_APPS;
        case SDL_SCANCODE_NUMLOCKCLEAR: return KEY_NUMLOCK;
        case SDL_SCANCODE_PAUSE: return KEY_PAUSE;

        case SDL_SCANCODE_LCTRL: return KEY_LCONTROL;
        case SDL_SCANCODE_RCTRL: return KEY_RCONTROL;
        case SDL_SCANCODE_LSHIFT: return KEY_LSHIFT;
        case SDL_SCANCODE_RSHIFT: return KEY_RSHIFT;
        case SDL_SCANCODE_LALT: return KEY_LALT;
        case SDL_SCANCODE_RALT: return KEY_RALT;
        default:
            PAUSE(("Unsupported scancode: %d\n", scancode));
    };

    return KEY_ESCAPE;
}

SDL_Scancode remap_gos_to_sdl(const gosEnum_KeyIndex key_index) {

    if(key_index >= KEY_1 && key_index <= KEY_9)
        return (SDL_Scancode)(key_index - KEY_1 + SDL_SCANCODE_1);
    if(key_index == KEY_0) return SDL_SCANCODE_0;

    if(key_index >= KEY_A && key_index <= KEY_Z)
        return (SDL_Scancode)(key_index - KEY_A + SDL_SCANCODE_A);

    switch(key_index) {
        case KEY_ESCAPE: return SDL_SCANCODE_ESCAPE;
        case KEY_MINUS: return SDL_SCANCODE_MINUS;
        case KEY_EQUALS: return SDL_SCANCODE_EQUALS;
        case KEY_BACK: return SDL_SCANCODE_BACKSPACE;
        case KEY_TAB: return SDL_SCANCODE_TAB;
        case KEY_LBRACKET: return SDL_SCANCODE_LEFTBRACKET;
        case KEY_RBRACKET: return SDL_SCANCODE_RIGHTBRACKET;

        // KEY_COLON: 
        case KEY_SEMICOLON: return SDL_SCANCODE_SEMICOLON;

        //case KEY_APOSTROPHE: return SDL_SCANCODE_BACKSLASH;
        // KEY_AT: 
        case KEY_GRAVE: return SDL_SCANCODE_GRAVE;

        //KEY_YEN: //return SDL_SCANCODE_INTERNATIONAL3;
        case KEY_BACKSLASH: return SDL_SCANCODE_BACKSLASH;

        case KEY_COMMA: return SDL_SCANCODE_COMMA;
        case KEY_PERIOD: return SDL_SCANCODE_PERIOD;
        case KEY_SLASH: return SDL_SCANCODE_SLASH;
        case KEY_MULTIPLY: return SDL_SCANCODE_KP_MULTIPLY;
        case KEY_LMENU: return SDL_SCANCODE_MENU;
        case KEY_SPACE: return SDL_SCANCODE_SPACE;
        case KEY_CAPITAL: return SDL_SCANCODE_CAPSLOCK;
        case KEY_F1: return SDL_SCANCODE_F1;
        case KEY_F2: return SDL_SCANCODE_F2;
        case KEY_F3: return SDL_SCANCODE_F3;
        case KEY_F4: return SDL_SCANCODE_F4;
        case KEY_F5: return SDL_SCANCODE_F5;
        case KEY_F6: return SDL_SCANCODE_F6;
        case KEY_F7: return SDL_SCANCODE_F7;
        case KEY_F8: return SDL_SCANCODE_F8;
        case KEY_F9: return SDL_SCANCODE_F9;
        case KEY_F10: return SDL_SCANCODE_F10;
        case KEY_F11: return SDL_SCANCODE_F11;
        case KEY_F12: return SDL_SCANCODE_F12;
        case KEY_F13: return SDL_SCANCODE_F13;
        case KEY_F14: return SDL_SCANCODE_F14;
        case KEY_F15: return SDL_SCANCODE_F15;
        case KEY_NUMPAD0: return SDL_SCANCODE_KP_0;
        case KEY_NUMPAD1: return SDL_SCANCODE_KP_1;
        case KEY_NUMPAD2: return SDL_SCANCODE_KP_2;
        case KEY_NUMPAD3: return SDL_SCANCODE_KP_3;
        case KEY_NUMPAD4: return SDL_SCANCODE_KP_4;
        case KEY_NUMPAD5: return SDL_SCANCODE_KP_5;
        case KEY_NUMPAD6: return SDL_SCANCODE_KP_6;
        case KEY_NUMPAD7: return SDL_SCANCODE_KP_7;
        case KEY_NUMPAD8: return SDL_SCANCODE_KP_8;
        case KEY_NUMPAD9: return SDL_SCANCODE_KP_9;
        case KEY_SUBTRACT: return SDL_SCANCODE_KP_MINUS;
        case KEY_ADD: return SDL_SCANCODE_KP_PLUS;
        case KEY_DECIMAL: return SDL_SCANCODE_KP_PERIOD;
        case KEY_KANA: return SDL_SCANCODE_LANG3;

        //case KEY_CIRCUMFLEX: return SDL_SCANCODE_GRAVE;
        case KEY_NUMPADEQUALS: return SDL_SCANCODE_KP_EQUALS;
        case KEY_KANJI: return SDL_SCANCODE_LANG2;; // is it correct?

        // KEY_NUMPADENTER: 
        case KEY_RETURN: return SDL_SCANCODE_RETURN;

        case KEY_NUMPADCOMMA: return SDL_SCANCODE_KP_COMMA;
        case KEY_DIVIDE: return SDL_SCANCODE_KP_DIVIDE;
        case KEY_SYSRQ: return SDL_SCANCODE_SYSREQ;
        case KEY_HOME: return SDL_SCANCODE_HOME;
        case KEY_UP: return SDL_SCANCODE_UP;
        case KEY_PRIOR: return SDL_SCANCODE_PAGEUP;
        case KEY_LEFT: return SDL_SCANCODE_LEFT;
        case KEY_RIGHT: return SDL_SCANCODE_RIGHT;
        case KEY_END: return SDL_SCANCODE_END;
        case KEY_DOWN: return SDL_SCANCODE_DOWN;
        case KEY_NEXT: return SDL_SCANCODE_PAGEDOWN;
        case KEY_INSERT: return SDL_SCANCODE_INSERT;
        case KEY_DELETE: return SDL_SCANCODE_DELETE;
        case KEY_LWIN: return SDL_SCANCODE_LGUI;
        case KEY_RWIN: return SDL_SCANCODE_RGUI;
        case KEY_APPS: return SDL_SCANCODE_APPLICATION;
        case KEY_NUMLOCK: return SDL_SCANCODE_NUMLOCKCLEAR;

        //KEY_PAUSE: 
        case KEY_STOP:return SDL_SCANCODE_PAUSE;

                        // gos does not differentiate
        //KEY_LCONTROL: 
        case KEY_RCONTROL: return SDL_SCANCODE_LCTRL;

        // KEY_LSHIFT
        case KEY_RSHIFT: return SDL_SCANCODE_LSHIFT;

        case KEY_LALT: return SDL_SCANCODE_LALT;
        case KEY_RALT: return SDL_SCANCODE_RALT;
        default:
            PAUSE(("Unsupported KEY_*: %d\n", key_index));
    };

    return SDL_SCANCODE_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////
DWORD __stdcall gos_GetKey()
{
    if(g_keyboard_info.first_pressed_!=-1) {
        SDL_Scancode sc = (SDL_Scancode)g_keyboard_info.first_pressed_;
        SDL_Keycode kc = SDL_GetKeyFromScancode(sc);
        unsigned char c = 0;
        // check if it represents a character:
        if(!(kc & SDLK_SCANCODE_MASK) && isascii(kc)) {
            c = (unsigned char)kc;

            // this is all hardcodede crap, mayeb will fix it later

			// FIXME: it is incorrect to do so because caps lock is not held constantly but either turned on or off
            //const bool caps_pressed = !!g_keyboard_info.last_state_[SDL_SCANCODE_CAPSLOCK];

            const bool shift_pressed = !!(g_keyboard_info.last_state_[SDL_SCANCODE_LSHIFT] || g_keyboard_info.last_state_[SDL_SCANCODE_RSHIFT]);
            const bool register_mod = /*caps_pressed ^ */shift_pressed; 
            if(c >= SDLK_a && c<=SDLK_z && register_mod)
            {
                c-=32;
            }
            else if(register_mod)
            {
                switch(c)
                {
                    case SDLK_0:        c = SDLK_RIGHTPAREN; break;
                    case SDLK_1:        c = SDLK_EXCLAIM; break;
                    case SDLK_2:        c = SDLK_AT; break;
                    case SDLK_3:        c = SDLK_HASH; break;
                    case SDLK_4:        c = SDLK_DOLLAR; break;
                    case SDLK_5:        c = 37; break; // %
                    case SDLK_6:        c = SDLK_COLON; break;
                    case SDLK_7:        c = SDLK_AMPERSAND; break;
                    case SDLK_8:        c = SDLK_ASTERISK; break;
                    case SDLK_9:        c = SDLK_LEFTPAREN; break;
                    case SDLK_EQUALS:   c = SDLK_PLUS; break;
                    case SDLK_MINUS:    c = SDLK_UNDERSCORE; break;
                    case SDLK_COMMA:    c = SDLK_LESS; break;
                    case SDLK_PERIOD:   c = SDLK_GREATER; break;
                    case SDLK_SLASH:    c = SDLK_QUESTION; break;
                    case SDLK_BACKSLASH:c = '|'; break;
                    default: break;
                }
            }
        }

        gosEnum_KeyIndex ki = remap_sdl_to_gos(sc);

        return (ki<<8) | c;
    }

    if(Environment.ButtonsAsKeys) {
        // TODO: check buttons
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
const char* __stdcall gos_DescribeKey( DWORD Key )
{
    const DWORD k = (Key>>8)&0xFF;
    DWORD ascii = Key&0xFF; (void)ascii; // may be 0 if extended key
    bool is_extended = (Key&0x100)!=0; (void)is_extended;

    SDL_Scancode scancode = remap_gos_to_sdl((gosEnum_KeyIndex)k);
    return SDL_GetScancodeName(scancode);
}

////////////////////////////////////////////////////////////////////////////////
void __stdcall gos_KeyboardFlush()
{
    g_keyboard_info.key_pressed_ = g_keyboard_info.key_released_ = false;
}

////////////////////////////////////////////////////////////////////////////////
gosEnum_KeyStatus __stdcall gos_GetKeyStatus(gosEnum_KeyIndex index)
{
    if(index >= KEY_LMOUSE && index <= KEY_MOUSEX2) {
        gosASSERT(index > 0 && index <= input::MouseInfo::NUM_BUTTONS);
        return (gosEnum_KeyStatus)g_mouse_info.button_state_[index-1];
    }

    // if keyboard
    if((index & 0x01000000) == 0) {
        if(index > 255)
        {
            int asfa=0;
            (void)asfa;
        }
        SDL_Scancode sc = remap_gos_to_sdl((gosEnum_KeyIndex)(index & 0xFF));
        gosASSERT(sc>=0 && sc<sizeof(g_keyboard_info.last_state_)/sizeof(g_keyboard_info.last_state_[0]));

        return (gosEnum_KeyStatus)g_keyboard_info.last_state_[sc];
    }

    return KEY_FREE;
}

