#include "gameos.hpp"

#include <SDL2/SDL.h> // can avoid inluding this here if move MouseInfo struct to separate file
#include "gos_input.h"

#include<string.h>

extern input::MouseInfo g_mouse_info;

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
    const float w = (float)Environment.screenWidth;
    const float h = (float)Environment.screenHeight;
    if(pXPosition)
        *pXPosition = g_mouse_info.x_ / w;
    if(pYPosition)
        *pYPosition = g_mouse_info.y_ / h;

    if(pXDelta)
        *pXDelta = g_mouse_info.rel_x_ / w;
    if(pYDelta)
        *pYDelta = g_mouse_info.rel_y_ / h;

    if(pWheelDelta)
        *pWheelDelta = g_mouse_info.wheel_vert_ / h;

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


////////////////////////////////////////////////////////////////////////////////
DWORD __stdcall gos_GetKey()
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
const char* __stdcall gos_DescribeKey( DWORD Key )
{
    return "implement me";
}

////////////////////////////////////////////////////////////////////////////////
void __stdcall gos_KeyboardFlush()
{

}

////////////////////////////////////////////////////////////////////////////////
gosEnum_KeyStatus __stdcall gos_GetKeyStatus(gosEnum_KeyIndex index)
{
    if(index >= KEY_LMOUSE && index <= KEY_MOUSEX2) {
        gosASSERT(index > 0 && index <= input::MouseInfo::NUM_BUTTONS);
        return (gosEnum_KeyStatus)g_mouse_info.button_state_[index-1];
    }

    return KEY_FREE;
}

