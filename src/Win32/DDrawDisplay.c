/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  DDrawDisplay.c

 ***************************************************************************/

#include "driver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#define DIRECTDRAW_VERSION 0x0300
#include <ddraw.h>
#include <assert.h>
#include <math.h>
#include "MAME32.h"
#include "resource.h"
#include "DDrawDisplay.h"
#include "RenderBitmap.h"
#include "M32Util.h"
#include "dirty.h"
#include "DirectDraw.h"
#include "led.h"
#include "avi.h"

#define MAKECOL(r, g, b) ((r & 0x1F) << (This.m_Red.m_nSize   - 5) << This.m_Red.m_nShift   |    \
                          (g & 0x1F) << (This.m_Green.m_nSize - 5) << This.m_Green.m_nShift |    \
                          (b & 0x1F) << (This.m_Blue.m_nSize  - 5) << This.m_Blue.m_nShift)

/***************************************************************************
    Internal structures
 ***************************************************************************/

struct tPixelInfo
{
    int     m_nShift;
    int     m_nSize;
    DWORD   m_dwMask;
};

struct tDisplay_private
{
    struct osd_bitmap*  m_pMAMEBitmap;

    tRect               m_GameRect;         /* possibly doubled and clipped */
    UINT                m_nDisplayLines;    /* # of visible lines of bitmap */
    UINT                m_nDisplayColumns;  /* # of visible columns of bitmap */
    UINT                m_nSkipLinesMin;
    UINT                m_nSkipLinesMax;
    UINT                m_nSkipColumnsMax;
    UINT                m_nSkipColumnsMin;
    BOOL                m_bPanScreen;
    BOOL                m_bUseBackBuffer;
    BOOL                m_bVDouble;         /* for 1:2 aspect ratio */

    /* Options/Parameters */
    UINT                m_nDisplayIndex;
    BOOL                m_bBestFit;
    UINT                m_nScreenWidth;
    UINT                m_nScreenHeight;
    UINT                m_nSkipLines;
    UINT                m_nSkipColumns;
    BOOL                m_bHScanLines;
    BOOL                m_bVScanLines;
    BOOL                m_bDouble;
    BOOL                m_bStretchX;
    BOOL                m_bStretchY;
    BOOL                m_bUseDirty;
    UINT                m_bBltDouble;
    BOOL                m_bDisableMMX;

    /* AVI Capture params */
    BOOL                m_bAviCapture;      /* Capture AVI mode */
    BOOL                m_bAviRun;          /* Capturing frames */
    int                 m_nAviShowMessage;  /* Show status if !0 */

    /* Palette/Color */
    BOOL                m_bUpdatePalette;
    BOOL                m_bUpdateBackground;
    int                 m_nBlackPen;
    UINT                m_nUIPen;
    PALETTEENTRY        m_PalEntries[OSD_NUMPENS];
    PALETTEENTRY        m_AdjustedPalette[OSD_NUMPENS];
    struct tPixelInfo   m_Red;
    struct tPixelInfo   m_Green;
    struct tPixelInfo   m_Blue;

    /* DirectDraw */
    IDirectDraw*        m_pDDraw;
    IDirectDrawSurface* m_pDDSPrimary;
    IDirectDrawSurface* m_pDDSBack;
    IDirectDrawPalette* m_pDDPal;

    RenderMethod        Render;
};

/***************************************************************************
    Function prototypes
 ***************************************************************************/

static void             ReleaseDDrawObjects(void);
static void             ClearSurface(IDirectDrawSurface* pddSurface);
static void             DrawSurface(IDirectDrawSurface* pddSurface);
static BOOL             BuildPalette(IDirectDraw* pDDraw, IDirectDrawPalette** ppDDPalette);
static void             SetPen(int pen, unsigned char red, unsigned char green, unsigned char blue);
static void             SetPaletteColors(void);
static int              FindBlackPen(void);
static BOOL             SurfaceLockable(IDirectDrawSurface* pddSurface);
static void             GetPixelInfo(DWORD dwMask, struct tPixelInfo* pPixelInfo);
static void             PanDisplay(void);
static void             AdjustDisplay(int xmin, int ymin, int xmax, int ymax);
static void             ScaleVectorGames(int gfx_width, int gfx_height, int *width, int *height);
static void             SelectDisplayMode(int depth);
static BOOL             FindDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwDepth);
static BOOL             FindBestDisplayMode(DWORD  dwWidthIn,   DWORD  dwHeightIn, DWORD dwDepth,
                                DWORD* pdwWidthOut, DWORD* pdwHeightOut);
static void             AdjustPalette(void);
static BOOL             OnSetCursor(HWND hWnd, HWND hWndCursor, UINT codeHitTest, UINT msg);

static int                DDraw_init(options_type *options);
static void               DDraw_exit(void);
static struct osd_bitmap* DDraw_new_bitmap(int width, int height, int depth);
static void               DDraw_clearbitmap(struct osd_bitmap* bitmap);
static void               DDraw_free_bitmap(struct osd_bitmap* bitmap);
static struct osd_bitmap* DDraw_create_display(int width, int height, int depth, int attributes);
static int                DDraw_set_display(int width, int height, int attributes);
static void               DDraw_close_display(void);
static void               DDraw_allocate_colors(unsigned int totalcolors, const unsigned char *palette, unsigned short *pens);
static void               DDraw_modify_pen(int pen, unsigned char red, unsigned char green, unsigned char blue);
static void               DDraw_get_pen(int pen, unsigned char* pRed, unsigned char* pGreen, unsigned char* pBlue);
static void               DDraw_mark_dirty(int x1, int y1, int x2, int y2, int ui);
static void               DDraw_update_display(void);
static void               DDraw_led_w(int led, int on);
static void               DDraw_set_gamma(float gamma);
static void               DDraw_set_brightness(int brightness);
static void               DDraw_save_snapshot(void);
static BOOL               DDraw_OnMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
static void               DDraw_Refresh(void);
static int                DDraw_GetBlackPen(void);
static void               DDraw_UpdateFPS(BOOL bShow, int nSpeed, int nFPS, int nMachineFPS, int nFrameskip, int nVecUPS);

/***************************************************************************
    External variables
 ***************************************************************************/

struct OSDDisplay   DDrawDisplay = 
{
    { DDraw_init },                 /* init              */
    { DDraw_exit },                 /* exit              */
    { DDraw_new_bitmap },           /* new_bitmap        */
    { DDraw_clearbitmap },          /* clearbitmap       */
    { DDraw_free_bitmap },          /* free_bitmap       */
    { DDraw_create_display },       /* create_display    */
    { DDraw_set_display },          /* set_display       */
    { DDraw_close_display },        /* close_display     */
    { DDraw_allocate_colors },      /* allocate_colors   */
    { DDraw_modify_pen },           /* modify_pen        */
    { DDraw_get_pen },              /* get_pen           */
    { DDraw_mark_dirty },           /* mark_dirty        */
    { 0 },                          /* skip_this_frame   */
    { DDraw_update_display },       /* update_display    */
    { DDraw_led_w },                /* led_w             */
    { DDraw_set_gamma },            /* set_gamma         */
    { Display_get_gamma },          /* get_gamma         */
    { DDraw_set_brightness },       /* set_brightness    */
    { Display_get_brightness },     /* get_brightness    */
    { DDraw_save_snapshot },        /* save_snapshot     */
    
