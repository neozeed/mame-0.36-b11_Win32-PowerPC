/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  GDIDisplay.c

 ***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <math.h>
#include "MAME32.h"
#include "resource.h"
#include "driver.h"
#include "GDIDisplay.h"
#include "M32Util.h"
#include "status.h"
#include "dirty.h"
#include "avi.h"

#define MAKECOL(r, g, b) (((r) & 0x001F) << 10 | ((g) & 0x001F) << 5 | ((b) & 0x001F))

/***************************************************************************
    function prototypes
 ***************************************************************************/

static void             OnCommand(HWND hWnd, int CmdID, HWND hWndCtl, UINT codeNotify);
static void             OnPaint(HWND hWnd);
static void             OnPaletteChanged(HWND hWnd, HWND hWndPaletteChange);
static BOOL             OnQueryNewPalette(HWND hWnd);
static void             OnGetMinMaxInfo(HWND hWnd, MINMAXINFO* pMinMaxInfo);
static BOOL             OnEraseBkgnd(HWND hWnd, HDC hDC);
static void             OnSize(HWND hwnd,UINT state,int cx,int cy);
static void             OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItem);
static void             SetPen(int pen, unsigned char red, unsigned char green, unsigned char blue);
static void             SetPaletteColors(void);
static LRESULT CALLBACK AboutDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
static void             AdjustVisibleRect(int xmin, int ymin, int xmax, int ymax);
static void             AdjustPalette(void);

static int                GDI_init(options_type *options);
static void               GDI_exit(void);
static struct osd_bitmap* GDI_new_bitmap(int width, int height,int depth);
static void               GDI_clearbitmap(struct osd_bitmap* bitmap);
static void               GDI_free_bitmap(struct osd_bitmap* bitmap);
static struct osd_bitmap* GDI_create_display(int width, int height, int depth, int attributes);
static int                GDI_set_display(int width, int height, int attributes);
static void               GDI_close_display(void);
static void               GDI_allocate_colors(unsigned int totalcolors, const unsigned char *palette, unsigned short *pens);
static void               GDI_modify_pen(int pen, unsigned char red, unsigned char green, unsigned char blue);
static void               GDI_get_pen(int pen, unsigned char* pRed, unsigned char* pGreen, unsigned char* pBlue);
static void               GDI_mark_dirty(int x1, int y1, int x2, int y2, int ui);
static void               GDI_update_display(void);
static void               GDI_led_w(int led, int on);
static void               GDI_set_gamma(float gamma);
static void               GDI_set_brightness(int brightness);
static void               GDI_save_snapshot(void);
static BOOL               GDI_OnMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
static void               GDI_Refresh(void);
static int                GDI_GetBlackPen(void);
static void               GDI_UpdateFPS(BOOL bShow, int nSpeed, int nFPS, int nMachineFPS, int nFrameskip, int nVecUPS);

/***************************************************************************
    External variables
 ***************************************************************************/

struct OSDDisplay   GDIDisplay = 
{
    { GDI_init },               /* init              */
    { GDI_exit },               /* exit              */
    { GDI_new_bitmap },         /* new_bitmap        */
    { GDI_clearbitmap },        /* clearbitmap       */
    { GDI_free_bitmap },        /* free_bitmap       */
    { GDI_create_display },     /* create_display    */
    { GDI_set_display },        /* set_display       */
    { GDI_close_display },      /* close_display     */
    { GDI_allocate_colors },    /* allocate_colors   */
    { GDI_modify_pen },         /* modify_pen        */
    { GDI_get_pen },            /* get_pen           */
    { GDI_mark_dirty },         /* mark_dirty        */
    { 0 },                      /* skip_this_frame   */
    { GDI_update_display },     /* update_display    */
    { GDI_led_w },              /* led_w             */
    { GDI_set_gamma },          /* set_gamma         */
    { Display_get_gamma },      /* get_gamma         */
    { GDI_set_brightness },     /* set_brightness    */
    { Display_get_brightness }, /* get_brightness    */
    { GDI_save_snapshot },      /* save_snapshot     */

    { GDI_OnMessage },          /* OnMessage         */
    { GDI_Refresh },            /* Refresh           */
    { GDI_GetBlackPen },        /* GetBlackPen       */
    { GDI_UpdateFPS },          /* UpdateFPS         */
};

