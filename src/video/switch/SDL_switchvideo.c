/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_SWITCH

#include <switch.h>

#include "../SDL_sysvideo.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_windowevents_c.h"

#include "SDL_switchvideo.h"
#include "SDL_switchvideogl.h"
#include "SDL_switchtouch.h"

int SWITCH_Available(void)
{
    return 1;
}

void SWITCH_DeleteDevice(SDL_VideoDevice *device)
{
    if (device->gles_data != NULL) {
        SDL_free(device->gles_data);
        device->gles_data = NULL;
    }

    if (device->driverdata != NULL) {
        SDL_free(device->driverdata);
        device->driverdata = NULL;
    }

    SDL_free(device);
}

SDL_VideoDevice *SWITCH_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    SDL_VideoData *vdata;
    SDL_PrivateGLESData *glesdata;

    printf("SWITCH_CreateDevice\n");

    /* Initialize SDL_VideoDevice structure */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    /* Initialize specific data */
    vdata = (SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    if (vdata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }

    glesdata = (SDL_PrivateGLESData *) SDL_calloc(1, sizeof(SDL_PrivateGLESData));
    if (glesdata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        SDL_free(vdata);
        return NULL;
    }

    device->gles_data = glesdata;
    device->driverdata = vdata;
    vdata->egl_initialized = SDL_TRUE;

    device->num_displays = 0;

    device->VideoInit = SWITCH_VideoInit;
    device->VideoQuit = SWITCH_VideoQuit;
    device->PumpEvents = SWITCH_PumpEvents;
    device->CreateSDLWindow = SWITCH_CreateWindow;
    device->DestroyWindow = SWITCH_DestroyWindow;
    device->free = SWITCH_DeleteDevice;

    device->GL_LoadLibrary = SWITCH_GL_LoadLibrary;
    device->GL_GetProcAddress = SWITCH_GL_GetProcAddress;
    device->GL_UnloadLibrary = SWITCH_GL_UnloadLibrary;
    device->GL_CreateContext = SWITCH_GL_CreateContext;
    device->GL_MakeCurrent = SWITCH_GL_MakeCurrent;
    device->GL_SetSwapInterval = SWITCH_GL_SetSwapInterval;
    device->GL_GetSwapInterval = SWITCH_GL_GetSwapInterval;
    device->GL_SwapWindow = SWITCH_GL_SwapWindow;
    device->GL_DeleteContext = SWITCH_GL_DeleteContext;

    return device;
}

VideoBootStrap SWITCH_bootstrap = {
    "Switch", "Video driver for Nintendo Switch",
    SWITCH_Available, SWITCH_CreateDevice
};

int
SWITCH_VideoInit(_THIS)
{
    SDL_VideoDisplay display;
    SDL_DisplayMode current_mode;

    printf("SWITCH_VideoInit\n");

    SDL_zero(current_mode);
    current_mode.w = 1280;
    current_mode.h = 720;
    current_mode.refresh_rate = 60;
    current_mode.format = SDL_PIXELFORMAT_ABGR8888;
    current_mode.driverdata = NULL;

    SDL_zero(display);
    display.desktop_mode = current_mode;
    display.current_mode = current_mode;
    display.driverdata = NULL;

    SDL_AddVideoDisplay(&display);

    // init touch
    SWITCH_InitTouch();

    return 0;
}

void
SWITCH_VideoQuit(_THIS)
{
    printf("SWITCH_VideoQuit\n");

    // exit touch
    SWITCH_QuitTouch();
}

int
SWITCH_CreateWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *wdata;

    printf("SWITCH_CreateWindow\n");

    /* Allocate window internal data */
    wdata = (SDL_WindowData *) SDL_calloc(1, sizeof(SDL_WindowData));
    if (wdata == NULL) {
        return SDL_OutOfMemory();
    }

    /* Setup driver data for this window */
    window->driverdata = wdata;
    window->flags |= SDL_WINDOW_FULLSCREEN;

    /* Window has been successfully created */
    return 0;
}

void
SWITCH_DestroyWindow(_THIS, SDL_Window *window)
{
    printf("SWITCH_DestroyWindow\n");
}

void
SWITCH_PumpEvents(_THIS)
{
    if (!appletMainLoop()) {
        SDL_Event ev;
        ev.type = SDL_QUIT;
        SDL_PushEvent(&ev);
        return;
    }

    hidScanInput();
    SWITCH_PollTouch();
}

#endif /* SDL_VIDEO_DRIVER_SWITCH */