    { DDraw_OnMessage },            /* OnMessage         */
    { DDraw_Refresh },              /* Refresh           */
    { DDraw_GetBlackPen },          /* GetBlackPen       */
    { DDraw_UpdateFPS },            /* UpdateFPS         */
};

/***************************************************************************
    Internal variables
 ***************************************************************************/

static struct tDisplay_private      This;

/***************************************************************************
    External OSD function definitions
 ***************************************************************************/

/*
    put here anything you need to do when the program is started. Return 0 if 
    initialization was successful, nonzero otherwise.
*/
static int DDraw_init(options_type *options)
{
    OSDDisplay.init(options);

    This.m_pMAMEBitmap       = NULL;

    This.m_GameRect.m_Top    = 0;
    This.m_GameRect.m_Left   = 0;
    This.m_GameRect.m_Width  = 0;
    This.m_GameRect.m_Height = 0;
    This.m_nDisplayLines     = 0;
    This.m_nDisplayColumns   = 0;
    This.m_bPanScreen        = FALSE;
    This.m_nSkipLinesMin     = 0;
    This.m_nSkipLinesMax     = 0;
    This.m_nSkipColumnsMax   = 0;
    This.m_nSkipColumnsMin   = 0;
    This.m_bUseBackBuffer    = FALSE;
    This.m_bVDouble          = FALSE;

    This.m_nDisplayIndex     = options->display_monitor;
    This.m_bBestFit          = options->display_best_fit;
    This.m_nScreenWidth      = options->width;
    This.m_nScreenHeight     = options->height;
    This.m_nSkipColumns      = max(0, options->skip_columns);
    This.m_nSkipLines        = max(0, options->skip_lines);
    This.m_bHScanLines       = options->hscan_lines;
    This.m_bVScanLines       = options->vscan_lines;
    This.m_bDouble           = (options->scale == 2);
    This.m_bStretchX         = FALSE;
    This.m_bStretchY         = FALSE;
    This.m_bUseDirty         = options->use_dirty;
    This.m_bBltDouble        = options->use_blit;
    This.m_bDisableMMX       = options->disable_mmx;

    This.m_bAviCapture       = GetAviCapture();
    This.m_bAviRun           = FALSE;
    This.m_nAviShowMessage   = (This.m_bAviCapture) ? 10: 0;

    This.m_bUpdatePalette    = FALSE;
    This.m_bUpdateBackground = FALSE;
    This.m_nBlackPen         = 0;
    This.m_nUIPen            = 0;

    This.m_pDDraw            = NULL;
    This.m_pDDSPrimary       = NULL;
    This.m_pDDSBack          = NULL;
    This.m_pDDPal            = NULL;

    This.Render              = NULL;

    memset(This.m_PalEntries,      0, OSD_NUMPENS * sizeof(PALETTEENTRY));
    memset(This.m_AdjustedPalette, 0, OSD_NUMPENS * sizeof(PALETTEENTRY));
    memset(&This.m_Red,   0, sizeof(struct tPixelInfo));
    memset(&This.m_Green, 0, sizeof(struct tPixelInfo));
    memset(&This.m_Blue,  0, sizeof(struct tPixelInfo));


    /* Check for inconsistent parameters. */
    if (This.m_bDouble == FALSE
    && (This.m_bHScanLines == FALSE || This.m_bVScanLines == FALSE))
    {
        This.m_bHScanLines = FALSE;
        This.m_bVScanLines = FALSE;
    }
    
    This.m_bBltDouble = (This.m_bBltDouble  == TRUE  &&
                         This.m_bDouble     == TRUE  && 
                         This.m_bHScanLines == FALSE && 
                         This.m_bVScanLines == FALSE);
    if (This.m_bBltDouble == TRUE)
        This.m_bUseBackBuffer = TRUE;
    
    LED_init();

    return 0;
}

/*
    put here cleanup routines to be executed when the program is terminated.
*/
static void DDraw_exit(void)
{
    OSDDisplay.exit();
    ReleaseDDrawObjects();
    LED_exit();
}

static struct osd_bitmap* DDraw_new_bitmap(int width, int height, int depth)
{
    assert(OSDDisplay.new_bitmap != 0);

    return OSDDisplay.new_bitmap(width, height, depth);
}

static void DDraw_clearbitmap(struct osd_bitmap* bitmap)
{
    assert(OSDDisplay.clearbitmap != 0);

    OSDDisplay.clearbitmap(bitmap);
}

static void DDraw_free_bitmap(struct osd_bitmap* pBitmap)
{
    assert(OSDDisplay.free_bitmap != 0);

    OSDDisplay.free_bitmap(pBitmap);
}

/*
    Create a display surface large enough to accomodate a bitmap
    of the given dimensions.
    Return a osd_bitmap pointer or 0 in case of error.
*/

static struct osd_bitmap *DDraw_create_display(int width, int height, int depth, int attributes)
{
    DDSURFACEDESC           ddSurfaceDesc;
    BOOL                    bResult = TRUE;
    HRESULT                 hResult = DD_OK;
    enum DirtyMode          eDirtyMode = NO_DIRTY;

    /*
        Set up the DirectDraw object.
    */

    DirectDraw_CreateByIndex(This.m_nDisplayIndex);

    This.m_pDDraw = dd;
    if (This.m_pDDraw == NULL)
    {
        ErrorMsg("No DirectDraw object has been created");
        return NULL;
    }

    if (This.m_bUseDirty == TRUE)
    {
        if (attributes & VIDEO_SUPPORTS_DIRTY)
        {
            eDirtyMode = USE_DIRTYRECT;
        }
        else
        {
            /* Don't use dirty if game doesn't support it. */
            eDirtyMode = NO_DIRTY;
        }
    }
    else
    {
        eDirtyMode = NO_DIRTY;
    }

    if ((attributes & VIDEO_PIXEL_ASPECT_RATIO_MASK) == VIDEO_PIXEL_ASPECT_RATIO_1_2)
    {
        This.m_bVDouble    = TRUE;
        This.m_bDouble     = FALSE;
        This.m_bVScanLines = FALSE;
    }

    if (This.m_bBestFit == TRUE)
    {
        SelectDisplayMode(depth);
    }
    else
    {
        BOOL bFound = FALSE;
        /*
            Check to see if display mode exists since user
            can specify any arbitrary size.
        */
        bFound = FindDisplayMode(This.m_nScreenWidth, This.m_nScreenHeight, depth);
        if (bFound == FALSE)
        {
            ErrorMsg("DirectDraw does not support the requested mode %dx%d (%d bpp)",
                     This.m_nScreenWidth,
                     This.m_nScreenHeight,
                     depth);
            goto error;
        }
    }

    if (attributes & VIDEO_TYPE_VECTOR)
    {
        ScaleVectorGames(This.m_nScreenWidth,
                         This.m_nScreenHeight,
                         &width, &height);

        /* Vector games are always non-doubling. */
        This.m_bDouble = FALSE;

        /* Vector monitors have no scanlines. */
        This.m_bHScanLines = FALSE;
        This.m_bVScanLines = FALSE;

        if (This.m_bUseDirty == TRUE)
            eDirtyMode = USE_DIRTYRECT;
        else
            eDirtyMode = NO_DIRTY;

        /* Center display. */
        AdjustDisplay(0, 0, width - 1, height - 1);
    }
    else /* Center display based on visible area. */
    {
        struct rectangle vis = Machine->drv->visible_area;
        AdjustDisplay(vis.min_x, vis.min_y, vis.max_x, vis.max_y);
    }

