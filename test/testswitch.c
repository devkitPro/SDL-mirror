/*
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include <stdlib.h>
#include <stdio.h>

#include <switch.h>
#include "SDL2/SDL.h"

int main(int argc, char *argv[])
{
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    int done = 0, x = 0;

    // redirect stdout to emulators
    // consoleDebugInit(debugDevice_SVC);
    // stdout = stderr;

    // mandatory at least on switch, else gfx is not properly closed
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // create an OpenGL ES2 window and renderer
    window = SDL_CreateWindow("switch_gles2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    if (!window) {
        printf("SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // switch support OpenGL ES2 hardware renderer
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // open CONTROLLER_PLAYER_1 and CONTROLLER_PLAYER_2
    // when railed, both joycons are mapped to joystick #0,
    // else joycons are individually mapped to joystick #0, joystick #1, ...
    // https://github.com/devkitPro/SDL/blob/switch-sdl2/src/joystick/switch/SDL_sysjoystick.c#L45
    for (int i = 0; i < 2; i++) {
        if (SDL_JoystickOpen(i) == NULL) {
            printf("SDL_JoystickOpen: %s\n", SDL_GetError());
            SDL_Quit();
            return -1;
        }
    }

    while (!done) {

        while (SDL_PollEvent(&event)) {

            switch (event.type) {

                case SDL_JOYAXISMOTION:
                    printf("Joystick %d axis %d value: %d\n",
                           event.jaxis.which,
                           event.jaxis.axis, event.jaxis.value);
                    break;

                case SDL_JOYBUTTONDOWN:
                    printf("Joystick %d button %d down\n",
                           event.jbutton.which, event.jbutton.button);
                    // seek for joystick #0 down (B)
                    // https://github.com/devkitPro/SDL/blob/switch-sdl2/src/joystick/switch/SDL_sysjoystick.c#L52
                    if (event.jbutton.which == 0 && event.jbutton.button == 1) {
                        printf("exiting...\n");
                        done = 1;
                    }
                    break;

                default:
                    break;
            }
        }


        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // R
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect r = {x, 0, 64, 64};
        SDL_RenderFillRect(renderer, &r);

        // G
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect g = {x + 64, 0, 64, 64};
        SDL_RenderFillRect(renderer, &g);

        // B
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect b = {x + 128, 0, 64, 64};
        SDL_RenderFillRect(renderer, &b);

        SDL_RenderPresent(renderer);

        x++;
        if (x > 256) {
            break;
        }
    }

    printf("SDL_DestroyRenderer...\n");
    SDL_DestroyRenderer(renderer);
    printf("SDL_DestroyWindow...\n");
    SDL_DestroyWindow(window);
    printf("SDL_Quit...\n");
    SDL_Quit();

    return 0;
}

//-----------------------------------------------------------------------------
// nxlink support
//-----------------------------------------------------------------------------

#include <unistd.h>

static int s_nxlinkSock = -1;

static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault()))
        return;

    s_nxlinkSock = nxlinkStdio();
    if (s_nxlinkSock >= 0)
        printf("printf output now goes to nxlink server\n");
    else
        socketExit();
}

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0) {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

void userAppInit()
{
    initNxLink();
}

void userAppExit()
{
    deinitNxLink();
}
