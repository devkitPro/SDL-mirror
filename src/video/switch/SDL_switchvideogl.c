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
#include "SDL_switchvideogl.h"

/*****************************************************************************/
/* SDL OpenGL/OpenGL ES functions                                            */
/*****************************************************************************/

int
SWITCH_GL_LoadLibrary(_THIS, const char *path)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    EGLenum profile = EGL_OPENGL_ES_API;

    printf("SWITCH_GL_LoadLibrary: eglGetDisplay\n");
    data->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!data->display) {
        return SDL_SetError("SWITCH_GL_LoadLibrary: eglGetDisplay = %d", eglGetError());
    }

    printf("SWITCH_GL_LoadLibrary: eglInitialize\n");
    if (eglInitialize(data->display, NULL, NULL) == EGL_FALSE) {
        return SDL_SetError("SWITCH_GL_LoadLibrary: eglInitialize = %d", eglGetError());
    }

    if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_CORE) {
        profile = EGL_OPENGL_API;
    }
    printf("SWITCH_GL_LoadLibrary: eglBindAPI: 0x%x\n", profile);
    if (eglBindAPI(profile) == EGL_FALSE) {
        return SDL_SetError("SWITCH_GL_LoadLibrary: eglBindAPI(0x%x) = %d", eglGetError());
    }

    return 0;
}

void *
SWITCH_GL_GetProcAddress(_THIS, const char *proc)
{
    printf("SWITCH_GL_GetProcAddress: %s\n", proc);
    return eglGetProcAddress(proc);
}

void
SWITCH_GL_UnloadLibrary(_THIS)
{
    SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
    printf("SWITCH_GL_UnloadLibrary\n");
    if (vdata->display) {
        printf("SWITCH_GL_UnloadLibrary: eglTerminate\n");
        eglTerminate(vdata->display);
    }
}

SDL_GLContext
SWITCH_GL_CreateContext(_THIS, SDL_Window *window)
{
    SDL_WindowData *wdata = (SDL_WindowData *) window->driverdata;
    SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
    SDL_PrivateGLESData *gdata = _this->gles_data;

    EGLint attr[32];
    EGLConfig config;
    EGLint num_configs;
    int i = 0;

    wdata->uses_gles = SDL_TRUE;

    printf("SWITCH_GL_CreateContext: profile_mask: %i\n", _this->gl_config.profile_mask);
    attr[i++] = EGL_RED_SIZE;
    attr[i++] = _this->gl_config.red_size;
    attr[i++] = EGL_GREEN_SIZE;
    attr[i++] = _this->gl_config.green_size;
    attr[i++] = EGL_BLUE_SIZE;
    attr[i++] = _this->gl_config.blue_size;

    if (_this->gl_config.alpha_size) {
        attr[i++] = EGL_ALPHA_SIZE;
        attr[i++] = _this->gl_config.alpha_size;
    }

    if (_this->gl_config.buffer_size) {
        attr[i++] = EGL_BUFFER_SIZE;
        attr[i++] = _this->gl_config.buffer_size;
    }

    attr[i++] = EGL_DEPTH_SIZE;
    attr[i++] = _this->gl_config.depth_size;

    if (_this->gl_config.stencil_size) {
        attr[i++] = EGL_STENCIL_SIZE;
        attr[i++] = _this->gl_config.stencil_size;
    }

    if (_this->gl_config.multisamplebuffers) {
        attr[i++] = EGL_SAMPLE_BUFFERS;
        attr[i++] = _this->gl_config.multisamplebuffers;
    }

    if (_this->gl_config.multisamplesamples) {
        attr[i++] = EGL_SAMPLES;
        attr[i++] = _this->gl_config.multisamplesamples;
    }

    attr[i++] = EGL_NONE;

    printf("SWITCH_GL_CreateContext: eglChooseConfig\n");
    eglChooseConfig(vdata->display, attr, &config, 1, &num_configs);
    if (num_configs == 0) {
        SDL_SetError("SWITCH_GL_CreateContext: eglChooseConfig = %d", eglGetError());
        return NULL;
    }

    printf("SWITCH_GL_CreateContext: eglCreateContext\n");
    gdata->context = eglCreateContext(vdata->display, config, EGL_NO_CONTEXT, NULL);
    if (!gdata->context) {
        SDL_SetError("SWITCH_GL_CreateContext: eglCreateContext = %d", eglGetError());
        return NULL;
    }

    printf("SWITCH_GL_CreateContext: eglCreateWindowSurface\n");
    gdata->surface = eglCreateWindowSurface(vdata->display, config, (char *) "", NULL);
    if (!gdata->surface) {
        SDL_SetError("SWITCH_GL_CreateContext: eglCreateWindowSurface = %d", eglGetError());
        return NULL;
    }

    printf("SWITCH_GL_CreateContext: eglMakeCurrent\n");
    if (eglMakeCurrent(vdata->display, gdata->surface, gdata->surface, gdata->context) == EGL_FALSE) {
        SDL_SetError("SWITCH_GL_CreateContext: eglMakeCurrent = %d", eglGetError());
        return NULL;
    }

    return gdata->context;
}

