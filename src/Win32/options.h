/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define ROTATE_NONE  0
#define ROTATE_RIGHT 1
#define ROTATE_LEFT  2

#define SOUND_NONE   0

#ifndef NOSEAL
#define SOUND_SEAL   (SOUND_NONE + 1)
#else
#define SOUND_SEAL   SOUND_NONE
#endif

#ifndef NOMIDAS
#define SOUND_MIDAS  (SOUND_SEAL + 1)
#else
#define SOUND_MIDAS  SOUND_SEAL
#endif

#define SOUND_DIRECT (SOUND_MIDAS + 1)

#define MAX_GAMEDESC 256

enum {
    COLUMN_GAMES = 0,
    COLUMN_ROMS,
    COLUMN_SAMPLES,
    COLUMN_DIRECTORY,
    COLUMN_TYPE,
    COLUMN_TRACKBALL,
    COLUMN_PLAYED,
    COLUMN_MANUFACTURER,
    COLUMN_YEAR,
    COLUMN_CLONE,
    COLUMN_MAX
};

enum {
    VIEW_LARGE_ICONS = 0,
    VIEW_SMALL_ICONS,
    VIEW_INLIST,
    VIEW_REPORT,
    VIEW_MAX
};

enum {
    SPLITTER_LEFT = 0,
    SPLITTER_RIGHT,
    SPLITTER_MAX
};

/* per-game data we calculate */
enum
{
    UNKNOWN = 2,
};

/* Reg helpers types */
enum RegTypes
{
    RO_BOOL = 0, /* BOOL value                                            */
    RO_INT,      /* int value                                             */
    RO_DOUBLE,   /* double value         -                                */
    RO_STRING,   /* string               - m_vpData is an array           */
    RO_PSTRING,  /* pointer to string    - m_vpData is an allocated array */
    RO_ENCODE    /* encode/decode string - calls decode/encode functions  */
};

/* List of artwork types to display in the screen shot area */
enum {
	PICT_SCREENSHOT = 0,
	PICT_FLYER,
	PICT_CABINET,
	MAX_PICT_TYPES
};

/* Reg data list */
typedef struct
{
    char m_cName[40];                       /* reg key name     */
    int  m_iType;                           /* reg key type     */
    void *m_vpData;                         /* reg key data     */
    void (*encode)(void *data, char *str);  /* encode function  */
    void (*decode)(char *str, void *data);  /* decode function  */
} REG_OPTIONS;

typedef struct
{
    int x, y, width, height;
} AREA;

typedef struct
{
    BOOL use_default; /* only for non-default options */
    
    BOOL is_window;
    BOOL display_best_fit;  /* full screen only */
    int  display_monitor;   /* full screen only */
    int width;              /* full screen only */
    int height;             /* full screen only */
    int skip_lines;         /* full screen only */
    int skip_columns;       /* full screen only */
    BOOL double_vector;
    BOOL window_ddraw;
    BOOL auto_frame_skip;
    int  frame_skip;
    BOOL use_dirty;
    BOOL hscan_lines;
    BOOL vscan_lines;
    BOOL use_blit;
    BOOL disable_mmx;
    int scale;
    int rotate;
    BOOL flipx;
    BOOL flipy;
    double gamma;
    int brightness;
    int depth;
    BOOL antialias;
    BOOL translucency;
    double beam;
    int flicker;

    BOOL use_joystick;
    BOOL use_ai_mouse;
    BOOL di_keyboard;
    BOOL di_joystick;

    int sound;
    int seal_index;
    int sample_rate;
    int sample_bits;
    int volume;
    BOOL stereo;

    BOOL fm_ym3812;

    BOOL cheat;
    BOOL auto_pause;
    BOOL error_log;
    BOOL use_artwork;       /* Use background artwork if found */
    BOOL use_samples;       /* Use samples if needed? */

    int  play_count;
    int  has_roms;
    int  has_samples;
    BOOL is_favorite;
} options_type;