/***************************************************************************
    Internal structures
 ***************************************************************************/

struct tDisplay_private
{
    struct osd_bitmap*  m_pMAMEBitmap;
    BITMAPINFO*         m_pInfo;

    tRect               m_VisibleRect;
    int                 m_nClientWidth;
    int                 m_nClientHeight;
    RECT                m_ClientRect;
    int                 m_nWindowWidth;
    int                 m_nWindowHeight;

    BOOL                m_bScanlines;
    BOOL                m_bDouble;
    BOOL                m_bVectorDouble;
    BOOL                m_bUseDirty;
    int                 m_nDepth;

    /* AVI Capture params */
    BOOL                m_bAviCapture;      /* Capture AVI mode */
    BOOL                m_bAviRun;          /* Capturing frames */
    int                 m_nAviShowMessage;  /* Show status if !0 */

    BOOL                m_bUpdatePalette;
    HPALETTE            m_hPalette;
    PALETTEENTRY        m_PalEntries[OSD_NUMPENS];
    PALETTEENTRY        m_AdjustedPalette[OSD_NUMPENS];
    UINT                m_nUIPen;

    HWND                m_hWndAbout;
};

/***************************************************************************
    Internal variables
 ***************************************************************************/

static struct tDisplay_private      This;

/***************************************************************************
    External OSD functions  
 ***************************************************************************/

/*
    put here anything you need to do when the program is started. Return 0 if 
    initialization was successful, nonzero otherwise.
*/
static int GDI_init(options_type *options)
{
    OSDDisplay.init(options);

    This.m_pMAMEBitmap      = NULL;
    This.m_pInfo            = NULL;

    memset(&This.m_VisibleRect, 0, sizeof(tRect));
    This.m_nClientWidth     = 0;
    This.m_nClientHeight    = 0;
    This.m_nWindowWidth     = 0;
    This.m_nWindowHeight    = 0;

    This.m_bScanlines       = options->hscan_lines;
    This.m_bDouble          = (options->scale == 2);
    This.m_bVectorDouble    = options->double_vector;
    This.m_bUseDirty        = FALSE; /* options->use_dirty; */
    This.m_nDepth           = Machine->color_depth;

    This.m_bAviCapture      = 0;	//GetAviCapture();
    This.m_bAviRun          = FALSE;
    This.m_nAviShowMessage  = (This.m_bAviCapture) ? 10: 0;

    This.m_bUpdatePalette   = FALSE;
    This.m_hPalette         = NULL;
    This.m_hWndAbout        = NULL;
    memset(This.m_PalEntries,      0, OSD_NUMPENS * sizeof(PALETTEENTRY));
    memset(This.m_AdjustedPalette, 0, OSD_NUMPENS * sizeof(PALETTEENTRY));
    This.m_nUIPen           = 0;

    /* Scanlines don't work. */
    This.m_bScanlines = FALSE;

    /* Check for inconsistent parameters. */
    if (This.m_bScanlines == TRUE
    &&  This.m_bDouble    == FALSE)
    {
        This.m_bScanlines = FALSE;
    }

    return 0;
}

/*
    put here cleanup routines to be executed when the program is terminated.
*/
static void GDI_exit(void)
{
    OSDDisplay.exit();
}

static struct osd_bitmap* GDI_new_bitmap(int width, int height,int depth)
{
    assert(OSDDisplay.new_bitmap != 0);

    return OSDDisplay.new_bitmap(width, height, depth);
}

static void GDI_clearbitmap(struct osd_bitmap* bitmap)
{
    assert(OSDDisplay.clearbitmap != 0);

    OSDDisplay.clearbitmap(bitmap);
}

static void GDI_free_bitmap(struct osd_bitmap* pBitmap)
{
    assert(OSDDisplay.free_bitmap != 0);

    OSDDisplay.free_bitmap(pBitmap);
}

/*
    Create a display screen, or window, large enough to accomodate a bitmap
    of the given dimensions.
    Return a osd_bitmap pointer or 0 in case of error.
*/
static struct osd_bitmap *GDI_create_display(int width, int height, int depth, int attributes)
{
    unsigned int    i;
    RECT            Rect;
    HMENU           hMenu;
    LOGPALETTE      LogPalette;
    char            TitleBuf[256];

