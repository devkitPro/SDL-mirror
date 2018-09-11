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

#ifndef __SDL_SWITCHVIDEO_H__
#define __SDL_SWITCHVIDEO_H__

#if SDL_VIDEO_DRIVER_SWITCH

#include <glad/glad.h>

#include "../../SDL_internal.h"
#include "../SDL_sysvideo.h"

typedef struct SDL_VideoData
{
    SDL_bool egl_initialized;
    EGLDisplay display;
} SDL_VideoData;

typedef struct SDL_WindowData
{
    SDL_bool uses_gles;
} SDL_WindowData;

int SWITCH_Available(void);
int SWITCH_VideoInit(_THIS);
void SWITCH_VideoQuit(_THIS);
void SWITCH_PumpEvents(_THIS);

int SWITCH_CreateWindow(_THIS, SDL_Window *window);
void SWITCH_DestroyWindow(_THIS, SDL_Window *window);

#endif /* SDL_VIDEO_DRIVER_SWITCH */
#endif /* __SDL_SWITCHVIDEO_H__ */

/* vi: set ts=4 sw=4 expandtab: */