    /*
        Setup the rendering method.
    */
    if (This.m_bBltDouble == TRUE)
    {
        This.Render = SelectRenderMethod(FALSE,
                                         FALSE,
                                         FALSE,
                                         FALSE,
                                         eDirtyMode,
                                         depth == 16 ? TRUE : FALSE,
                                         MAME32App.m_bMMXDetected && !This.m_bDisableMMX ? TRUE: FALSE);
    }
    else
    {
        This.Render = SelectRenderMethod(This.m_bDouble,
                                         This.m_bVDouble,
                                         This.m_bHScanLines,
                                         This.m_bVScanLines,
                                         eDirtyMode,
                                         depth == 16 ? TRUE : FALSE,
                                         MAME32App.m_bMMXDetected && !This.m_bDisableMMX ? TRUE: FALSE);
    }

    /*
        Create the bitmap MAME uses for drawing.
        osd_create_bitmap will swap width and height if neccesary.
    */
    This.m_pMAMEBitmap = osd_new_bitmap(width, height, depth);

    if (Machine->orientation & ORIENTATION_SWAP_XY)
    {
        int temp;

        temp   = width;
        width  = height;
        height = temp;
    }

    /* width, height must be rotation adjusted. */
    InitDirty(width, height, eDirtyMode);

    /*
        Modify the main window to suit our needs.
    */
    SetWindowLong(MAME32App.m_hWnd, GWL_STYLE, WS_POPUP);
    SetWindowPos(MAME32App.m_hWnd,
                 HWND_TOPMOST,
                 0, 0,
                 GetSystemMetrics(SM_CXSCREEN)/4,
                 GetSystemMetrics(SM_CYSCREEN)/4,
                 0);

    ShowWindow(MAME32App.m_hWnd, SW_SHOW);
    UpdateWindow(MAME32App.m_hWnd);
    MAME32App.ProcessMessages();

    /*
        Get exclusive mode.
        "Windows does not support Mode X modes; therefore, when your application
        is in a Mode X mode, you cannot use the IDirectDrawSurface2::Lock or 
        IDirectDrawSurface2::Blt methods to lock or blit the primary surface."
    */
    hResult = IDirectDraw_SetCooperativeLevel(This.m_pDDraw,
                                              MAME32App.m_hWnd,
                                              /*DDSCL_ALLOWMODEX |*/
                                              DDSCL_EXCLUSIVE |
                                              DDSCL_FULLSCREEN);
    if (FAILED(hResult))
    {
        ErrorMsg("IDirectDraw.SetCooperativeLevel failed error=%x", hResult);
        goto error;
    }

    hResult = IDirectDraw_SetDisplayMode(This.m_pDDraw,
                                         This.m_nScreenWidth,
                                         This.m_nScreenHeight,
                                         depth);
    if (FAILED(hResult))
    {
        ErrorMsg("IDirectDraw.SetDisplayMode failed error=%x", hResult);
        goto error;
    }

    /*
        Create Surface(s).
    */
    if (This.m_bUseBackBuffer == FALSE)
    {
        /* Create the primary surface with no back buffers. */
        memset(&ddSurfaceDesc, 0, sizeof(ddSurfaceDesc));
        ddSurfaceDesc.dwSize         = sizeof(ddSurfaceDesc);
        ddSurfaceDesc.dwFlags        = DDSD_CAPS;
        ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        hResult = IDirectDraw_CreateSurface(This.m_pDDraw,
                                            &ddSurfaceDesc,
                                            &This.m_pDDSPrimary,
                                            NULL);

        if (FAILED(hResult))
        {
            ErrorMsg("IDirectDraw.CreateSurface failed! %x", hResult);
            goto error;
        }

        /*
            This will cause the update_display to write directly to
            the primary display surface.
        */
        This.m_pDDSBack = This.m_pDDSPrimary;

        /* If the surface can't be locked, clean up and use the backbuffer scheme. */
        if (SurfaceLockable(This.m_pDDSPrimary) == FALSE)
        {
            IDirectDraw_Release(This.m_pDDSPrimary);

            This.m_bUseBackBuffer = TRUE;
        }
    }

    if (This.m_bUseBackBuffer == TRUE)
    {
        /* Create the primary surface with 1 back buffer. */
        memset(&ddSurfaceDesc, 0, sizeof(ddSurfaceDesc));
        ddSurfaceDesc.dwSize            = sizeof(ddSurfaceDesc);
        ddSurfaceDesc.dwFlags           = DDSD_CAPS;
        ddSurfaceDesc.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE;

        hResult = IDirectDraw_CreateSurface(This.m_pDDraw,
                                            &ddSurfaceDesc,
                                            &This.m_pDDSPrimary,
                                            NULL);

        if (FAILED(hResult))
        {
            ErrorMsg("IDirectDraw.CreateSurface failed error=%x", hResult);
            goto error;
        }


        /* CMK 10/16/97 */
        /* Create a separate back buffer to blit with */
        
        ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

        if (This.m_bBltDouble == TRUE)
        {
            /* the backbuffer is now game sized when blitting 1/30/98 MR */
            ddSurfaceDesc.dwWidth  = width;
            ddSurfaceDesc.dwHeight = height;
        }
        else
        {
            /*
               this doesn't really need to be this big, but the code is setup
               to assume the backbuffer is screen sized, not game sized.  
               Until that's changed, this works.
            */
            ddSurfaceDesc.dwWidth  = This.m_nScreenWidth;
            ddSurfaceDesc.dwHeight = This.m_nScreenHeight;
        }
      
        ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        ddSurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        hResult = IDirectDraw2_CreateSurface(This.m_pDDraw,&ddSurfaceDesc,&This.m_pDDSBack,NULL);

        if (FAILED(hResult))
        {
            ErrorMsg("IDirectDrawSurface.CreateSurface back buffer failed error=%x", hResult);
            goto error;
        }

    }


    UpdateWindow(MAME32App.m_hWnd);

    return This.m_pMAMEBitmap;

error:
    DDraw_close_display();

    return NULL;
}

/*
    Set the actual display screen but don't allocate the screen bitmap.
*/
static int DDraw_set_display(int width, int height, int attributes)
{
    return 0; 
}

/*
     Shut down the display
*/
static void DDraw_close_display(void)
{
    osd_free_bitmap(This.m_pMAMEBitmap);
    This.m_pMAMEBitmap = NULL;
 
    if (This.m_pDDraw != NULL)
    {
        IDirectDraw_RestoreDisplayMode(This.m_pDDraw);
        IDirectDraw_SetCooperativeLevel(This.m_pDDraw, MAME32App.m_hWnd, DDSCL_NORMAL);
    }

    ReleaseDDrawObjects();

    ExitDirty();
}

