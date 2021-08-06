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

#if SDL_VIDEO_DRIVER_RISCOS

#include "../../events/SDL_events_c.h"

#include "SDL_log.h"
#include "SDL_riscosvideo.h"
#include "SDL_riscosevents_c.h"
#include "scancodes_riscos.h"

#include <kernel.h>

static SDL_Scancode
SDL_RISCOS_translate_keycode(int keycode)
{
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;

    if (keycode < SDL_arraysize(riscos_scancode_table)) {
        scancode = riscos_scancode_table[keycode];

        if (scancode == SDL_SCANCODE_UNKNOWN) {
            SDL_Log("The key you just pressed is not recognized by SDL: %d", keycode);
        }
    }

    return scancode;
}

void
RISCOS_PollKeyboard(_THIS)
{
    SDL_VideoData *driverdata = (SDL_VideoData *)_this->driverdata;
    Uint8 key = 2;
    int i;

    /* Check for key releases */
    for (i = 0; i < RISCOS_MAX_KEYS_PRESSED; i++) {
        if (driverdata->key_pressed[i] != 255) {
            if ((_kernel_osbyte(129, driverdata->key_pressed[i] ^ 0xff, 0xff) & 0xff) != 255) {
                SDL_SendKeyboardKey(SDL_RELEASED, SDL_RISCOS_translate_keycode(driverdata->key_pressed[i]));
                driverdata->key_pressed[i] = 255;
            }
        }
    }

    /* Check for key presses */
    while (key < 0xff) {
        SDL_bool already_pressed = SDL_FALSE;
        key = _kernel_osbyte(121, key + 1, 0) & 0xff;
        switch (key) {
        case 255:
        /* Ignore mouse keys */
        case 9:
        case 10:
        case 11:
        /* Ignore keys with multiple INKEY codes */
        case 24:
        case 40:
        case 71:
        case 87:
            break;

        default:
            /* Do we already know of this key? */
            for (i = 0; i < RISCOS_MAX_KEYS_PRESSED; i++) {
                if (driverdata->key_pressed[i] == key) {
                    already_pressed = SDL_TRUE;
                    break;
                }
            }

            if (!already_pressed) {
                SDL_SendKeyboardKey(SDL_PRESSED, SDL_RISCOS_translate_keycode(key));
                /* Record the press so we can detect release later. */
                for (i = 0; i < RISCOS_MAX_KEYS_PRESSED; i++) {
                    if (driverdata->key_pressed[i] == 255) {
                        driverdata->key_pressed[i] = key;
                        break;
                    }
                }
            }
        }
    }
}

int
RISCOS_InitEvents(_THIS)
{
    SDL_VideoData *driverdata = (SDL_VideoData *) _this->driverdata;
    int i, status;

    for (i = 0; i < RISCOS_MAX_KEYS_PRESSED; i++)
        driverdata->key_pressed[i] = 255;

    status = (_kernel_osbyte(202, 0, 255) & 0xFF);
    SDL_ToggleModState(KMOD_NUM,  (status & (1 << 2)) == 0);
    SDL_ToggleModState(KMOD_CAPS, (status & (1 << 4)) == 0);

    /* Disable escape. */
    _kernel_osbyte(229, 1, 0);

    return 0;
}

void
RISCOS_PumpEvents(_THIS)
{
    RISCOS_PollKeyboard(_this);
}

void
RISCOS_QuitEvents(_THIS)
{
    /* Re-enable escape. */
    _kernel_osbyte(229, 0, 0);
}

#endif /* SDL_VIDEO_DRIVER_RISCOS */

/* vi: set ts=4 sw=4 expandtab: */