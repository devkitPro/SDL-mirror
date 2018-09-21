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
#include "../../render/SDL_sysrender.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_windowevents_c.h"

#include "SDL_switchvideo.h"
#include "SDL_switchopengles.h"
#include "SDL_switchtouch.h"

static int
SWITCH_Available(void)
{
    return 1;
}

static void
SWITCH_Destroy(SDL_VideoDevice *device)
{
    if (device) {
        SDL_free(device);
    }
}

static SDL_VideoDevice *
SWITCH_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;

    /* Initialize SDL_VideoDevice structure */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    /* Setup amount of available displays */
    device->num_displays = 0;

    /* Set device free function */
    device->free = SWITCH_Destroy;

    /* Setup all functions which we can handle */
    device->VideoInit = SWITCH_VideoInit;
    device->VideoQuit = SWITCH_VideoQuit;
    device->GetDisplayModes = SWITCH_GetDisplayModes;
    device->SetDisplayMode = SWITCH_SetDisplayMode;
    device->CreateSDLWindow = SWITCH_CreateWindow;
    device->CreateSDLWindowFrom = SWITCH_CreateWindowFrom;
    device->SetWindowTitle = SWITCH_SetWindowTitle;
    device->SetWindowIcon = SWITCH_SetWindowIcon;
    device->SetWindowPosition = SWITCH_SetWindowPosition;
    device->SetWindowSize = SWITCH_SetWindowSize;
    device->ShowWindow = SWITCH_ShowWindow;
    device->HideWindow = SWITCH_HideWindow;
    device->RaiseWindow = SWITCH_RaiseWindow;
    device->MaximizeWindow = SWITCH_MaximizeWindow;
    device->MinimizeWindow = SWITCH_MinimizeWindow;
    device->RestoreWindow = SWITCH_RestoreWindow;
    device->SetWindowGrab = SWITCH_SetWindowGrab;
    device->DestroyWindow = SWITCH_DestroyWindow;

    device->GL_LoadLibrary = SWITCH_GLES_LoadLibrary;
    device->GL_GetProcAddress = SWITCH_GLES_GetProcAddress;
    device->GL_UnloadLibrary = SWITCH_GLES_UnloadLibrary;
    device->GL_CreateContext = SWITCH_GLES_CreateContext;
    device->GL_MakeCurrent = SWITCH_GLES_MakeCurrent;
    device->GL_SetSwapInterval = SWITCH_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = SWITCH_GLES_GetSwapInterval;
    device->GL_SwapWindow = SWITCH_GLES_SwapWindow;
    device->GL_DeleteContext = SWITCH_GLES_DeleteContext;
    device->GL_DefaultProfileConfig = SWITCH_GLES_DefaultProfileConfig;
    device->GL_GetDrawableSize = SWITCH_GLES_GetDrawableSize;

    device->PumpEvents = SWITCH_PumpEvents;

    return device;
}

VideoBootStrap SWITCH_bootstrap = {
    "Switch",
    "OpenGL ES2 video driver for Nintendo Switch",
    SWITCH_Available,
    SWITCH_CreateDevice
};

/*****************************************************************************/
/* SDL Video and Display initialization/handling functions                   */
/*****************************************************************************/
int
SWITCH_VideoInit(_THIS)
{
    SDL_VideoDisplay display;
    SDL_DisplayMode current_mode;
    SDL_DisplayData *data;
    SDL_DisplayModeData *mdata;

    SDL_zero(current_mode);
    current_mode.w = 1280;
    current_mode.h = 720;
    current_mode.refresh_rate = 60;
    current_mode.format = SDL_PIXELFORMAT_RGBA8888;
    mdata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
    mdata->padding = 0;
    current_mode.driverdata = mdata;

    SDL_zero(display);
    display.desktop_mode = current_mode;
    display.current_mode = current_mode;

    /* Allocate display internal data */
    data = (SDL_DisplayData *) SDL_calloc(1, sizeof(SDL_DisplayData));
    if (data == NULL) {
        return SDL_OutOfMemory();
    }
    data->egl_display = EGL_DEFAULT_DISPLAY;
    display.driverdata = data;

    SDL_AddVideoDisplay(&display);

    // init touch
    SWITCH_InitTouch();

    return 1;
}

void
SWITCH_VideoQuit(_THIS)
{
    // exit touch
    SWITCH_QuitTouch();
}