/*
    palette is an array of 'totalcolors' R,G,B triplets. The function returns
    in *pens the pen values corresponding to the requested colors.
    If 'totalcolors' is 32768, 'palette' is ignored and the *pens array is filled
    with pen values corresponding to a 5-5-5 15-bit palette
*/
static void DDraw_allocate_colors(unsigned int totalcolors, const unsigned char *palette, unsigned short *pens)
{
    unsigned int    i;
    BOOL            bResult = TRUE;
    HRESULT         hResult = DD_OK;

    if (This.m_pMAMEBitmap->depth == 8)
    {
        /*
            Set the palette Entries.
        */

        if (totalcolors >= OSD_NUMPENS - 1)
        {
            int bestblack;
            int bestwhite;
            int bestblackscore;
            int bestwhitescore;

            bestblack = bestwhite = 0;
            bestblackscore = 3 * 255 * 255;
            bestwhitescore = 0;
            for (i = 0; i < totalcolors; i++)
            {
                int r, g, b, score;

                r = palette[3 * i];
                g = palette[3 * i + 1];
                b = palette[3 * i + 2];
                score = r * r + g * g + b * b;

                if (score < bestblackscore)
                {
                    bestblack      = i;
                    bestblackscore = score;
                }
                if (score > bestwhitescore)
                {
                    bestwhite      = i;
                    bestwhitescore = score;
                }
            }

            for (i = 0; i < totalcolors; i++)
                pens[i] = i;

            /* map black to pen 0, otherwise the screen border will not be black */
            pens[bestblack] = 0;
            pens[0] = bestblack;

            Machine->uifont->colortable[0] = pens[bestblack];
            Machine->uifont->colortable[1] = pens[bestwhite];
            Machine->uifont->colortable[2] = pens[bestwhite];
            Machine->uifont->colortable[3] = pens[bestblack];

            This.m_nUIPen = pens[bestwhite];
        }
        else
        {
            /*
                If we have free places, fill the palette starting from the end,
                so we don't touch color 0, which is better left black.
            */
            for (i = 0; i < totalcolors; i++)
                pens[i] = OSD_NUMPENS - 1 - i;

            /* As long as there are free pens, set 0 as black for GetBlackPen. */
            SetPen(0, 0, 0, 0);

            /* reserve color 1 for the user interface text */
            This.m_nUIPen = 1;
            SetPen(This.m_nUIPen, 0xFF, 0xFF, 0xFF);

            Machine->uifont->colortable[0] = 0;
            Machine->uifont->colortable[1] = This.m_nUIPen;
            Machine->uifont->colortable[2] = This.m_nUIPen;
            Machine->uifont->colortable[3] = 0;
        }

        for (i = 0; i < totalcolors; i++)
        {
            SetPen(pens[i],
                   palette[3 * i],
                   palette[3 * i + 1],
                   palette[3 * i + 2]);
        }

        /*
            Create the palette.
        */

        bResult = BuildPalette(This.m_pDDraw, &This.m_pDDPal);
        if (bResult == FALSE)
            return;

        hResult = IDirectDrawSurface_SetPalette(This.m_pDDSPrimary, This.m_pDDPal);
        if (FAILED(hResult))
        {
            ErrorMsg("IDirectDrawSurface.SetPalette failed error=%x", hResult);
            return;
        }
        
        This.m_bUpdatePalette = TRUE;
    }
    else
    {
        DDPIXELFORMAT   ddPixelFormat;
        int r, g, b;

        memset(&ddPixelFormat, 0, sizeof(DDPIXELFORMAT));
        ddPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddPixelFormat.dwFlags = DDPF_RGB;
        hResult = IDirectDrawSurface_GetPixelFormat(This.m_pDDSPrimary, &ddPixelFormat);
        if (FAILED(hResult))
        {
            ErrorMsg("IDirectDrawSurface.GetPixelFormat failed error=%x", hResult);
            return;
        }

        //printf("RedMask: 0x%08x - ",ddPixelFormat.dwRBitMask);
        GetPixelInfo(ddPixelFormat.dwRBitMask, &This.m_Red);
        
        //printf("GrnMask: 0x%08x - ",ddPixelFormat.dwGBitMask);
        GetPixelInfo(ddPixelFormat.dwGBitMask, &This.m_Green);

        //printf("BleMask: 0x%08x -",ddPixelFormat.dwBBitMask);
        GetPixelInfo(ddPixelFormat.dwBBitMask, &This.m_Blue);

        for (r = 0; r < 32; r++)
            for (g = 0; g < 32; g++)
                for (b = 0; b < 32; b++)
                {
                    int r1, g1, b1;

                    r1 = (int)(31.0 * Display_get_brightness() * pow(r / 31.0, 1.0 / Display_get_gamma()) / 100.0);
                    g1 = (int)(31.0 * Display_get_brightness() * pow(g / 31.0, 1.0 / Display_get_gamma()) / 100.0);
                    b1 = (int)(31.0 * Display_get_brightness() * pow(b / 31.0, 1.0 / Display_get_gamma()) / 100.0);

                    *pens++ = MAKECOL(r1, g1, b1);
                }

        Machine->uifont->colortable[0] = 0;
        Machine->uifont->colortable[1] = MAKECOL(0xFF, 0xFF, 0xFF);
        Machine->uifont->colortable[2] = MAKECOL(0xFF, 0xFF, 0xFF);
        Machine->uifont->colortable[3] = 0;

    }

    /* Find the black pen to use for background. It may not be zero. */
    This.m_nBlackPen = FindBlackPen();

    /* Clear buffers. */
    UpdateWindow(MAME32App.m_hWnd);
    ClearSurface(This.m_pDDSPrimary);
    ClearSurface(This.m_pDDSBack);
}

/*
    Change the color of the pen.
*/
static void DDraw_modify_pen(int pen, unsigned char red, unsigned char green, unsigned char blue)
{
    assert(This.m_pMAMEBitmap->depth == 8); /* Shouldn't modify pen in 16 bit color depth game mode! */

    assert(0 <= pen && pen < OSD_NUMPENS);

    if (This.m_PalEntries[pen].peRed    == red
    &&  This.m_PalEntries[pen].peGreen  == green
    &&  This.m_PalEntries[pen].peBlue   == blue)
        return;

    if (pen == This.m_nBlackPen)
        /* This.m_nBlackPen will be updated on update_display() */
        This.m_bUpdateBackground = TRUE;

    SetPen(pen, red, green, blue);

    This.m_bUpdatePalette = TRUE;
}

/*
    Get the color of a pen.
*/
static void DDraw_get_pen(int pen, unsigned char* pRed, unsigned char* pGreen, unsigned char* pBlue)
{
    if (This.m_pMAMEBitmap->depth == 8)
    {
        assert(0 <= pen && pen < OSD_NUMPENS);

        if (OSD_NUMPENS <= pen)
            pen = 0;

        *pRed   = This.m_PalEntries[pen].peRed;
        *pGreen = This.m_PalEntries[pen].peGreen;
        *pBlue  = This.m_PalEntries[pen].peBlue;
    }
    else
    {
        struct tPixelInfo * r = &This.m_Red;
        struct tPixelInfo * g = &This.m_Green;
        struct tPixelInfo * b = &This.m_Blue;

        *pRed   = ((pen & r->m_dwMask) >> r->m_nShift) << (8 - r->m_nSize);
        *pGreen = ((pen & g->m_dwMask) >> g->m_nShift) << (8 - g->m_nSize);
        *pBlue  = ((pen & b->m_dwMask) >> b->m_nShift) << (8 - b->m_nSize);
    }
}

static void DDraw_mark_dirty(int x1, int y1, int x2, int y2, int ui)
{
    MarkDirty(x1, y1, x2, y2, ui);
}

