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

#if SDL_JOYSTICK_SWITCH

/* This is the dummy implementation of the SDL joystick API */

#include "SDL_hints.h"
#include "SDL_events.h"
#include "../SDL_sysjoystick.h"

#include <switch.h>

#define JOYSTICK_COUNT 8

typedef struct JoystickState
{
    HidControllerID id;
    JoystickPosition l_pos;
    JoystickPosition r_pos;
    u64 buttons;
    u32 vibrationDeviceHandles[2][2];
    HidVibrationValue vibrationValues[2];
} JoystickState;

/* Current pad state */
static JoystickState pad[JOYSTICK_COUNT];

static HidControllerID pad_id[JOYSTICK_COUNT] = {
        CONTROLLER_P1_AUTO, CONTROLLER_PLAYER_2,
        CONTROLLER_PLAYER_3, CONTROLLER_PLAYER_4,
        CONTROLLER_PLAYER_5, CONTROLLER_PLAYER_6,
        CONTROLLER_PLAYER_7, CONTROLLER_PLAYER_8
};

static const HidControllerKeys pad_mapping[] = {
        KEY_A, KEY_B, KEY_X, KEY_Y,
        KEY_LSTICK, KEY_RSTICK,
        KEY_L, KEY_R,
        KEY_ZL, KEY_ZR,
        KEY_PLUS, KEY_MINUS,
        KEY_DLEFT, KEY_DUP, KEY_DRIGHT, KEY_DDOWN,
        KEY_LSTICK_LEFT, KEY_LSTICK_UP, KEY_LSTICK_RIGHT, KEY_LSTICK_DOWN,
        KEY_RSTICK_LEFT, KEY_RSTICK_UP, KEY_RSTICK_RIGHT, KEY_RSTICK_DOWN,
        KEY_SL_LEFT, KEY_SR_LEFT, KEY_SL_RIGHT, KEY_SR_RIGHT,
        // Generic catch-all directions, also works for single Joy-Con
        KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_SL, KEY_SR, KEY_PLUS | KEY_MINUS
};

/* Function to scan the system for joysticks.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
static int
SWITCH_JoystickInit(void)
{
    for (int i = 0; i < JOYSTICK_COUNT; i++) {
        pad[i].id = pad_id[i];
        hidInitializeVibrationDevices(pad[i].vibrationDeviceHandles[0], 2, CONTROLLER_HANDHELD, TYPE_HANDHELD);
        if(pad[i].id == CONTROLLER_P1_AUTO) {
            hidInitializeVibrationDevices(pad[i].vibrationDeviceHandles[1], 2, CONTROLLER_PLAYER_1, TYPE_HANDHELD);
        } else {
            hidInitializeVibrationDevices(pad[i].vibrationDeviceHandles[1], 2, pad[i].id, TYPE_JOYCON_PAIR);
        }
    }

    return JOYSTICK_COUNT;
}

static int
SWITCH_JoystickGetCount(void)
{
    return JOYSTICK_COUNT;
}

static void
SWITCH_JoystickDetect(void)
{
}

/* Function to get the device-dependent name of a joystick */
static const char *
SWITCH_JoystickGetDeviceName(int device_index)
{
    return "Switch Controller";
}

static int
SWITCH_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void
SWITCH_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_JoystickGUID
SWITCH_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID guid;
    /* the GUID is just the first 16 chars of the name for now */
    const char *name = SWITCH_JoystickGetDeviceName(device_index);
    SDL_zero(guid);
    SDL_memcpy(&guid, name, SDL_min(sizeof(guid), SDL_strlen(name)));
    return guid;
}

/* Function to perform the mapping from device index to the instance id for this index */
static SDL_JoystickID
SWITCH_JoystickGetDeviceInstanceID(int device_index)
{
    return device_index;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
static int
SWITCH_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    joystick->nbuttons = sizeof(pad_mapping) / sizeof(*pad_mapping);
    joystick->naxes = 4;
    joystick->nhats = 0;
    joystick->instance_id = device_index;

    return 0;
}

static int
SWITCH_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    int target_device = 0;
    int id = joystick->instance_id;

    if (!hidGetHandheldMode()) {
        target_device = 1;
    }

    pad[id].vibrationValues[0].amp_low = pad[id].vibrationValues[0].amp_high = low_frequency_rumble == 0 ? 0.0f : 320.0f;
    pad[id].vibrationValues[0].freq_low = low_frequency_rumble == 0 ? 160.0f : (float) low_frequency_rumble / 204;
    pad[id].vibrationValues[0].freq_high = high_frequency_rumble == 0 ? 320.0f : (float) high_frequency_rumble / 204;
    memcpy(&pad[id].vibrationValues[1], &pad[id].vibrationValues[0], sizeof(HidVibrationValue));

    hidSendVibrationValues(pad[id].vibrationDeviceHandles[target_device], pad[id].vibrationValues, 2);

    return 0;
}

