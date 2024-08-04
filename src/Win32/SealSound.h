/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef __SEALSOUND_H__
#define __SEALSOUND_H__

#include "Sound.h"

#define MAXSOUNDDEVICES 32

#define SEALSOUND_PRODUCT_NONE      0x0000
#define SEALSOUND_PRODUCT_WINDOWS   0x0100
#define SEALSOUND_PRODUCT_DSOUND    0x0104

struct tSealSoundDevice
{
    char    m_pName[30];
    UINT    m_nDeviceID;
    WORD    m_wProductID;
};

struct tSealSoundDevices
{
    struct tSealSoundDevice m_Devices[MAXSOUNDDEVICES];
    int                     m_nNumDevices;
};

extern struct tSealSoundDevices*    SealSound_GetSoundDevices();

extern struct OSDSound SealSound;

#endif