/*
    Update the display using This.m_pMAMEBitmap.
*/
static void DDraw_update_display(void)
{
    HRESULT     hResult;

    if (This.m_pDDSBack    == NULL
    ||  This.m_pDDSPrimary == NULL)
        return;

    if (This.m_bUpdateBackground == TRUE)
    {
        This.m_nBlackPen = FindBlackPen();
        ClearSurface(This.m_pDDSPrimary);
        osd_mark_dirty(0, 0,
                       Machine->scrbitmap->width  - 1,
                       Machine->scrbitmap->height - 1, 1);

        This.m_bUpdateBackground = FALSE;
    }

    if (This.m_bUpdatePalette == TRUE)
    {
        SetPaletteColors();
        This.m_bUpdatePalette = FALSE;
    }

    profiler_mark(PROFILER_BLIT);
    DrawSurface(This.m_pDDSBack);

    if (This.m_bUseBackBuffer == TRUE)
    {
        /* CMK 10/16/97 */
        RECT    rectSrc, rectDest;

        rectDest.top    = This.m_GameRect.m_Top;
        rectDest.left   = This.m_GameRect.m_Left;
        rectDest.bottom = This.m_GameRect.m_Height + This.m_GameRect.m_Top;
        rectDest.right  = This.m_GameRect.m_Width  + This.m_GameRect.m_Left;

        if (This.m_bBltDouble)
        {
            rectSrc.top    = This.m_nSkipLines;
            rectSrc.left   = This.m_nSkipColumns;
            rectSrc.bottom = This.m_nSkipLines   + This.m_nDisplayLines;
            rectSrc.right  = This.m_nSkipColumns + This.m_nDisplayColumns;
        }
        else
        {
            rectSrc = rectDest;
        }

        if (This.m_bStretchX)
        {
            rectDest.left   = 0;
            rectDest.right  = This.m_nScreenWidth;
        }
        if (This.m_bStretchY)
        {
            rectDest.top    = 0;
            rectDest.bottom = This.m_nScreenHeight;
        }

        while (1)
        {
            hResult = IDirectDrawSurface_Blt(This.m_pDDSPrimary,
                                             &rectDest,
                                             This.m_pDDSBack,
                                             &rectSrc,
                                             DDBLT_WAIT,
                                             NULL);
                
            if (hResult == DD_OK)
                break;

            if (hResult == DDERR_SURFACELOST)
            {
                hResult = IDirectDrawSurface_Restore(This.m_pDDSPrimary);
                if (FAILED(hResult))
                    break;
            }
            else
            if (hResult != DDERR_WASSTILLDRAWING)
            {
                ErrorMsg("IDirectDrawSurface.Blt failed error=%x", hResult);
                break;
            }
        }
    }

    profiler_mark(PROFILER_END);

    /* Check for PGUP, PGDN and pan screen */
    if (This.m_bPanScreen == TRUE)
    {
        if (keyboard_pressed(KEYCODE_PGDN) || keyboard_pressed(KEYCODE_PGUP))
            PanDisplay();
    }
}

/* control keyboard leds or other indicators */
static void DDraw_led_w(int led, int on)
{
    LED_write(led, on & 1);
}

static void DDraw_set_gamma(float gamma)
{
    assert(OSDDisplay.set_gamma != 0);

    OSDDisplay.set_gamma(gamma);

    AdjustPalette();
}

static void DDraw_set_brightness(int brightness)
{
    assert(OSDDisplay.set_brightness != 0);

    OSDDisplay.set_brightness(brightness);

    AdjustPalette();
}

static void DDraw_save_snapshot()
{
    if (This.m_bAviCapture)
    {
        This.m_nAviShowMessage = 10;        /* show message for 10 frames */
		This.m_bAviRun = !This.m_bAviRun;   /* toggle capture on/off */
	}
    else
        Display_WriteBitmap(This.m_pMAMEBitmap, This.m_PalEntries);
}

/***************************************************************************
    External functions
 ***************************************************************************/

/***************************************************************************
    Internal functions
 ***************************************************************************/

/*
    Draw the entire MAMEBitmap on the DirectDraw surface;
*/
static void DrawSurface(IDirectDrawSurface* pddSurface)
{
    BYTE*           pbScreen;
    HRESULT         hResult;
    DDSURFACEDESC   ddSurfaceDesc;

    if (pddSurface == NULL)
        return;

    ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);

    while (1)
    {
        hResult = IDirectDrawSurface_Lock(pddSurface, NULL, &ddSurfaceDesc, DDLOCK_WAIT, NULL);

        if (hResult == DD_OK)
            break;

        if (hResult == DDERR_SURFACELOST)
        {
            hResult = IDirectDrawSurface_Restore(pddSurface);
            if (FAILED(hResult))
                return;
        }
        else
        if (hResult != DDERR_WASSTILLDRAWING)
        {
            ErrorMsg("IDirectDrawSurface.Lock failed error=%x", hResult);
            return;
        }
    }

    if (This.m_bBltDouble == TRUE)
    {
        if (This.m_pMAMEBitmap->depth == 8)
        {
            pbScreen = &((BYTE*)(ddSurfaceDesc.lpSurface))
                        [This.m_nSkipLines * ddSurfaceDesc.lPitch +
                         This.m_nSkipColumns];
        }
        else
        {
            pbScreen = (BYTE*)&((unsigned short*)(ddSurfaceDesc.lpSurface))
                        [This.m_nSkipLines * ddSurfaceDesc.lPitch / 2 +
                         This.m_nSkipColumns];
        }
    }
    else
    {
        if (This.m_pMAMEBitmap->depth == 8)
        {
            pbScreen = &((BYTE*)(ddSurfaceDesc.lpSurface))
                        [This.m_GameRect.m_Top * ddSurfaceDesc.lPitch +
                         This.m_GameRect.m_Left];
        }
        else
        {
            pbScreen = (BYTE*)&((unsigned short*)(ddSurfaceDesc.lpSurface))
                        [This.m_GameRect.m_Top * ddSurfaceDesc.lPitch / 2 +
                         This.m_GameRect.m_Left];
        }
    }

    if (This.m_bAviRun) /* add avi frame */
        AviAddBitmap(This.m_pMAMEBitmap, This.m_PalEntries);
    
    if (This.m_nAviShowMessage > 0)
    {
        char    buf[32];
        
        This.m_nAviShowMessage--;
               
        sprintf(buf, "AVI Capture %s", (This.m_bAviRun) ? "ON" : "OFF");
        ui_text(buf, Machine->uiwidth - strlen(buf) * Machine->uifontwidth, 0);        
    }

    This.Render(This.m_pMAMEBitmap,
                This.m_nSkipLines,
                This.m_nSkipColumns,
                This.m_nDisplayLines,
                This.m_nDisplayColumns,
                pbScreen,
                ddSurfaceDesc.lPitch);

    hResult = IDirectDrawSurface_Unlock(pddSurface, NULL);
    if (FAILED(hResult))
    {
        ErrorMsg("IDirectDrawSurface.Unlock failed error=%x", hResult);
        return;
    }
}

/*
    Set the entire DirectDraw surface to black;
*/
static void ClearSurface(IDirectDrawSurface* pddSurface)
{
    HRESULT     hResult;
    DDBLTFX     ddBltFX;

    memset(&ddBltFX, 0, sizeof(DDBLTFX));
    ddBltFX.dwSize      = sizeof(DDBLTFX);
    ddBltFX.dwFillColor = This.m_nBlackPen;

    while (1)
    {
        hResult = IDirectDrawSurface_Blt(pddSurface,
                                         NULL,
                                         NULL,
                                         NULL,
                                         DDBLT_WAIT | DDBLT_COLORFILL,
                                         &ddBltFX);
            
        if (hResult == DD_OK)
            break;

        if (hResult == DDERR_SURFACELOST)
        {
            hResult = IDirectDrawSurface_Restore(pddSurface);
            if (FAILED(hResult))
                break;
        }
        else
        if (hResult != DDERR_WASSTILLDRAWING)
        {
            break;
        }
    }

    return;
}