typedef struct
{
    INT      folder_id;
    BOOL     view;
    BOOL     show_folderlist;
    BOOL     show_toolbar;
    BOOL     show_statusbar;
    BOOL     show_screenshot;
	int		 show_pict_type;
    BOOL     game_check;        /* Startup GameCheck */
    BOOL     version_check;     /* Version mismatch warings */
    BOOL     mmx_check;         /* Detect MMX processors */
    char     default_game[MAX_GAMEDESC];
    int      column_width[COLUMN_MAX];
    int      column_order[COLUMN_MAX];
    int      column_shown[COLUMN_MAX];
    int      sort_column;
    BOOL     sort_reverse;
    AREA     area;
    int      splitter[SPLITTER_MAX];
    LOGFONT  list_font;
    COLORREF list_font_color;
    char*    romdirs;
    char*    sampledirs;
    char*    cfgdir;
    char*    hidir;
    char*    inpdir;
    char*    imgdir;
    char*    statedir;
    char*    artdir;
	char*    memcarddir;
	char*	 flyerdir;
	char*    cabinetdir;
} settings_type; /* global settings for the UI only */

#ifdef MAME_NET

typedef struct
{
    char*   net_transport;
    int     net_tcpip_mode;
    char*   net_tcpip_server;    
    int     net_tcpip_port;
    char*   net_player;
    int     net_num_clients;
} net_options;

net_options  * GetNetOptions();

#endif /* MAME_NET */

void OptionsInit(int total_games);
void OptionsExit();

options_type * GetDefaultOptions();
options_type * GetGameOptions(int num_game);

void ResetGUI(void);
void ResetGameDefaults(void);
void ResetAllGameOptions(void);

void SetViewMode(int val);
int  GetViewMode(void);

void SetGameCheck(BOOL game_check);
BOOL GetGameCheck();

void SetVersionCheck(BOOL version_check);
BOOL GetVersionCheck(void);

void SetMMXCheck(BOOL mmx_check);
BOOL GetMMXCheck(void);

void SetSavedFolderID(UINT val);
UINT GetSavedFolderID(void);

void SetShowScreenShot(BOOL val);
BOOL GetShowScreenShot(void);

void SetShowFolderList(BOOL val);
BOOL GetShowFolderList(void);

void SetShowStatusBar(BOOL val);
BOOL GetShowStatusBar(void);

void SetShowToolBar(BOOL val);
BOOL GetShowToolBar(void);

void SetShowPictType(int val);
int  GetShowPictType(void);

void SetDefaultGame(const char *name);
const char *GetDefaultGame(void);

void SetWindowArea(AREA *area);
void GetWindowArea(AREA *area);

void SetColumnWidths(int widths[]);
void GetColumnWidths(int widths[]);

void SetColumnOrder(int order[]);
void GetColumnOrder(int order[]);

void SetColumnShown(int shown[]);
void GetColumnShown(int shown[]);

void SetSplitterPos(int splitterId, int pos);
int  GetSplitterPos(int splitterId);

void SetListFont(LOGFONT *font);
void GetListFont(LOGFONT *font);

DWORD GetFolderFlags(char *folderName);
void  SetFolderFlags(char *folderName, DWORD dwFlags);

void SetListFontColor(COLORREF uColor);
COLORREF GetListFontColor();

void SetSortColumn(int column);
int  GetSortColumn(void);

const char* GetRomDirs(void);
void SetRomDirs(const char* paths);

const char* GetSampleDirs(void);
void  SetSampleDirs(const char* paths);

const char* GetCfgDir(void);
void SetCfgDir(const char* path);

const char* GetHiDir(void);
void SetHiDir(const char* path);

const char* GetInpDir(void);
void SetInpDir(const char* path);

const char* GetImgDir(void);
void SetImgDir(const char* path);

const char* GetStateDir(void);
void SetStateDir(const char* path);

const char* GetArtDir(void);
void SetArtDir(const char* path);

const char* GetMemcardDir(void);
void SetMemcardDir(const char* path);

const char* GetFlyerDir(void);
void SetFlyerDir(const char* path);

const char* GetCabinetDir(void);
void SetCabinetDir(const char* path);

void ResetGameOptions(int num_game);

int  GetHasRoms(int num_game);
void SetHasRoms(int num_game, int has_roms);

int  GetHasSamples(int num_game);
void SetHasSamples(int num_game, int has_samples);

int  GetIsFavorite(int num_game);
void SetIsFavorite(int num_game, BOOL is_favorite);

void IncrementPlayCount(int num_game);
int  GetPlayCount(int num_game);

char * GetVersionString();

void SaveGameOptions(int game_num);
void SaveDefaultOptions(void);

#endif