void
SWITCH_GetDisplayModes(_THIS, SDL_VideoDisplay *display)
{
    SDL_DisplayMode mode;
    SDL_DisplayModeData *data;

    // 1920x1080 (16/9) 16RGBA8888
    if (appletGetOperationMode() == AppletOperationMode_Docked) {
        SDL_zero(mode);
        mode.w = 1920;
        mode.h = 1080;
        mode.refresh_rate = 60;
        mode.format = SDL_PIXELFORMAT_RGBA8888;
        data = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
        data->padding = 0;
        mode.driverdata = data;
        SDL_AddDisplayMode(display, &mode);
    }

    // 1280x720 (16/9) RGBA8888
    SDL_AddDisplayMode(display, &display->current_mode);

    // 960x540 (16/9) RGBA8888
    SDL_zero(mode);
    mode.w = 960;
    mode.h = 540;
    mode.refresh_rate = 60;
    mode.format = SDL_PIXELFORMAT_RGBA8888;
    data = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
    data->padding = 0;
    mode.driverdata = data;
    SDL_AddDisplayMode(display, &mode);

    // 800x600 (4/3) RGBA8888
    SDL_zero(mode);
    mode.w = 800;
    mode.h = 600;
    mode.refresh_rate = 60;
    mode.format = SDL_PIXELFORMAT_RGBA8888;
    data = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
    data->padding = (int) ((600.0f * 1.7774f) - 800.0f) / 2;
    mode.driverdata = data;
    SDL_AddDisplayMode(display, &mode);

    // 640x480 (4/3) RGBA8888
    SDL_zero(mode);
    mode.w = 640;
    mode.h = 480;
    mode.refresh_rate = 60;
    mode.format = SDL_PIXELFORMAT_RGBA8888;
    data = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
    data->padding = (int) ((480.0f * 1.7774f) - 640.0f) / 2;
    mode.driverdata = data;
    SDL_AddDisplayMode(display, &mode);
}

int
SWITCH_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    SDL_Renderer *renderer = SDL_GetRenderer(_this->windows);
    SDL_DisplayModeData *data = (SDL_DisplayModeData *) mode->driverdata;

    if (!data) {
        return -1;
    }

    gfxConfigureResolution(mode->w + data->padding * 2, mode->h);
    display->current_mode = *mode;
    _this->windows->w = mode->w;
    _this->windows->h = mode->h;
    if (renderer) {
        renderer->UpdateViewport(renderer);
    }

    return 0;
}

int
SWITCH_CreateWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *wdata;
    //SDL_VideoDisplay *display;

    /* Allocate window internal data */
    wdata = (SDL_WindowData *) SDL_calloc(1, sizeof(SDL_WindowData));
    if (wdata == NULL) {
        return SDL_OutOfMemory();
    }

    window->flags |= SDL_WINDOW_FULLSCREEN;

    if (!_this->egl_data) {
        return SDL_SetError("SWITCH_CreateWindow: EGL not initialized");
    }
    wdata->egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType) &wdata->egl_surface);

    if (wdata->egl_surface == EGL_NO_SURFACE) {
        return SDL_SetError("Could not create GLES window surface");
    }

    /* Setup driver data for this window */
    window->driverdata = wdata;

    /* One window, it always has focus */
    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);

    /* Window has been successfully created */
    return 0;
}

void
SWITCH_DestroyWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;

    if (data) {
        if (data->egl_surface != EGL_NO_SURFACE) {
            SDL_EGL_DestroySurface(_this, data->egl_surface);
        }
        SDL_free(data);
        window->driverdata = NULL;
    }
}

int
SWITCH_CreateWindowFrom(_THIS, SDL_Window *window, const void *data)
{
    return -1;
}
void
SWITCH_SetWindowTitle(_THIS, SDL_Window *window)
{
}
void
SWITCH_SetWindowIcon(_THIS, SDL_Window *window, SDL_Surface *icon)
{
}
void
SWITCH_SetWindowPosition(_THIS, SDL_Window *window)
{
}
void
SWITCH_SetWindowSize(_THIS, SDL_Window *window)
{
}
void
SWITCH_ShowWindow(_THIS, SDL_Window *window)
{
}
void
SWITCH_HideWindow(_THIS, SDL_Window *window)
{
}
void
SWITCH_RaiseWindow(_THIS, SDL_Window *window)
{
}
void
SWITCH_MaximizeWindow(_THIS, SDL_Window *window)
{
}
void
SWITCH_MinimizeWindow(_THIS, SDL_Window *window)
{
}
void
SWITCH_RestoreWindow(_THIS, SDL_Window *window)
{
}
void
SWITCH_SetWindowGrab(_THIS, SDL_Window *window, SDL_bool grabbed)
{

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