    This.m_nDepth = depth;

    if (attributes & VIDEO_TYPE_VECTOR)
    {
        if (This.m_bVectorDouble == TRUE)
        {
            width  *= 2;
            height *= 2;
        }
        /* padding to a DWORD value */
        width  -= width  % 4;
        height -= height % 4;

        AdjustVisibleRect(0, 0, width - 1, height - 1);
    }
    else
    {
        AdjustVisibleRect(Machine->drv->visible_area.min_x,
                          Machine->drv->visible_area.min_y,
                          Machine->drv->visible_area.max_x,
                          Machine->drv->visible_area.max_y);
    }

    This.m_nClientWidth  = This.m_VisibleRect.m_Width;
    This.m_nClientHeight = This.m_VisibleRect.m_Height;

    if ((attributes & VIDEO_PIXEL_ASPECT_RATIO_MASK) == VIDEO_PIXEL_ASPECT_RATIO_1_2)
    {
        if (Machine->orientation & ORIENTATION_SWAP_XY)
            This.m_nClientWidth  *= 2;
        else
            This.m_nClientHeight *= 2;

        This.m_bDouble = FALSE;
    }
/*
    if ((attributes & VIDEO_PIXEL_ASPECT_RATIO_MASK) == VIDEO_PIXEL_ASPECT_RATIO_3_2)
    {
        if (Machine->orientation & ORIENTATION_SWAP_XY)
        {
            This.m_nClientHeight *= 3.0 / 2.0;
        }
        else
        {
            This.m_nClientWidth  *= 3.0 / 2.0;
        }
    }

    if ((attributes & VIDEO_PIXEL_ASPECT_RATIO_MASK) == VIDEO_PIXEL_ASPECT_RATIO_4_3)
    {
        if (Machine->orientation & ORIENTATION_SWAP_XY)
        {
            This.m_nClientHeight *= 4.0 / 3.0;
        }
        else
        {
            This.m_nClientWidth  *= 4.0 / 3.0;
        }
    }
*/
    if (This.m_bDouble == TRUE)
    {
        This.m_nClientWidth  *= 2;
        This.m_nClientHeight *= 2;
    }

    if (This.m_bUseDirty == TRUE)
    {
        if (Machine->orientation & ORIENTATION_SWAP_XY)
            InitDirty(height, width, NO_DIRTY);
        else
            InitDirty(width, height, NO_DIRTY);
    }

    /* osd_create_bitmap will swap width and height if neccesary. */
    This.m_pMAMEBitmap = osd_new_bitmap(width, height, This.m_nDepth);
    
    /* Palette */
    LogPalette.palVersion    = 0x0300;
    LogPalette.palNumEntries = 1;
    This.m_hPalette = CreatePalette(&LogPalette);
    ResizePalette(This.m_hPalette, OSD_NUMPENS);