void
SWITCH_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
    EGLBoolean status;

    if (vdata->egl_initialized != SDL_TRUE) {
        SDL_SetError("SWITCH_GL_DeleteContext: egl not initialized");
        return;
    }

    if (vdata->display != EGL_NO_DISPLAY && context != EGL_NO_CONTEXT) {
        printf("SWITCH_GL_DeleteContext\n");
        status = eglDestroyContext(vdata->display, _this->gles_data->context);
        if (status != EGL_TRUE) {
            SDL_SetError("SWITCH_GL_DeleteContext: eglDestroyContext = %d", eglGetError());
        }

        if (_this->gles_data->surface) {
            status = eglDestroySurface(vdata->display, _this->gles_data->surface);
            if (status != EGL_TRUE) {
                SDL_SetError("SWITCH_GL_DeleteContext: eglDestroySurface = %d", eglGetError());
            }
        }
    }
    else {
        SDL_SetError("SWITCH_GL_DeleteContext: EGL_NO_DISPLAY/EGL_NO_CONTEXT");
    }
}

int
SWITCH_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    printf("SWITCH_GL_MakeCurrent\n");
    if (!eglMakeCurrent(((SDL_VideoData *) _this->driverdata)->display,
                        _this->gles_data->surface, _this->gles_data->surface, _this->gles_data->context)) {
        return SDL_SetError("SWITCH_GL_MakeCurrent: eglMakeCurrent = %d", eglGetError());
    }

    return 0;
}

int
SWITCH_GL_SetSwapInterval(_THIS, int interval)
{
    EGLBoolean status;

    printf("SWITCH_GL_SetSwapInterval: %i\n", interval);
    status = eglSwapInterval(((SDL_VideoData *) _this->driverdata)->display, interval);
    if (status == EGL_TRUE) {
        _this->gles_data->swap_interval = (uint32_t) interval;
        return 0;
    }

    return SDL_SetError("SWITCH_GL_SetSwapInterval: eglSwapInterval = %d", eglGetError());
}

int
SWITCH_GL_GetSwapInterval(_THIS)
{
    printf("SWITCH_GL_GetSwapInterval: %i\n", _this->gles_data->swap_interval);
    return _this->gles_data->swap_interval;
}

int
SWITCH_GL_SwapWindow(_THIS, SDL_Window *window)
{
    //printf("SWITCH_GL_SwapWindow\n");
    if (!eglSwapBuffers(((SDL_VideoData *) _this->driverdata)->display,
                        _this->gles_data->surface)) {
        return SDL_SetError("SWITCH_GL_SwapWindow: eglSwapBuffers = %d", eglGetError());
    }
    return 0;
}

#endif /* SDL_VIDEO_DRIVER_SWITCH */

/* vi: set ts=4 sw=4 expandtab: */
