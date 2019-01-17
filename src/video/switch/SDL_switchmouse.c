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

#include "SDL_events.h"
#include "SDL_log.h"
#include "SDL_mouse.h"
#include "SDL_switchvideo.h"
#include "SDL_switchmouse_c.h"
#include "../../events/SDL_mouse_c.h"

static Uint64 prev_buttons = 0;
static int prev_x = 0;
static int prev_y = 0;

void 
SWITCH_InitMouse(void)
{
}

void 
SWITCH_PollMouse(void)
{
	SDL_Window *window = SDL_GetFocusWindow();
	Uint64 buttons;
	Uint64 changed_buttons;
	MousePosition mouse_pos;
	int x,y;

	// We skip polling mouse if no window is created
	if (window == NULL)
		return;

	buttons = hidMouseButtonsHeld() | hidMouseButtonsDown();
	changed_buttons = buttons ^ prev_buttons;

	if (changed_buttons & MOUSE_LEFT) {
		if (prev_buttons & MOUSE_LEFT)
			SDL_SendMouseButton(window, 0, SDL_RELEASED, SDL_BUTTON_LEFT);
		else
			SDL_SendMouseButton(window, 0, SDL_PRESSED, SDL_BUTTON_LEFT);
	}
	if (changed_buttons & MOUSE_RIGHT) {
		if (prev_buttons & MOUSE_RIGHT)
			SDL_SendMouseButton(window, 0, SDL_RELEASED, SDL_BUTTON_RIGHT);
		else
			SDL_SendMouseButton(window, 0, SDL_PRESSED, SDL_BUTTON_RIGHT);
	}
	if (changed_buttons & MOUSE_MIDDLE) {
		if (prev_buttons & MOUSE_MIDDLE)
			SDL_SendMouseButton(window, 0, SDL_RELEASED, SDL_BUTTON_MIDDLE);
		else
			SDL_SendMouseButton(window, 0, SDL_PRESSED, SDL_BUTTON_MIDDLE);
	}

	prev_buttons = buttons;
	
	hidMouseRead(&mouse_pos);
	x = (int) mouse_pos.x;
	y = (int) mouse_pos.y;
	if (x != prev_x || y != prev_y)
	{
		int dx, dy;
		dx = x - prev_x;
		dy = y - prev_y;
		SDL_SendMouseMotion(window, 0, 1, dx, dy);
		prev_x = x;
		prev_y = y;
	}
}

#endif /* SDL_VIDEO_DRIVER_SWITCH */

/* vi: set ts=4 sw=4 expandtab: */
