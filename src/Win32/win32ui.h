/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef __WIN32UI_H__
#define __WIN32UI_H__

#include "options.h"
#include "ScreenShot.h"

enum
{
    TAB_PICKER = 0,
    TAB_DISPLAY,
    TAB_MISC,
    NUM_TABS
};

typedef struct
{
    //int  has_roms;
    //int  has_samples;
    BOOL in_list;
    BOOL neogeo_clone;
} game_data_type;

/* global variables */
extern char *column_names[COLUMN_MAX];

options_type * GetPlayingGameOptions(void);
game_data_type * GetGameData(void);

HWND  GetMainWindow(void);
int   GetNumGames(void);
void  GetRealColumnOrder(int order[]);
HICON LoadIconFromFile(char *iconname);
BOOL  GameUsesTrackball(int game);
void  UpdateScreenShot(void);
void  ResizePickerControls(HWND hWnd);
int   UpdateLoadProgress(int current,int total);
int   UpdateLoadProgress(int current,int total);
// Move The in "The Title (notes)" to "Title, The (notes)"
char *ModifyThe(const char *str);

/* globalized for painting tree control */
extern HBITMAP      hBitmap;
extern HPALETTE     hPALbg;
extern MYBITMAPINFO bmDesc;

#endif
