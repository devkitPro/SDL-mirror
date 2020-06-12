/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/* This is the system specific header for the SDL joystick API */
#include <nds.h>

#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

/* The private structure used to keep track of a joystick */
struct joystick_hwdata
{
	u32 prev_keys;
};

/* Function to scan the system for joysticks.
 * This function should set SDL_numjoysticks to the number of available
 * joysticks.  Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
int SDL_SYS_JoystickInit(void)
{
	SDL_numjoysticks = 1;

	return 1;
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	if(!index)
		return "NDS builtin joypad";

	SDL_SetError("No joystick available with that index");

	return (NULL);
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	/* allocate memory for system specific hardware data */
	joystick->hwdata = (struct joystick_hwdata *) SDL_malloc(sizeof(*joystick->hwdata));
	if (joystick->hwdata == NULL)
	{
		SDL_OutOfMemory();
		return(-1);
	}
	SDL_memset(joystick->hwdata, 0, sizeof(*joystick->hwdata));

	joystick->nbuttons=8;
	joystick->nhats=1;
	joystick->nballs=0;
	joystick->naxes=0;

	return 0;
}


/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
	const int sdl_buttons[] = {
		KEY_A, KEY_B, KEY_X, KEY_Y, KEY_L, KEY_R, KEY_SELECT, KEY_START
	};

	u32 keys = keysCurrent();
	u32 changed = (keys ^ joystick->hwdata->prev_keys);

	int i;

	if (changed & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
		int hat = SDL_HAT_CENTERED;
		if (keys & KEY_UP) hat |= SDL_HAT_UP;
		if (keys & KEY_DOWN) hat |= SDL_HAT_DOWN;
		if (keys & KEY_LEFT) hat |= SDL_HAT_LEFT;
		if (keys & KEY_RIGHT) hat |= SDL_HAT_RIGHT;
		SDL_PrivateJoystickHat(joystick, 0, hat);
	}

	for (i = 0; i < SDL_arraysize(sdl_buttons); i++) {
		if (changed & sdl_buttons[i]) {
			SDL_PrivateJoystickButton(joystick, i, (keys & sdl_buttons[i]) ? SDL_PRESSED : SDL_RELEASED);
		}
	}

	joystick->hwdata->prev_keys = keys;
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	if (joystick->hwdata != NULL) {
		/* free system specific hardware data */
		SDL_free(joystick->hwdata);
	}
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
}
