/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  LED.c

 ***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "led.h"
#include "Ntddkbd.h" /* DDK */
#include "winioctl.h"

/***************************************************************************
    Internal structures
 ***************************************************************************/

struct tLED_private
{
    BOOL                            m_bInitialized;
    HANDLE                          m_hKeyboard;
    KEYBOARD_INDICATOR_PARAMETERS	m_KeyboardIndicators;
    KEYBOARD_INDICATOR_PARAMETERS	m_InitialKeyboardIndicators;
};

/***************************************************************************
    Internal variables
 ***************************************************************************/

static struct tLED_private      This = {FALSE, NULL, {0, 0}, {0, 0}};

static const int led_flags[] =
{
    KEYBOARD_NUM_LOCK_ON,
    KEYBOARD_CAPS_LOCK_ON,
    KEYBOARD_SCROLL_LOCK_ON
};

/***************************************************************************
    External functions
 ***************************************************************************/

void LED_init()
{
    BOOL    bResult;
    DWORD	dwBytesRead;

    This.m_bInitialized = FALSE;
    This.m_hKeyboard    = NULL;

    /* Not supported on 95, so m_bInitialized == FALSE. */
    bResult = DefineDosDevice(DDD_RAW_TARGET_PATH,
                              "KEYBOARD",
                              "\\Device\\KeyboardClass0");
    if (bResult == FALSE)
        return;

    This.m_hKeyboard = CreateFile("\\\\.\\KEYBOARD",
                                  GENERIC_READ,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

    if (This.m_hKeyboard == INVALID_HANDLE_VALUE)
    {
        DefineDosDevice(DDD_REMOVE_DEFINITION, "KEYBOARD", NULL);
        return;
    }

    bResult = DeviceIoControl(This.m_hKeyboard,
	                          IOCTL_KEYBOARD_QUERY_INDICATORS,
	                          NULL,
                              0,
	                          &This.m_InitialKeyboardIndicators,
                              sizeof(This.m_InitialKeyboardIndicators),
	                          &dwBytesRead,
	                          NULL);
    if (bResult == FALSE)
    {
        CloseHandle(This.m_hKeyboard);
        DefineDosDevice(DDD_REMOVE_DEFINITION, "KEYBOARD", NULL);
        return;
    }

    memcpy(&This.m_KeyboardIndicators,
           &This.m_InitialKeyboardIndicators,
           sizeof(KEYBOARD_INDICATOR_PARAMETERS));


    This.m_bInitialized = TRUE;
}

void LED_exit()
{
    DWORD	dwBytesRead;

    if (This.m_bInitialized == FALSE)
        return;

    /* Restore original keyboard indicator LEDs. */
    DeviceIoControl(This.m_hKeyboard,
	                IOCTL_KEYBOARD_SET_INDICATORS,
	                &This.m_InitialKeyboardIndicators,
                    sizeof(This.m_InitialKeyboardIndicators),
	                NULL,
                    0,
	                &dwBytesRead,
	                NULL);

    CloseHandle(This.m_hKeyboard);

    DefineDosDevice(DDD_REMOVE_DEFINITION, "KEYBOARD", NULL);

    This.m_bInitialized = FALSE;
}

void LED_write(int nLED, BOOL bOn)
{
    USHORT  LedFlags;

    if (This.m_bInitialized == FALSE)
        return;

    if (3 <= nLED)
        return;

    LedFlags = This.m_KeyboardIndicators.LedFlags;

    if (bOn)
	    LedFlags |=  led_flags[nLED];
    else
	    LedFlags &= ~led_flags[nLED];

    if (LedFlags != This.m_KeyboardIndicators.LedFlags)
    {
        DWORD   dwBytesRead;

        This.m_KeyboardIndicators.LedFlags = LedFlags;

        /* Set keyboard indicator LEDs. */
        DeviceIoControl(This.m_hKeyboard,
	                    IOCTL_KEYBOARD_SET_INDICATORS,
	                    &This.m_KeyboardIndicators,
                        sizeof(This.m_KeyboardIndicators),
	                    NULL,
                        0,
	                    &dwBytesRead,
	                    NULL);
    }
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

