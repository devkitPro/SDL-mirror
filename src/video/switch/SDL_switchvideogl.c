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
#define EGLCHK(stmt)                            \
    do {                                        \
        EGLint err;                             \
                                                \
        stmt;                                   \
        err = eglGetError();                    \
        if (err != EGL_SUCCESS) {               \
            SDL_SetError("EGL error %d", err);  \
            return 0;                           \
        }                                       \
    } while (0)

int
SWITCH_GL_LoadLibrary(_THIS, const char *path)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    EGLenum profile = EGL_OPENGL_ES_API;

    printf("SWITCH_GL_LoadLibrary: eglGetDisplay\n");
    EGLCHK(data->display = eglGetDisplay(EGL_DEFAULT_DISPLAY));
    printf("SWITCH_GL_LoadLibrary: eglInitialize\n");
    EGLCHK(eglInitialize(data->display, NULL, NULL));

    if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_CORE) {
        profile = EGL_OPENGL_API;
    }
    printf("SWITCH_GL_LoadLibrary: eglBindAPI: 0x%x\n", profile);
    if (eglBindAPI(profile) == EGL_FALSE) {
        SDL_SetError("SWITCH_GL_LoadLibrary: eglBindAPI(0x%x) == EGL_FALSE", profile);
        return 0;
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
        eglTerminate(vdata->display);
    }
}

SDL_GLContext
SWITCH_GL_CreateContext(_THIS, SDL_Window *window)
{
    SDL_WindowData *wdata = (SDL_WindowData *) window->driverdata;
    SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;

    EGLint attribs[32];
    EGLConfig config;
    EGLint num_configs;
    int i;

    printf("SWITCH_GL_CreateContext\n");

    wdata->uses_gles = SDL_TRUE;
    window->flags |= SDL_WINDOW_FULLSCREEN;

    printf("SWITCH_GL_CreateContext: profile_mask: %i\n", _this->gl_config.profile_mask);
    printf("SWITCH_GL_CreateContext: red_size: %i\n", _this->gl_config.red_size);
    printf("SWITCH_GL_CreateContext: green_size: %i\n", _this->gl_config.green_size);
    printf("SWITCH_GL_CreateContext: blue_size: %i\n", _this->gl_config.blue_size);

    /* Setup the config based on SDL's current values. */
    i = 0;
    attribs[i++] = EGL_RED_SIZE;
    attribs[i++] = _this->gl_config.red_size;
    attribs[i++] = EGL_GREEN_SIZE;
    attribs[i++] = _this->gl_config.green_size;
    attribs[i++] = EGL_BLUE_SIZE;
    attribs[i++] = _this->gl_config.blue_size;

    attribs[i++] = EGL_DEPTH_SIZE;
    attribs[i++] = _this->gl_config.depth_size;

    if (_this->gl_config.alpha_size) {
        attribs[i++] = EGL_ALPHA_SIZE;
        attribs[i++] = _this->gl_config.alpha_size;
    }
    if (_this->gl_config.stencil_size) {
        attribs[i++] = EGL_STENCIL_SIZE;
        attribs[i++] = _this->gl_config.stencil_size;
    }

    attribs[i++] = EGL_NONE;

    printf("SWITCH_GL_CreateContext: eglChooseConfig\n");
    EGLCHK(eglChooseConfig(vdata->display, attribs, &config, 1, &num_configs));
    if (num_configs == 0) {
        SDL_SetError("No valid EGL configs for requested mode");
        return 0;
    }

    printf("SWITCH_GL_CreateContext: eglCreateContext\n");
    EGLCHK(_this->gles_data->context = eglCreateContext(vdata->display, config, EGL_NO_CONTEXT, NULL));
    printf("SWITCH_GL_CreateContext: eglCreateWindowSurface\n");
    EGLCHK(_this->gles_data->surface = eglCreateWindowSurface(vdata->display, config, (char *) "", NULL));
    printf("SWITCH_GL_CreateContext: eglMakeCurrent\n");
    EGLCHK(eglMakeCurrent(vdata->display, _this->gles_data->surface,
                          _this->gles_data->surface, _this->gles_data->context));

    return _this->gles_data->context;
}

int
SWITCH_GL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    /*
    if (!window || !context) {
        printf("SWITCH_GL_MakeCurrent(EGL_NO_SURFACE)\n");
        eglMakeCurrent(_this->gles_data->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        return 0;
    }
    */

    printf("SWITCH_GL_MakeCurrent\n");
    if (!eglMakeCurrent(((SDL_VideoData *) _this->driverdata)->display,
                        _this->gles_data->surface, _this->gles_data->surface, _this->gles_data->context)) {
        printf("SWITCH_GL_MakeCurrent: Unable to make EGL context current\n");
        return SDL_SetError("SWITCH_GL_MakeCurrent: Unable to make EGL context current");
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

    return SDL_SetError("Unable to set the EGL swap interval");
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
        return SDL_SetError("SWITCH_GL_SwapWindow: eglSwapBuffers() failed");
    }
    return 0;
}

void
SWITCH_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
    EGLBoolean status;

    if (vdata->egl_initialized != SDL_TRUE) {
        SDL_SetError("SWITCH_GL_DeleteContext: egl not initialize");
        printf("SWITCH_GL_DeleteContext: egl not initialize\n");
        return;
    }

    if (vdata->display != EGL_NO_DISPLAY && context != EGL_NO_CONTEXT) {
        printf("SWITCH_GL_DeleteContext\n");
        status = eglDestroyContext(vdata->display, _this->gles_data->context);
        if (status != EGL_TRUE) {
            SDL_SetError("SWITCH_GL_DeleteContext: OpenGL ES context destroy error");
            printf("SWITCH_GL_DeleteContext: OpenGL ES context destroy error\n");
        }

        if (_this->gles_data->surface) {
            eglDestroySurface(vdata->display, _this->gles_data->surface);
        }
    }
    else {
        printf("SWITCH_GL_DeleteContext: EGL_NO_DISPLAY/EGL_NO_CONTEXT\n");
    }
}

#endif /* SDL_VIDEO_DRIVER_SWITCH */

/* vi: set ts=4 sw=4 expandtab: */