    /* Create BitmapInfo */
    This.m_pInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) +
                                       sizeof(RGBQUAD) * OSD_NUMPENS);

    This.m_pInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER); 
    This.m_pInfo->bmiHeader.biWidth         =  (This.m_pMAMEBitmap->line[1] - This.m_pMAMEBitmap->line[0]) / (This.m_nDepth / 8);
    This.m_pInfo->bmiHeader.biHeight        = -(int)(This.m_VisibleRect.m_Height); /* Negative means "top down" */
    This.m_pInfo->bmiHeader.biPlanes        = 1;
    This.m_pInfo->bmiHeader.biBitCount      = This.m_nDepth;
    This.m_pInfo->bmiHeader.biCompression   = BI_RGB;
    This.m_pInfo->bmiHeader.biSizeImage     = 0;
    This.m_pInfo->bmiHeader.biXPelsPerMeter = 0;
    This.m_pInfo->bmiHeader.biYPelsPerMeter = 0;
    This.m_pInfo->bmiHeader.biClrUsed       = 0;
    This.m_pInfo->bmiHeader.biClrImportant  = 0;

    if (This.m_nDepth == 8)
    {
        /* Map image values to palette index */
        for (i = 0; i < OSD_NUMPENS; i++)
            ((WORD*)(This.m_pInfo->bmiColors))[i] = i;
    }

    /*
        Modify the main window to suit our needs.
    */
    hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAIN_MENU));
    SetMenu(MAME32App.m_hWnd, hMenu);

    SetWindowLong(MAME32App.m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME | WS_BORDER);

    sprintf(TitleBuf, "%s - %s", Machine->gamedrv->description, MAME32App.m_Name);
    SetWindowText(MAME32App.m_hWnd, TitleBuf);

    StatusCreate();

    This.m_ClientRect.left   = 0;
    This.m_ClientRect.top    = 0;
    This.m_ClientRect.right  = This.m_nClientWidth;
    This.m_ClientRect.bottom = This.m_nClientHeight;

    /* Calculate size of window based on desired client area. */
    Rect.left   = This.m_ClientRect.left;
    Rect.top    = This.m_ClientRect.top;
    Rect.right  = This.m_ClientRect.right;
    Rect.bottom = This.m_ClientRect.bottom + GetStatusHeight();
    AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME | WS_BORDER, hMenu != NULL);

    This.m_nWindowWidth  = RECT_WIDTH(Rect);
    This.m_nWindowHeight = RECT_HEIGHT(Rect);

    SetWindowPos(MAME32App.m_hWnd,
                 HWND_NOTOPMOST,
                 0, 0,
                 This.m_nWindowWidth,
                 This.m_nWindowHeight,
                 SWP_NOMOVE);

    This.m_hWndAbout = CreateDialog(GetModuleHandle(NULL),
                                    MAKEINTRESOURCE(IDD_ABOUT),
                                    MAME32App.m_hWnd,
                                    AboutDialogProc);

    ShowWindow(MAME32App.m_hWnd, SW_SHOW);
    SetForegroundWindow(MAME32App.m_hWnd);

    set_ui_visarea(This.m_VisibleRect.m_Left,
                   This.m_VisibleRect.m_Top,
                   This.m_VisibleRect.m_Left + This.m_VisibleRect.m_Width  - 1,
                   This.m_VisibleRect.m_Top  + This.m_VisibleRect.m_Height - 1);

    return This.m_pMAMEBitmap;
}

/*
    Set the actual display screen but don't allocate the screen bitmap.
*/
static int GDI_set_display(int width, int height, int attributes)
{
    SetForegroundWindow(MAME32App.m_hWnd);
    return 0; 
}

/*
     Shut down the display
*/
static void GDI_close_display(void)
{
    ExitDirty();
    
    if (This.m_nDepth == 8)
    {
        if (This.m_hPalette != NULL)
        {
            DeletePalette(This.m_hPalette);
            This.m_hPalette = NULL;
        }
    }

    if (IsWindow(This.m_hWndAbout))
    {
        DestroyWindow(This.m_hWndAbout);
        This.m_hWndAbout = NULL;
    }

    StatusDelete();
   
    osd_free_bitmap(This.m_pMAMEBitmap);

    if (This.m_pInfo != NULL)
        free(This.m_pInfo);
}

/*
    palette is an array of 'totalcolors' R,G,B triplets. The function returns
    in *pens the pen values corresponding to the requested colors.
    If 'totalcolors' is 32768, 'palette' is ignored and the *pens array is filled
    with pen values corresponding to a 5-5-5 15-bit palette
*/
static void GDI_allocate_colors(unsigned int totalcolors, const unsigned char *palette, unsigned short *pens)
{
    unsigned int i;

    if (This.m_nDepth == 8)
    {
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

        SetPaletteColors();
    }
    else
    {
        int r, g, b;

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

}

/*
    Change the color of the pen.
*/
static void GDI_modify_pen(int pen, unsigned char red, unsigned char green, unsigned char blue)
{
    assert(This.m_nDepth == 8); /* Shouldn't modify pen in 16 bit color depth game mode! */

    assert(0 <= pen && pen < OSD_NUMPENS);

    if (This.m_PalEntries[pen].peRed    == red
    &&  This.m_PalEntries[pen].peGreen  == green
    &&  This.m_PalEntries[pen].peBlue   == blue)
        return;

    SetPen(pen, red, green, blue);

    This.m_bUpdatePalette = TRUE;
}

/*
    Get the color of a pen.
*/
static void GDI_get_pen(int pen, unsigned char* pRed, unsigned char* pGreen, unsigned char* pBlue)
{
    if (This.m_nDepth == 8)
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
        *pRed   = (pen >> 7) & 0xF8;
        *pGreen = (pen >> 2) & 0xF8;
        *pBlue  = (pen << 3) & 0xF8;
    }
}