/*
    Build a palette.
*/
static BOOL BuildPalette(IDirectDraw* pDDraw, IDirectDrawPalette** ppDDPalette)
{
    int             i;
    HRESULT         hResult;
    PALETTEENTRY    PaletteEntries[OSD_NUMPENS];

    for (i = 0; i < OSD_NUMPENS; i++)
    {
        PaletteEntries[i].peRed   = 0;
        PaletteEntries[i].peGreen = 0;
        PaletteEntries[i].peBlue  = 0;
        PaletteEntries[i].peFlags = (BYTE)PC_NOCOLLAPSE;
    }

    hResult = IDirectDraw_CreatePalette(pDDraw,
                                        DDPCAPS_8BIT | DDPCAPS_ALLOW256,
                                        PaletteEntries,
                                        ppDDPalette,
                                        NULL);

    if (*ppDDPalette == NULL || FAILED(hResult))
    {
        ErrorMsg("IDirectDraw.CreatePalette failed error=%x", hResult);
        return FALSE;
    }
    return TRUE;
}

/*
    Finished with all DirectDraw objects we use; release them.
*/
static void ReleaseDDrawObjects(void)
{
    if (This.m_pDDraw != NULL)
    {
        if (This.m_pDDPal != NULL)
        {
            IDirectDrawPalette_Release(This.m_pDDPal);
            This.m_pDDPal = NULL;
        }

        if (This.m_pDDSPrimary != NULL)
        {
            IDirectDrawSurface_Release(This.m_pDDSPrimary);
            This.m_pDDSPrimary = NULL;
            This.m_pDDSBack    = NULL;
        }

        This.m_pDDraw = NULL;
    }
}

static void SetPen(int pen, unsigned char red, unsigned char green, unsigned char blue)
{
    assert(0 <= pen && pen < OSD_NUMPENS);

    This.m_PalEntries[pen].peRed    = red;
    This.m_PalEntries[pen].peGreen  = green;
    This.m_PalEntries[pen].peBlue   = blue;
    This.m_PalEntries[pen].peFlags  = PC_NOCOLLAPSE;

    memcpy(&This.m_AdjustedPalette[pen], &This.m_PalEntries[pen], sizeof(PALETTEENTRY));
    Display_MapColor(&This.m_AdjustedPalette[pen].peRed,
                     &This.m_AdjustedPalette[pen].peGreen,
                     &This.m_AdjustedPalette[pen].peBlue);
}

static void SetPaletteColors()
{
    if (This.m_pMAMEBitmap->depth == 8)
    {
        HRESULT     hResult;

        assert(This.m_pDDPal != NULL);

        hResult = IDirectDrawPalette_SetEntries(This.m_pDDPal,
                                                0,
                                                0,
                                                OSD_NUMPENS,
                                                This.m_AdjustedPalette);
        if (FAILED(hResult))
        {
            ErrorMsg("IDirectDrawPalette.SetEntries failed error=%x", hResult);
        }
    }
}

static int FindBlackPen(void)
{    
    if (This.m_pMAMEBitmap->depth == 8)
    {
        int i;

        for (i = 0; i < OSD_NUMPENS; i++)
        {
            if (This.m_PalEntries[i].peRed   == 0
            &&  This.m_PalEntries[i].peGreen == 0
            &&  This.m_PalEntries[i].peBlue  == 0)
            {
                return i;
            }
        }
    }

    return 0;
}

static BOOL SurfaceLockable(IDirectDrawSurface* pddSurface)
{
    HRESULT         hResult;
    DDSURFACEDESC   ddSurfaceDesc;

    if (pddSurface == NULL)
        return FALSE;

    memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
    ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);

    while (1)
    {
        hResult = IDirectDrawSurface_Lock(pddSurface, NULL, &ddSurfaceDesc, DDLOCK_WAIT, NULL);

        if (hResult == DD_OK)
        {
            hResult = IDirectDrawSurface_Unlock(pddSurface, NULL);
            return TRUE;
        }

        /*
            "Access to this surface is being refused because no driver exists
            which can supply a pointer to the surface.
            This is most likely to happen when attempting to lock the primary
            surface when no DCI provider is present.
            Will also happen on attempts to lock an optimized surface."
        */
        if (hResult == DDERR_CANTLOCKSURFACE)
        {
            return FALSE;
        }

        if (hResult == DDERR_SURFACELOST)
        {
            hResult = IDirectDrawSurface_Restore(pddSurface);
            if (FAILED(hResult))
                return FALSE;
        }
        else
        if (hResult != DDERR_WASSTILLDRAWING)
        {
            ErrorMsg("IDirectDrawSurface.Lock failed error=%x", hResult);
            return FALSE;
        }
    }
}

static void GetPixelInfo(DWORD dwMask, struct tPixelInfo* pPixelInfo)
{
    int nShift = 0;
    int nCount = 0;
    int i;

    pPixelInfo->m_dwMask = dwMask;

    while ((dwMask & 1) == 0 && nShift < sizeof(DWORD) * 8)
    {
        nShift++;
        dwMask >>= 1;
    }
    pPixelInfo->m_nShift = nShift;

    for (i = 0; i < sizeof(DWORD) * 8; i++)
    {
        if (dwMask & (1 << i))
            nCount++;
    }
    pPixelInfo->m_nSize = nCount;

    //printf("Shift (%d), Size(%d)\n", pPixelInfo->m_nShift, pPixelInfo->m_nSize);

}

/*
    FindBestDisplayMode will search through the available display modes
    and select the mode which is just large enough for the input dimentions.
    If no display modes are large enough, the largest mode available is returned.
*/
static BOOL FindBestDisplayMode(DWORD  dwWidthIn,   DWORD  dwHeightIn, DWORD dwDepth,
                                DWORD* pdwWidthOut, DWORD* pdwHeightOut)
{
    struct tDisplayModes*   pDisplayModes;
    int     i;
    BOOL    bFound = FALSE;
    DWORD   dwBestWidth  = 10000;
    DWORD   dwBestHeight = 10000;
    DWORD   dwBiggestWidth  = 0;
    DWORD   dwBiggestHeight = 0;

    pDisplayModes = DirectDraw_GetDisplayModes();

    assert(0 < pDisplayModes->m_nNumModes);

    for (i = 0; i < pDisplayModes->m_nNumModes; i++)
    {
        if (dwDepth != pDisplayModes->m_Modes[i].m_dwBPP)
            continue;

        /* Find a mode big enough for input size. */
        if (dwWidthIn  <= pDisplayModes->m_Modes[i].m_dwWidth
        &&  dwHeightIn <= pDisplayModes->m_Modes[i].m_dwHeight)
        {
            /* Get the smallest display size. */
            if (pDisplayModes->m_Modes[i].m_dwWidth  < dwBestWidth
            ||  pDisplayModes->m_Modes[i].m_dwHeight < dwBestHeight)
            {
                dwBestWidth  = pDisplayModes->m_Modes[i].m_dwWidth;
                dwBestHeight = pDisplayModes->m_Modes[i].m_dwHeight;
                bFound = TRUE;
            }
        }

        /*
            Keep track of the biggest size in case
            we can't find a mode that works.
        */
        if (dwBiggestWidth  < pDisplayModes->m_Modes[i].m_dwWidth
        ||  dwBiggestHeight < pDisplayModes->m_Modes[i].m_dwHeight)
        {
            dwBiggestWidth  = pDisplayModes->m_Modes[i].m_dwWidth;
            dwBiggestHeight = pDisplayModes->m_Modes[i].m_dwHeight;
        }
    }

    assert(pdwWidthOut  != NULL);
    assert(pdwHeightOut != NULL);

    if (bFound == TRUE)
    {
        *pdwWidthOut  = dwBestWidth;
        *pdwHeightOut = dwBestHeight;
    }
    else
    {
        *pdwWidthOut  = dwBiggestWidth;
        *pdwHeightOut = dwBiggestHeight;
    }

    return bFound;
}

