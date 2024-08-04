/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "osdepend.h"

struct OSDJoystick
{
    int         (*init)(options_type *options);
    void        (*exit)(void);
    const struct JoystickInfo* (*get_joy_list)(void);
    int         (*is_joy_pressed)(int joycode);
    void        (*poll_joysticks)(void);
    void        (*analogjoy_read)(int player, int *analog_x, int *analog_y);
    int         (*standard_analog_read)(int player, int axis);

    BOOL        (*Available)(int nJoyStick);
    BOOL        (*OnMessage)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

extern struct OSDJoystick Joystick;

/* cmk temp */
enum
{
   OSD_JOY_LEFT,
   OSD_JOY_RIGHT,
   OSD_JOY_UP,
   OSD_JOY_DOWN,
   OSD_JOY_FIRE,
   OSD_JOY_FIRE1,
   OSD_JOY_FIRE2,
   OSD_JOY_FIRE3,
   OSD_JOY_FIRE4,
   OSD_JOY_FIRE5,
   OSD_JOY_FIRE6,
   OSD_JOY_FIRE7,
   OSD_JOY_FIRE8,
   OSD_JOY_FIRE9,
   OSD_JOY_FIRE10,

   OSD_JOY2_LEFT,
   OSD_JOY2_RIGHT,
   OSD_JOY2_UP,
   OSD_JOY2_DOWN,
   OSD_JOY2_FIRE,
   OSD_JOY2_FIRE1,
   OSD_JOY2_FIRE2,
   OSD_JOY2_FIRE3,
   OSD_JOY2_FIRE4,
   OSD_JOY2_FIRE5,
   OSD_JOY2_FIRE6,
   OSD_JOY2_FIRE7,
   OSD_JOY2_FIRE8,
   OSD_JOY2_FIRE9,
   OSD_JOY2_FIRE10,

   OSD_JOY3_LEFT,
   OSD_JOY3_RIGHT,
   OSD_JOY3_UP,
   OSD_JOY3_DOWN,
   OSD_JOY3_FIRE,
   OSD_JOY3_FIRE1,
   OSD_JOY3_FIRE2,
   OSD_JOY3_FIRE3,
   OSD_JOY3_FIRE4,
   OSD_JOY3_FIRE5,
   OSD_JOY3_FIRE6,
   OSD_JOY3_FIRE7,
   OSD_JOY3_FIRE8,
   OSD_JOY3_FIRE9,
   OSD_JOY3_FIRE10,

   OSD_JOY4_LEFT,
   OSD_JOY4_RIGHT,
   OSD_JOY4_UP,
   OSD_JOY4_DOWN,
   OSD_JOY4_FIRE,
   OSD_JOY4_FIRE1,
   OSD_JOY4_FIRE2,
   OSD_JOY4_FIRE3,
   OSD_JOY4_FIRE4,
   OSD_JOY4_FIRE5,
   OSD_JOY4_FIRE6,
   OSD_JOY4_FIRE7,
   OSD_JOY4_FIRE8,
   OSD_JOY4_FIRE9,
   OSD_JOY4_FIRE10,

   OSD_MAX_JOY,
};

#endif