static void
SWITCH_HandleSingleJoycons(int mode)
{
    // handle single joycon's states (@ rsn8887)
    static int previous_single_joycon_mode = -1;
    static int previous_handheld_mode = -1;
    int lastRightId = 8;
    int handheld_mode = hidGetHandheldMode();

    if ((mode != previous_single_joycon_mode) || (handheld_mode != previous_handheld_mode)) {
        previous_handheld_mode = handheld_mode;
        previous_single_joycon_mode = mode;
        if (!handheld_mode) {
            if (mode) {
                for (int id = 0; id < 8; id++) {
                    hidSetNpadJoyAssignmentModeSingleByDefault((HidControllerID) id);
                }
                hidSetNpadJoyHoldType(HidJoyHoldType_Horizontal);
                hidScanInput();
            } else {
                // find all left/right single JoyCon pairs and join them together
                for (int id = 0; id < 8; id++) {
                    hidSetNpadJoyAssignmentModeDual((HidControllerID) id);
                }
                for (int id0 = 0; id0 < 8; id0++) {
                    if (hidGetControllerType((HidControllerID) id0) & TYPE_JOYCON_LEFT) {
                        for (int id1 = lastRightId - 1; id1 >= 0; id1--) {
                            if (hidGetControllerType((HidControllerID) id1) & TYPE_JOYCON_RIGHT) {
                                lastRightId = id1;
                                // prevent missing player numbers
                                if (id0 < id1) {
                                    hidMergeSingleJoyAsDualJoy((HidControllerID) id0, (HidControllerID) id1);
                                } else if (id0 > id1) {
                                    hidMergeSingleJoyAsDualJoy((HidControllerID) id1, (HidControllerID) id0);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
static void
SWITCH_JoystickUpdate(SDL_Joystick *joystick)
{
    u64 changed = 0;
    static JoystickState pad_old[JOYSTICK_COUNT];
    const char *single_joycons_hint = NULL;

    int index = (int) SDL_JoystickInstanceID(joystick);
    if (index > JOYSTICK_COUNT || SDL_IsTextInputActive()) {
        return;
    }

    hidJoystickRead(&pad[index].l_pos, pad[index].id, JOYSTICK_LEFT);
    hidJoystickRead(&pad[index].r_pos, pad[index].id, JOYSTICK_RIGHT);
    pad[index].buttons = hidKeysHeld(pad[index].id);

    // Axes
    if (pad_old[index].l_pos.dx != pad[index].l_pos.dx) {
        SDL_PrivateJoystickAxis(joystick, 0, (Sint16) pad[index].l_pos.dx);
        pad_old[index].l_pos.dx = pad[index].l_pos.dx;
    }
    if (pad_old[index].l_pos.dy != pad[index].l_pos.dy) {
        SDL_PrivateJoystickAxis(joystick, 1, (Sint16) -pad[index].l_pos.dy);
        pad_old[index].l_pos.dy = -pad[index].l_pos.dy;
    }
    if (pad_old[index].r_pos.dx != pad[index].r_pos.dx) {
        SDL_PrivateJoystickAxis(joystick, 2, (Sint16) pad[index].r_pos.dx);
        pad_old[index].r_pos.dx = pad[index].r_pos.dx;
    }
    if (pad_old[index].r_pos.dy != pad[index].r_pos.dy) {
        SDL_PrivateJoystickAxis(joystick, 3, (Sint16) -pad[index].r_pos.dy);
        pad_old[index].r_pos.dy = -pad[index].r_pos.dy;
    }

    // Buttons
    changed = pad_old[index].buttons ^ pad[index].buttons;
    pad_old[index].buttons = pad[index].buttons;
    if (changed) {
        for (int i = 0; i < joystick->nbuttons; i++) {
            if (changed & pad_mapping[i]) {
                SDL_PrivateJoystickButton(
                        joystick, (Uint8) i,
                        (Uint8) ((pad[index].buttons & pad_mapping[i]) ? SDL_PRESSED : SDL_RELEASED));
            }
        }
    }

    // handle single joycons mode
    single_joycons_hint = SDL_GetHint("SDL_HINT_SINGLE_JOYCONS_MODE");
    if(single_joycons_hint != NULL) {
        SWITCH_HandleSingleJoycons(SDL_strncmp(single_joycons_hint, "1", 1) == 0);
    }
}

/* Function to close a joystick after use */
static void
SWITCH_JoystickClose(SDL_Joystick *joystick)
{
}

/* Function to perform any system-specific joystick related cleanup */
static void
SWITCH_JoystickQuit(void)
{
}

SDL_JoystickDriver SDL_SWITCH_JoystickDriver =
{
    SWITCH_JoystickInit,
    SWITCH_JoystickGetCount,
    SWITCH_JoystickDetect,
    SWITCH_JoystickGetDeviceName,
    SWITCH_JoystickGetDevicePlayerIndex,
    SWITCH_JoystickSetDevicePlayerIndex,
    SWITCH_JoystickGetDeviceGUID,
    SWITCH_JoystickGetDeviceInstanceID,
    SWITCH_JoystickOpen,
    SWITCH_JoystickRumble,
    SWITCH_JoystickUpdate,
    SWITCH_JoystickClose,
    SWITCH_JoystickQuit,
};

#endif /* SDL_JOYSTICK_SWITCH */

/* vi: set ts=4 sw=4 expandtab: */