static BOOL FindDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwDepth)
{
    int     i;
    BOOL    bModeFound = FALSE;
    struct tDisplayModes*   pDisplayModes;

    pDisplayModes = DirectDraw_GetDisplayModes();

    for (i = 0; i < pDisplayModes->m_nNumModes; i++)
    {
        if (pDisplayModes->m_Modes[i].m_dwWidth  == dwWidth
        &&  pDisplayModes->m_Modes[i].m_dwHeight == dwHeight
        &&  pDisplayModes->m_Modes[i].m_dwBPP    == dwDepth)
        {
            bModeFound = TRUE;
            break;
        }
    }

    return bModeFound;
}

/*
    This function tries to find the best display mode.
*/
static void SelectDisplayMode(int depth)
{
    int width, height;

    if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
    {
        width  = Machine->drv->screen_width;
        height = Machine->drv->screen_height;
    }
    else
    {
        width  = Machine->drv->visible_area.max_x - Machine->drv->visible_area.min_x + 1;
        height = Machine->drv->visible_area.max_y - Machine->drv->visible_area.min_y + 1;
    }

    if (Machine->orientation & ORIENTATION_SWAP_XY)
    {
        int temp;

        temp   = width;
        width  = height;
        height = temp;
    }

    if (This.m_bBestFit == TRUE)
    {
        /* vector games use 640x480 as default */
        if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
        {
            This.m_nScreenWidth  = 640;
            This.m_nScreenHeight = 480;
        }
        else
        {
            BOOL    bResult;

            if (This.m_bVDouble == TRUE)
            {
                height *= 2;
            }
            if (This.m_bDouble == TRUE)
            {
                width  *= 2;
                height *= 2;
            }

            if (This.m_bDouble == TRUE)
            {
                bResult = FindBestDisplayMode(width, height, depth,
                                              &This.m_nScreenWidth,
                                              &This.m_nScreenHeight);

                if (bResult == FALSE)
                {
                    /* no pixel doubled modes fit, revert to not doubled */
                    This.m_bDouble = FALSE;
                    width  /= 2;
                    height /= 2;
                }
            }

            if (This.m_bDouble == FALSE)
            {
                FindBestDisplayMode(width, height, depth,
                                    &This.m_nScreenWidth,
                                    &This.m_nScreenHeight);
            }
        }
    }
}

static void ScaleVectorGames(int gfx_width, int gfx_height, int *width, int *height)
{
    double x_scale, y_scale, scale;

    if (Machine->orientation & ORIENTATION_SWAP_XY)
    {
        x_scale = (double)gfx_width  / (double)(*height);
        y_scale = (double)gfx_height / (double)(*width);
    }
    else
    {
        x_scale = (double)gfx_width  / (double)(*width);
        y_scale = (double)gfx_height / (double)(*height);
    }

    if (x_scale < y_scale)
        scale = x_scale;
    else
        scale = y_scale;

    *width  = (int)((double)*width  * scale);
    *height = (int)((double)*height * scale);

    /* Padding to a dword value. */
    *width  -= *width  % 4;
    *height -= *height % 4;
}

