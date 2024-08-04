/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

    directdraw.c

    Direct Draw routines.
 
 ***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "DirectDraw.h"
#include "M32Util.h"
#include "win32ui.h"
#include "mame32.h"

#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

/***************************************************************************
    function prototypes
 ***************************************************************************/

static BOOL WINAPI DDEnumInfo(GUID FAR *lpGUID,
                              LPSTR     lpDriverDescription,
                              LPSTR     lpDriverName,        
                              LPVOID    lpContext,
                              HMONITOR  hm);

static BOOL WINAPI DDEnumOldInfo(GUID FAR *lpGUID,
                                 LPSTR     lpDriverDescription,
                                 LPSTR     lpDriverName,        
                                 LPVOID    lpContext);

static void CalculateDisplayModes();
static HRESULT CALLBACK EnumDisplayModesCallback(DDSURFACEDESC* pddsd, LPVOID Context);

/***************************************************************************
    External variables
 ***************************************************************************/

LPDIRECTDRAW    dd = NULL;

/***************************************************************************
    Internal structures
 ***************************************************************************/

typedef struct
{
   char *name;
   GUID *lpguid;
} display_type;

typedef HRESULT (WINAPI *ddc_proc)(GUID FAR *lpGUID,LPDIRECTDRAW FAR *lplpDD,
                                   IUnknown FAR *pUnkOuter);

/***************************************************************************
    Internal variables
 ***************************************************************************/

static HANDLE hDLL = NULL;

static ddc_proc ddc;

#define MAX_DISPLAYS 100
static int num_displays = 0;
static display_type displays[MAX_DISPLAYS];

static LPGUID lp_current_display_guid;

static struct tDisplayModes DisplayModes;

/***************************************************************************
    External functions  
 ***************************************************************************/

/****************************************************************************
 *      DirectDrawInitialize
 *
 *      Initialize the DirectDraw variables.
 *
 *      This entails the following functions:
 *
 *          DirectDrawCreate
 *
 ****************************************************************************/

#ifndef LPDIRECTDRAWNUMERATEEX
#ifdef UNICODE
/* For our friends doing translations */

typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXW)(GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR);
typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKW)(GUID FAR *, LPSTR, LPSTR, LPVOID);
typedef HRESULT (WINAPI *  LPDIRECTDRAWENUMERATEEXW)( LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT (WINAPI *  LPDIRECTDRAWENUMERATEW)( LPDDENUMCALLBACKW lpCallback, LPVOID lpContext ); 

#define LPDIRECTDRAWENUMERATEEX LPDIRECTDRAWENUMERATEEXW
#define LPDIRECTDRAWENUMERATE   LPDIRECTDRAWENUMERATEW

#define SDirectDrawEnumerateEx	    "DirectDrawEnumerateExW"
#define SDirectDrawEnumerate	    "DirectDrawEnumerateW"

#else   /* !UNICODE */

typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXA)(GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR);
typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKA)(GUID FAR *, LPSTR, LPSTR, LPVOID);
typedef HRESULT (WINAPI *  LPDIRECTDRAWENUMERATEEXA)( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT (WINAPI *  LPDIRECTDRAWENUMERATEA)( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext ); 

#define LPDIRECTDRAWENUMERATEEX LPDIRECTDRAWENUMERATEEXA
#define LPDIRECTDRAWENUMERATE   LPDIRECTDRAWENUMERATEA

#define SDirectDrawEnumerateEx	    "DirectDrawEnumerateExA"
#define SDirectDrawEnumerate 	    "DirectDrawEnumerateA"

#endif /* !UNICODE */
#endif /* LPDIRECTDRAWNUMERATEEX */


/****************************************************************************/
BOOL DirectDraw_Initialize()
{
    HRESULT hr;
    UINT error_mode;
    LPDIRECTDRAWENUMERATEEX lpDDEnumEx;

    if (hDLL != NULL)
        return TRUE;

    num_displays = 0;

    /* Turn off error dialog for this call */
    error_mode = SetErrorMode(0);
    hDLL = LoadLibrary("ddraw.dll");
    SetErrorMode(error_mode);

    if (hDLL == NULL)
       return FALSE;

    ddc = (ddc_proc)GetProcAddress(hDLL,"DirectDrawCreate");
    if (ddc == NULL)
        return FALSE;

    lp_current_display_guid = NULL;

    hr = ddc(NULL, &dd, NULL);
    if (FAILED(hr)) 
    {
        ErrorMsg("DirectDrawCreate failed! error=%x\n", hr);
        dd = NULL;
        return FALSE;
    }

    // Note that you must know which version of the
    // function to retrieve (see the following text).
    // For this example, we use the ANSI version.
    lpDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(hDLL,SDirectDrawEnumerateEx);

    // If the function is there, call it to enumerate all display devices
    // attached to the desktop, and any non-display DirectDraw devices.
//MSVC4 says no
#if 0
	if (lpDDEnumEx)
        lpDDEnumEx(DDEnumInfo, NULL, 
                   DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES );
    else
#endif
    {
        LPDIRECTDRAWENUMERATE lpDDEnum;

        lpDDEnum = (LPDIRECTDRAWENUMERATE) GetProcAddress(hDLL,SDirectDrawEnumerate);
        /*
         * We must be running on an old version of ddraw. Therefore, 
         * by definiton, multimon isn't supported. Fall back on
         * DirectDrawEnumerate to enumerate standard devices on a 
         * single monitor system.
         */
        if (lpDDEnum)
            lpDDEnum(DDEnumOldInfo, NULL); 
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

/****************************************************************************/
void DirectDraw_CreateByIndex(int num_display)
{
    HRESULT hr;

    if (dd != NULL && lp_current_display_guid == displays[num_display].lpguid)
        return;

    if (dd != NULL)
    {
        IDirectDraw_Release(dd);
        dd = NULL;
    }

    hr = ddc(displays[num_display].lpguid, &dd, NULL);

    if (FAILED(hr)) 
    {
        ErrorMsg("DirectDrawCreate failed! error=%x\n", hr);
        dd = NULL;
        DisplayModes.m_nNumModes = 0;
        return;
    }

    lp_current_display_guid = displays[num_display].lpguid;

    CalculateDisplayModes();
}

/****************************************************************************/
/*
    Return a list of 8 and 16 bit DirectDraw modes.
*/
struct tDisplayModes * DirectDraw_GetDisplayModes()
{
    if (DisplayModes.m_nNumModes == 0)
        CalculateDisplayModes();

    return &DisplayModes;

}

/****************************************************************************
 *
 *      DirectDraw_Close
 *
 *      Terminate our usage of DirectDraw.
 *
 ****************************************************************************/

void DirectDraw_Close()
{
   int i;

   for (i=0;i<num_displays;i++)
   {
      free(displays[i].name);
      displays[i].name = NULL;
      if (displays[i].lpguid != NULL)
      {
         free(displays[i].lpguid);
         displays[i].lpguid = NULL;
      }

   }
   num_displays = 0;

    /*
        Destroy any lingering IDirectDraw object.
    */
    if (dd) 
    {
        IDirectDraw_Release(dd);
        dd = NULL;
        FreeLibrary(hDLL);
    }
}

/****************************************************************************/
int DirectDraw_GetNumDisplays()
{
    return num_displays;
}

/****************************************************************************/
char * DirectDraw_GetDisplayName(int num_display)
{
    return displays[num_display].name;
}

/****************************************************************************/
/* internal functions */
/****************************************************************************/

static BOOL WINAPI DDEnumInfo(GUID FAR *lpGUID,
                              LPSTR     lpDriverDescription,
                              LPSTR     lpDriverName,        
                              LPVOID    lpContext,
                              HMONITOR  hm)
{
   displays[num_displays].name = malloc(strlen(lpDriverDescription) + 1);
   strcpy(displays[num_displays].name,lpDriverDescription);

   if (lpGUID == NULL)
      displays[num_displays].lpguid = NULL;
   else
   {
      displays[num_displays].lpguid = (LPGUID)malloc(sizeof(GUID));
      memcpy(displays[num_displays].lpguid,lpGUID,sizeof(GUID));
   }

   num_displays++;
   if (num_displays == MAX_DISPLAYS)
      return DDENUMRET_CANCEL;
   else
      return DDENUMRET_OK;
}

/****************************************************************************/
static BOOL WINAPI DDEnumOldInfo(GUID FAR *lpGUID,
                                 LPSTR     lpDriverDescription,
                                 LPSTR     lpDriverName,        
                                 LPVOID    lpContext)
{
    return DDEnumInfo(lpGUID,lpDriverDescription,lpDriverName,lpContext,NULL);
}


static HRESULT CALLBACK EnumDisplayModesCallback(DDSURFACEDESC* pddsd, LPVOID Context)
{
    struct tDisplayModes*   pDisplayModes = (struct tDisplayModes*)Context;

    if (pddsd->ddpfPixelFormat.dwRGBBitCount == 8
    ||  pddsd->ddpfPixelFormat.dwRGBBitCount == 16)
    {
        pDisplayModes->m_Modes[pDisplayModes->m_nNumModes].m_dwWidth  = pddsd->dwWidth;
        pDisplayModes->m_Modes[pDisplayModes->m_nNumModes].m_dwHeight = pddsd->dwHeight;
        pDisplayModes->m_Modes[pDisplayModes->m_nNumModes].m_dwBPP    = pddsd->ddpfPixelFormat.dwRGBBitCount;
        pDisplayModes->m_nNumModes++;
    }

    if (pDisplayModes->m_nNumModes == MAXMODES)
        return DDENUMRET_CANCEL;
    else
        return DDENUMRET_OK;
}

/****************************************************************************/
static void CalculateDisplayModes()
{
    HRESULT hResult;
    HWND    hWnd;
    
    hWnd = IsWindowVisible(MAME32App.m_hwndUI) ? MAME32App.m_hwndUI : MAME32App.m_hWnd;

    DisplayModes.m_nNumModes = 0;

    if (dd == NULL)
       return;

    hResult = IDirectDraw_SetCooperativeLevel(dd,
                                              hWnd,
                                              DDSCL_EXCLUSIVE  |
                                              DDSCL_FULLSCREEN |
                                              DDSCL_NOWINDOWCHANGES);
    if (FAILED(hResult))
    {
        ErrorMsg("IDirectDraw.SetCooperativeLevel failed error=%x", hResult);
        return;
    }

    IDirectDraw_EnumDisplayModes(dd, 0, NULL, &DisplayModes, EnumDisplayModesCallback);

    hResult = IDirectDraw_SetCooperativeLevel(dd, hWnd, DDSCL_NORMAL);
    if (FAILED(hResult))
    {
        ErrorMsg("IDirectDraw.SetCooperativeLevel failed error=%x", hResult);
        return;
    }

}