static void GDI_mark_dirty(int x1, int y1, int x2, int y2, int ui)
{
}

/*
    Update the display.
*/
static void GDI_update_display(void)
{
    if (This.m_bUpdatePalette == TRUE)
    {
        SetPaletteColors();
        This.m_bUpdatePalette = FALSE;
    }

    InvalidateRect(MAME32App.m_hWnd, &This.m_ClientRect, FALSE);


    UpdateWindow(MAME32App.m_hWnd);

    StatusUpdate();

    MAME32App.ProcessMessages();
}

/* control keyboard leds or other indicators */
static void GDI_led_w(int led, int on)
{
    StatusWrite(led, on & 1);
}

static void GDI_set_gamma(float gamma)
{
    assert(OSDDisplay.set_gamma != 0);

    OSDDisplay.set_gamma(gamma);

    AdjustPalette();
}

static void GDI_set_brightness(int brightness)
{
    assert(OSDDisplay.set_brightness != 0);

    OSDDisplay.set_brightness(brightness);

    AdjustPalette();
}

static void GDI_save_snapshot()
{
#if 0
    if (This.m_bAviCapture)
    {
        This.m_nAviShowMessage = 10;        /* show message for 10 frames */
		This.m_bAviRun = !This.m_bAviRun;   /* toggle capture on/off */
	}
    else
#endif
        Display_WriteBitmap(This.m_pMAMEBitmap, This.m_PalEntries);
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static void SetPen(int pen, unsigned char red, unsigned char green, unsigned char blue)
{
    assert(This.m_nDepth == 8); /* Shouldn't set pen in 16 bit color depth game mode! */

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

static void SetPaletteColors(void)
{
    assert(This.m_hPalette != NULL);

    if (This.m_nDepth == 8)
        SetPaletteEntries(This.m_hPalette, 0, OSD_NUMPENS, This.m_AdjustedPalette);        
}

static void AdjustVisibleRect(int xmin, int ymin, int xmax, int ymax)
{
    int temp;
    int w, h;

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

    This.m_VisibleRect.m_Left   = xmin;
    This.m_VisibleRect.m_Top    = ymin;
    This.m_VisibleRect.m_Width  = xmax - xmin + 1;
    This.m_VisibleRect.m_Height = ymax - ymin + 1;
}

static void AdjustPalette(void)
{
    if (This.m_nDepth == 8)
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

static BOOL GDI_OnMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    switch (Msg)
    {
        HANDLE_MESSAGE(hWnd, WM_COMMAND,            OnCommand);
        HANDLE_MESSAGE(hWnd, WM_PAINT,              OnPaint);
        HANDLE_MESSAGE(hWnd, WM_PALETTECHANGED,     OnPaletteChanged);
        HANDLE_MESSAGE(hWnd, WM_QUERYNEWPALETTE,    OnQueryNewPalette);
        HANDLE_MESSAGE(hWnd, WM_GETMINMAXINFO,      OnGetMinMaxInfo);
        HANDLE_MESSAGE(hWnd, WM_ERASEBKGND,         OnEraseBkgnd);
        HANDLE_MESSAGE(hWnd, WM_SIZE,               OnSize);
        HANDLE_MESSAGE(hWnd, WM_DRAWITEM,           OnDrawItem);
    }
    return FALSE;
}

static void GDI_Refresh()
{
    InvalidateRect(MAME32App.m_hWnd, &This.m_ClientRect, FALSE); 
    UpdateWindow(MAME32App.m_hWnd);
}

static int GDI_GetBlackPen(void)
{
    if (This.m_nDepth == 8)
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

static void GDI_UpdateFPS(BOOL bShow, int nSpeed, int nFPS, int nMachineFPS, int nFrameskip, int nVecUPS)
{
    StatusUpdateFPS(bShow, nSpeed, nFPS, nMachineFPS, nFrameskip, nVecUPS);
}

static void OnCommand(HWND hWnd, int CmdID, HWND hWndCtl, UINT codeNotify)
{
    switch (CmdID)
    {
        case ID_FILE_EXIT:
            /* same as double-clicking on main window close box */
            SendMessage(hWnd, WM_CLOSE, 0, 0);
        break;

        case ID_ABOUT:
            ShowWindow(This.m_hWndAbout, SW_SHOW);
        break;
    }
}

static void OnPaint(HWND hWnd)
{
    PAINTSTRUCT     ps;
#if 0
    if (This.m_bAviRun && This.m_pMAMEBitmap) /* add avi frame */
        AviAddBitmap(This.m_pMAMEBitmap, This.m_PalEntries);
    
    if (This.m_nAviShowMessage > 0)
    {
        char    buf[20];
        
        This.m_nAviShowMessage--;
        sprintf(buf, "AVI Capture %s", (This.m_bAviRun) ? "ON" : "OFF");
        StatusSetString(buf);
    }
#endif

    BeginPaint(hWnd, &ps);

    if (This.m_nDepth == 8)
    {
        HPALETTE hOldPalette;

        hOldPalette = SelectPalette(ps.hdc, This.m_hPalette, FALSE);
        RealizePalette(ps.hdc);

        StretchDIBits(ps.hdc,
                      0, 0,
                      This.m_nClientWidth,
                      This.m_nClientHeight,
                      0,
                      0,
                      This.m_VisibleRect.m_Width,
                      This.m_VisibleRect.m_Height,
                      This.m_pMAMEBitmap->line[This.m_VisibleRect.m_Top] + This.m_VisibleRect.m_Left,
                      This.m_pInfo,
                      DIB_PAL_COLORS,
                      SRCCOPY);

        if (hOldPalette != NULL)
            SelectPalette(ps.hdc, hOldPalette, FALSE);        
    }
    else
    {
        StretchDIBits(ps.hdc,
                      0, 0,
                      This.m_nClientWidth,
                      This.m_nClientHeight,
                      0,
                      0,
                      This.m_VisibleRect.m_Width,
                      This.m_VisibleRect.m_Height,
                      This.m_pMAMEBitmap->line[This.m_VisibleRect.m_Top] + (This.m_VisibleRect.m_Left << 1),
                      This.m_pInfo,
                      DIB_RGB_COLORS,
                      SRCCOPY);
    }

    EndPaint(hWnd, &ps); 
}

static void OnPaletteChanged(HWND hWnd, HWND hWndPaletteChange)
{
    if (hWnd == hWndPaletteChange) 
        return; 
    
    OnQueryNewPalette(hWnd);
}

static BOOL OnQueryNewPalette(HWND hWnd)
{
    InvalidateRect(hWnd, NULL, TRUE); 
    UpdateWindow(hWnd); 
    return TRUE;
}

static void OnGetMinMaxInfo(HWND hWnd, MINMAXINFO* pMinMaxInfo)
{
    pMinMaxInfo->ptMaxSize.x      = This.m_nWindowWidth;
    pMinMaxInfo->ptMaxSize.y      = This.m_nWindowHeight;
    pMinMaxInfo->ptMaxTrackSize.x = This.m_nWindowWidth;
    pMinMaxInfo->ptMaxTrackSize.y = This.m_nWindowHeight;
    pMinMaxInfo->ptMinTrackSize.x = This.m_nWindowWidth;
    pMinMaxInfo->ptMinTrackSize.y = This.m_nWindowHeight;
}

static BOOL OnEraseBkgnd(HWND hWnd, HDC hDC)
{
    /* No background erasing is required. */
    return TRUE;
}

static void OnSize(HWND hwnd,UINT state,int cx,int cy)
{
    StatusWindowSize(state,cx,cy);
}

static void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItem)
{
    StatusDrawItem(lpDrawItem);
}

static LRESULT CALLBACK AboutDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_INITDIALOG:
            return 1;

        case WM_COMMAND:
            ShowWindow(hDlg, SW_HIDE);
            return 1;
    }
    return 0;
}

