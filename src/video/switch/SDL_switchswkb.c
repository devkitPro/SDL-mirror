//
// Created by cpasjuste on 22/04/2020.
//

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_SWITCH

#include <switch.h>
#include "SDL_switchswkb.h"

static SwkbdConfig kbd;
static bool kbd_inited = SDL_FALSE;
static char kbd_str[512] = {0};

void
SWITCH_InitSwkb()
{

}

void SWITCH_QuitSwkb()
{
    if(kbd_inited) {
        swkbdClose(&kbd);
        kbd_inited = false;
    }
}

SDL_bool
SWITCH_HasScreenKeyboardSupport(_THIS)
{
    return SDL_TRUE;
}

SDL_bool
SWITCH_IsScreenKeyboardShown(_THIS, SDL_Window * window)
{
    // we dont use (need?) inline/async swkb for now...
    return false;
}

void
SWITCH_StartTextInput(_THIS)
{
    Result rc;

    if(!kbd_inited) {
        rc = swkbdCreate(&kbd, 0);
        if (R_SUCCEEDED(rc)) {
            swkbdConfigMakePresetDefault(&kbd);
            kbd_inited = true;
        }
    }

    if(kbd_inited) {
        memset(kbd_str, 0, sizeof(kbd_str));
        swkbdShow(&kbd, kbd_str, sizeof(kbd_str));
        if(strlen(kbd_str) > 0) {
            SDL_SendKeyboardText(kbd_str);
        }
    }
}

void
SWITCH_StopTextInput(_THIS)
{
}

#endif
