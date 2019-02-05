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
#include "SDL_switchvideo.h"
#include "SDL_switchkeyboard.h"
#include "../../events/SDL_keyboard_c.h"

#define NUM_SCANCODES_SWITCH 160

static uint8_t locks = 0;
static bool keystate[NUM_SCANCODES_SWITCH] = { 0 };
static const uint8_t switch_scancodes[NUM_SCANCODES_SWITCH] = {
	KBD_A,
	KBD_B,
	KBD_C,
	KBD_D,
	KBD_E,
	KBD_F,
	KBD_G,
	KBD_H,
	KBD_I,
	KBD_J,
	KBD_K,
	KBD_L,
	KBD_M,
	KBD_N,
	KBD_O,
	KBD_P,
	KBD_Q,
	KBD_R,
	KBD_S,
	KBD_T,
	KBD_U,
	KBD_V,
	KBD_W,
	KBD_X,
	KBD_Y,
	KBD_Z,
	KBD_1,
	KBD_2,
	KBD_3,
	KBD_4,
	KBD_5,
	KBD_6,
	KBD_7,
	KBD_8,
	KBD_9,
	KBD_0,
	KBD_ENTER,
	KBD_ESC,
	KBD_BACKSPACE,
	KBD_TAB,
	KBD_SPACE,
	KBD_MINUS,
	KBD_EQUAL,
	KBD_LEFTBRACE,
	KBD_RIGHTBRACE,
	KBD_BACKSLASH,
	KBD_HASHTILDE,
	KBD_SEMICOLON,
	KBD_APOSTROPHE,
	KBD_GRAVE,
	KBD_COMMA,
	KBD_DOT,
	KBD_SLASH,
	KBD_CAPSLOCK,
	KBD_F1,
	KBD_F2,
	KBD_F3,
	KBD_F4,
	KBD_F5,
	KBD_F6,
	KBD_F7,
	KBD_F8,
	KBD_F9,
	KBD_F10,
	KBD_F11,
	KBD_F12,
	KBD_SYSRQ,
	KBD_SCROLLLOCK,
	KBD_PAUSE,
	KBD_INSERT,
	KBD_HOME,
	KBD_PAGEUP,
	KBD_DELETE,
	KBD_END,
	KBD_PAGEDOWN,
	KBD_RIGHT,
	KBD_LEFT,
	KBD_DOWN,
	KBD_UP,
	KBD_NUMLOCK,
	KBD_KPSLASH,
	KBD_KPASTERISK,
	KBD_KPMINUS,
	KBD_KPPLUS,
	KBD_KPENTER,
	KBD_KP1,
	KBD_KP2,
	KBD_KP3,
	KBD_KP4,
	KBD_KP5,
	KBD_KP6,
	KBD_KP7,
	KBD_KP8,
	KBD_KP9,
	KBD_KP0,
	KBD_KPDOT,
	KBD_102ND,
	KBD_COMPOSE,
	KBD_POWER,
	KBD_KPEQUAL,
	KBD_F13,
	KBD_F14,
	KBD_F15,
	KBD_F16,
	KBD_F17,
	KBD_F18,
	KBD_F19,
	KBD_F20,
	KBD_F21,
	KBD_F22,
	KBD_F23,
	KBD_F24,
	KBD_OPEN,
	KBD_HELP,
	KBD_PROPS,
	KBD_FRONT,
	KBD_STOP,
	KBD_AGAIN,
	KBD_UNDO,
	KBD_CUT,
	KBD_COPY,
	KBD_PASTE,
	KBD_FIND,
	KBD_MUTE,
	KBD_VOLUMEUP,
	KBD_VOLUMEDOWN,
	KBD_CAPSLOCK_ACTIVE,
	KBD_NUMLOCK_ACTIVE,
	KBD_SCROLLLOCK_ACTIVE,
	KBD_KPCOMMA,
	KBD_KPLEFTPAREN,
	KBD_KPRIGHTPAREN,
	KBD_LEFTCTRL,
	KBD_LEFTSHIFT,
	KBD_LEFTALT,
	KBD_LEFTMETA,
	KBD_RIGHTCTRL,
	KBD_RIGHTSHIFT,
	KBD_RIGHTALT,
	KBD_RIGHTMETA,
	KBD_MEDIA_PLAYPAUSE,
	KBD_MEDIA_STOPCD,
	KBD_MEDIA_PREVIOUSSONG,
	KBD_MEDIA_NEXTSONG,
	KBD_MEDIA_EJECTCD,
	KBD_MEDIA_VOLUMEUP,
	KBD_MEDIA_VOLUMEDOWN,
	KBD_MEDIA_MUTE,
	KBD_MEDIA_WWW,
	KBD_MEDIA_BACK,
	KBD_MEDIA_FORWARD,
	KBD_MEDIA_STOP,
	KBD_MEDIA_FIND,
	KBD_MEDIA_SCROLLUP,
	KBD_MEDIA_SCROLLDOWN,
	KBD_MEDIA_EDIT,
	KBD_MEDIA_SLEEP,
	KBD_MEDIA_COFFEE,
	KBD_MEDIA_REFRESH,
	KBD_MEDIA_CALC
};

void
SWITCH_InitKeyboard(void)
{
}

void
SWITCH_PollKeyboard(void)
{
	// We skip polling keyboard if no window is created
	if (SDL_GetFocusWindow() == NULL)
		return;

	for (int i = 0; i < NUM_SCANCODES_SWITCH; i++) {

		int keyCode = switch_scancodes[i];

		if (hidKeyboardHeld(keyCode) && !keystate[i]) {
			switch (keyCode) {
				case SDL_SCANCODE_NUMLOCKCLEAR:
					if (!(locks & 0x1)) {
						SDL_SendKeyboardKey(SDL_PRESSED, keyCode);
						locks |= 0x1;
					} else {
						SDL_SendKeyboardKey(SDL_RELEASED, keyCode);
						locks &= ~0x1;
					}
					break;
				case SDL_SCANCODE_CAPSLOCK:
					if (!(locks & 0x2)) {
						SDL_SendKeyboardKey(SDL_PRESSED, keyCode);
						locks |= 0x2;
					} else {
						SDL_SendKeyboardKey(SDL_RELEASED, keyCode);
						locks &= ~0x2;
					}
					break;
				case SDL_SCANCODE_SCROLLLOCK:
					if (!(locks & 0x4)) {
						SDL_SendKeyboardKey(SDL_PRESSED, keyCode);
						locks |= 0x4;
					} else {
						SDL_SendKeyboardKey(SDL_RELEASED, keyCode);
						locks &= ~0x4;
					}
					break;
				default:
					SDL_SendKeyboardKey(SDL_PRESSED, keyCode);
			}
			keystate[i] = true;
		} else if (!hidKeyboardHeld(keyCode) && keystate[i]) {
			switch (keyCode) {
				case SDL_SCANCODE_CAPSLOCK:
				case SDL_SCANCODE_NUMLOCKCLEAR:
				case SDL_SCANCODE_SCROLLLOCK:
					break;
				default:
					SDL_SendKeyboardKey(SDL_RELEASED, keyCode);
			}
			keystate[i] = false;
		}
	}
}

void
SWITCH_QuitKeyboard(void)
{
}

#endif /* SDL_VIDEO_DRIVER_SWITCH */

/* vi: set ts=4 sw=4 expandtab: */