/* Center image inside the display based on the visual area. */
static void AdjustDisplay(int xmin, int ymin, int xmax, int ymax)
{
    int temp;
    int w, h;
    int gfx_xoffset;
    int gfx_yoffset;
    int width_factor=1;
    int xpad = 0; /* # of pad pixels for 8 pixel granularity */
    UINT viswidth;
    UINT visheight;

    if (Machine->orientation & ORIENTATION_SWAP_XY)
    {
        temp = xmin; xmin = ymin; ymin = temp;
        temp = xmax; xmax = ymax; ymax = temp;
        w = Machine->drv->screen_height;
        h = Machine->drv->screen_width;
    }
    else
    {
        w = Machine->drv->screen_width;
        h = Machine->drv->screen_height;
    }

    if (!(Machine->drv->video_attributes & VIDEO_TYPE_VECTOR))
    {
        if (Machine->orientation & ORIENTATION_FLIP_X)
        {
            temp = w - xmin - 1;
            xmin = w - xmax - 1;
            xmax = temp;
        }
        if (Machine->orientation & ORIENTATION_FLIP_Y)
        {
            temp = h - ymin - 1;
            ymin = h - ymax - 1;
            ymax = temp;
        }
    }


    viswidth  = xmax - xmin + 1;
    visheight = ymax - ymin + 1;


    if (!(Machine->drv->video_attributes & VIDEO_TYPE_VECTOR))
    {
        if (This.m_bDouble == TRUE)
            width_factor = 2;

        if (viswidth * width_factor < This.m_nScreenWidth)
        {
            UINT temp_width;
            int  temp_xmin;
            int  temp_xmax;

            temp_xmin  = xmin & (~0x7); /* quad align, 1 lower */
            temp_width = xmax - temp_xmin + 1;
            if (temp_width * width_factor <= This.m_nScreenWidth)
            {
                xpad = (xmax - temp_xmin + 1) - viswidth;
                viswidth += xpad;
                if (temp_width & 0x7)
                {
                    if (((temp_width + 7) & ~0x7 ) * width_factor <= This.m_nScreenWidth)
                    {
                        temp_xmax = xmax + 8 - (temp_width & 0x7);
                        xpad = (temp_xmax - temp_xmin + 1) - viswidth;
                        viswidth += xpad;
                    }
                }
            }
        }
    }

    This.m_nDisplayLines   = visheight;
    This.m_nDisplayColumns = viswidth;
    if (This.m_bDouble == TRUE)
    {
        gfx_xoffset = (int)(This.m_nScreenWidth - viswidth * 2) / 2;
        if (This.m_nDisplayColumns > This.m_nScreenWidth / 2)
            This.m_nDisplayColumns = This.m_nScreenWidth / 2;
    }
    else
    {
        gfx_xoffset = (int)(This.m_nScreenWidth - viswidth)  / 2;
        if (This.m_nDisplayColumns > This.m_nScreenWidth)
            This.m_nDisplayColumns = This.m_nScreenWidth;
    }

    if (This.m_bDouble == TRUE && This.m_bVDouble == TRUE)
    {
        gfx_yoffset = (int)(This.m_nScreenHeight - visheight * 4) / 2;
        if (This.m_nDisplayLines > This.m_nScreenHeight / 4)
            This.m_nDisplayLines = This.m_nScreenHeight / 4;
    }
    else
    if (This.m_bDouble == TRUE || This.m_bVDouble == TRUE)
    {
        gfx_yoffset = (int)(This.m_nScreenHeight - visheight * 2) / 2;
        if (This.m_nDisplayLines > This.m_nScreenHeight / 2)
            This.m_nDisplayLines = This.m_nScreenHeight / 2;
    }
    else
    {
        gfx_yoffset = (int)(This.m_nScreenHeight - visheight) / 2;
        if (This.m_nDisplayLines > This.m_nScreenHeight)
            This.m_nDisplayLines = This.m_nScreenHeight;
    }


    This.m_nSkipLinesMin   = ymin;
    This.m_nSkipLinesMax   = visheight - This.m_nDisplayLines   + ymin;
    This.m_nSkipColumnsMin = xmin;
    This.m_nSkipColumnsMax = viswidth  - This.m_nDisplayColumns + xmin;

    /* Align on a quadword !*/
    gfx_xoffset &= ~7;

    /* skipcolumns is relative to the visible area. */
    This.m_nSkipColumns = xmin + This.m_nSkipColumns;
    This.m_nSkipLines   = ymin + This.m_nSkipLines;

    /* Failsafe against silly parameters */
    if (This.m_nSkipLines < This.m_nSkipLinesMin)
        This.m_nSkipLines = This.m_nSkipLinesMin;
    if (This.m_nSkipColumns < This.m_nSkipColumnsMin)
        This.m_nSkipColumns = This.m_nSkipColumnsMin;
    if (This.m_nSkipLines > This.m_nSkipLinesMax)
        This.m_nSkipLines = This.m_nSkipLinesMax;
    if (This.m_nSkipColumns > This.m_nSkipColumnsMax)
        This.m_nSkipColumns = This.m_nSkipColumnsMax;

    /* Just in case the visual area doesn't fit */
    if (gfx_xoffset < 0)
    {
        gfx_xoffset = 0;
    }
    if (gfx_yoffset < 0)
    {
        gfx_yoffset = 0;
    }

    This.m_GameRect.m_Left   = gfx_xoffset;
    This.m_GameRect.m_Top    = gfx_yoffset;
    This.m_GameRect.m_Width  = This.m_nDisplayColumns;
    This.m_GameRect.m_Height = This.m_nDisplayLines;

    if (This.m_bVDouble == TRUE)
    {
        This.m_GameRect.m_Height *= 2;
    }
    if (This.m_bDouble == TRUE)
    {
        This.m_GameRect.m_Width  *= 2;
        This.m_GameRect.m_Height *= 2;
    }
        
    /* Figure out if we need to check for PGUP and PGDN for panning. */
    if (This.m_nDisplayColumns < viswidth
    ||  This.m_nDisplayLines   < visheight) 
        This.m_bPanScreen = TRUE;

    set_ui_visarea(This.m_nSkipColumns, This.m_nSkipLines,
                   This.m_nSkipColumns + This.m_nDisplayColumns - 1 - xpad,
                   This.m_nSkipLines   + This.m_nDisplayLines - 1);

/*
    ErrorMsg("Screen width = %d, height = %d\n"
             "Game xoffset = %d, yoffset  = %d\n"
             "xmin %d xmax %d ymin %d ymax %d\n"
             "viswidth %d visheight %d\n"
             "Skipcolumns Min %d, Max %d\n"
             "Skiplines Min %d, Max%d\n"
             "Skip columns %d, lines %d\n"
             "Display columns %d, lines %d\n",
             This.m_nScreenWidth,
             This.m_nScreenHeight,
             gfx_xoffset, gfx_yoffset,
             xmin, xmax, ymin, ymax,
             viswidth, visheight,
             This.m_nSkipColumnsMin, This.m_nSkipColumnsMax,
             This.m_nSkipLinesMin, This.m_nSkipLinesMax,
             This.m_nSkipColumns, This.m_nSkipLines,
             This.m_nDisplayColumns,
             This.m_nDisplayLines);
*/
}

static void PanDisplay(void)
{
    BOOL bMarkDirty = FALSE;

    /* Horizontal panning. */
    if (keyboard_pressed(KEYCODE_LSHIFT))
    {
        if (keyboard_pressed(KEYCODE_PGUP))
        {
            if (This.m_nSkipColumns < This.m_nSkipColumnsMax)
            {
                This.m_nSkipColumns++;
                bMarkDirty = TRUE;
            }
        }
        if (keyboard_pressed(KEYCODE_PGDN))
        {
            if (This.m_nSkipColumns > This.m_nSkipColumnsMin)
            {
                This.m_nSkipColumns--;
                bMarkDirty = TRUE;
            }
        }
    }
    else /* Vertical panning. */
    {
        if (keyboard_pressed(KEYCODE_PGDN))
        {
            if (This.m_nSkipLines < This.m_nSkipLinesMax)
            {
                This.m_nSkipLines++;
                bMarkDirty = TRUE;
            }

        }
        if (keyboard_pressed(KEYCODE_PGUP))
        {
            if (This.m_nSkipLines > This.m_nSkipLinesMin)
            {
                This.m_nSkipLines--;
                bMarkDirty = TRUE;
            }
        }
    }

    if (bMarkDirty == TRUE)
    {
        osd_mark_dirty(0, 0,
                       Machine->scrbitmap->width  - 1,
                       Machine->scrbitmap->height - 1, 1);
    }

    set_ui_visarea(This.m_nSkipColumns, This.m_nSkipLines,
                   This.m_nDisplayColumns + This.m_nSkipColumns - 1,
                   This.m_nDisplayLines   + This.m_nSkipLines   - 1);
}

static BOOL DDraw_OnMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    switch (Msg)
    {
        HANDLE_MESSAGE(hWnd, WM_SETCURSOR,  OnSetCursor);
    }
    return FALSE;
}

static BOOL OnSetCursor(HWND hWnd, HWND hWndCursor, UINT codeHitTest, UINT msg)
{
    /* Remove the cursor only after display is setup. */
    if (This.m_pMAMEBitmap != NULL)
        SetCursor(NULL);
    return TRUE;
}

static void DDraw_Refresh()
{
    This.m_bUpdateBackground = TRUE;

    DDraw_update_display();
}

static int DDraw_GetBlackPen(void)
{
    return This.m_nBlackPen;
}

static void DDraw_UpdateFPS(BOOL bShow, int nSpeed, int nFPS, int nMachineFPS, int nFrameskip, int nVecUPS)
{
    char buf[64];

    if (bShow)
    {
        sprintf(buf, "fskp%2d%4d%%%4d/%d fps", nFrameskip, nSpeed, nFPS, nMachineFPS);
        ui_text(buf, Machine->uiwidth - strlen(buf) * Machine->uifontwidth, 0);

        if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
        {
            sprintf(buf, " %d vector updates", nVecUPS);
            ui_text(buf,
                    Machine->uiwidth - strlen(buf) * Machine->uifontwidth,
                    Machine->uifontheight);
        }
    }
}

static void AdjustPalette(void)
{
    if (This.m_pMAMEBitmap->depth == 8)
    {
        UINT i;
        
        memcpy(This.m_AdjustedPalette, This.m_PalEntries, OSD_NUMPENS * sizeof(PALETTEENTRY));

        for (i = 0; i < OSD_NUMPENS; i++)
        {
            if (i != This.m_nUIPen) /* Don't map the UI pen. */
                Display_MapColor(&This.m_AdjustedPalette[i].peRed,
                                 &This.m_AdjustedPalette[i].peGreen,
                                 &This.m_AdjustedPalette[i].peBlue);
        }

        This.m_bUpdatePalette = TRUE;
    }
}
