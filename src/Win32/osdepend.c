/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  osdepend.c

  OS dependent stuff (display handling, keyboard scan...)

 ***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "osdepend.h"
#include "MAME32.h"
#include "M32Util.h"
#include "uclock.h"
//#include "DDrawDisplay.h"
//#include "DDrawWindow.h"
#include "NullSound.h"

#ifndef NOSEAL
#include "SealSound.h"
#endif

#ifndef NOMIDAS
#include "MIDASSound.h"
#endif

#include "DirectSound.h"
#include "GDIDisplay.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "DIKeyboard.h"
#include "DIJoystick.h"
#include "Trak.h"
#include "file.h"
#include "win32ui.h"
#include "fmsynth.h"
#include "NTfmsynth.h"

#ifdef MAME_NET
    #include <winsock.h>
    #include "network.h"
    #include "net32.h"
    #include "options.h"
#endif /* MAME_NET */

/***************************************************************************
    External variables
 ***************************************************************************/

int     autoframeskip, frameskip;  // Hack, Hack, Hack - FIXME! MSH 01/26/99

/***************************************************************************
    Internal variables
 ***************************************************************************/

/***************************************************************************
    External OSD function definitions  
 ***************************************************************************/

int osd_init()
{
    BOOL    bUseDirectDraw  = TRUE;
    BOOL    bUseWindow      = FALSE;
    BOOL    bNoSound        = FALSE;
    BOOL    bUseMIDASSound  = FALSE;
    BOOL    bUseDirectSound = FALSE;
    BOOL    bUseAIMouse     = FALSE;
    BOOL    bUseDIKeyboard  = FALSE;
    BOOL    bUseDIJoystick  = FALSE;
#ifdef MAME_NET
    BOOL    bUseNetwork     = FALSE;
#endif /* MAME_NET */
    BOOL    bDisplayInitialized  = FALSE;
    BOOL    bSoundInitialized    = FALSE;
    BOOL    bKeyboardInitialized = FALSE;
    BOOL    bJoystickInitialized = FALSE;
    BOOL    bTrakInitialized     = FALSE;
    BOOL    bFMSynthInitialized  = FALSE;
#ifdef MAME_NET
    BOOL    bNetworkInitialized  = FALSE;
#endif /* MAME_NET */

    options_type *options = GetPlayingGameOptions();

    if (options->is_window)
        bUseWindow = TRUE;

//    if (options->is_window && !options->window_ddraw)
        bUseDirectDraw = FALSE;

    if (options->sound == SOUND_NONE)
        bNoSound = TRUE;

#ifndef NOMIDAS
    if (options->sound == SOUND_MIDAS)
        bUseMIDASSound = TRUE;
#endif

#if 0
    if (options->sound == SOUND_DIRECT)
        bUseDirectSound = TRUE;
#endif

    if (options->use_ai_mouse)
        bUseAIMouse = TRUE;

    if (options->di_keyboard)
        bUseDIKeyboard = TRUE;

    if (options->di_joystick)
        bUseDIJoystick = TRUE;

#ifdef MAME_NET
    if (MAME32App.m_bUseNetwork)
        bUseNetwork = TRUE;

#endif /* MAME_NET */
   
    /*
        Set up.
    */
    MAME32App_init(options);

#if 0
    if (bUseDirectDraw == FALSE)
        MAME32App.m_pDisplay = &GDIDisplay;
    else
    if (bUseWindow == TRUE)
        MAME32App.m_pDisplay = &DDrawWindowDisplay;
    else
#else
	if(1==1)
	   MAME32App.m_pDisplay = &GDIDisplay;
#endif
#if defined(MAME_DEBUG)
        /* don't do fullscreen debugging */
        MAME32App.m_pDisplay = &GDIDisplay;
#else
//        MAME32App.m_pDisplay = &DDrawDisplay;
//RISC machines and newer sdk's seem ot just work better with GDI
// And windows on arm on PI has no DX... :(
        MAME32App.m_pDisplay = &GDIDisplay;
#endif

    if (bNoSound == TRUE)
        MAME32App.m_pSound = &NullSound;
    else
#ifndef NOMIDAS
    if (bUseMIDASSound == TRUE)
        MAME32App.m_pSound = &MIDASSound;
    else
#endif
#if 0
    if (bUseDirectSound == TRUE)
        MAME32App.m_pSound = &DirectSound;
#else
        MAME32App.m_pSound = &NullSound;
#endif
#ifndef NOSEAL
    else
        MAME32App.m_pSound = &SealSound;
#endif

    if (bUseAIMouse == TRUE)
        MAME32App.m_pTrak = &Trak;
    else
        MAME32App.m_pTrak = NULL;

    MAME32App.m_bUseAIMouse = bUseAIMouse;

#if 0
    if (bUseDIKeyboard == TRUE)
        MAME32App.m_pKeyboard = &DIKeyboard;
    else
        MAME32App.m_pKeyboard = &Keyboard;
    if (bUseDIJoystick == TRUE)
        MAME32App.m_pJoystick = &DIJoystick;
    else
        MAME32App.m_pJoystick = &Joystick;
#else
MAME32App.m_pKeyboard = &Keyboard;
MAME32App.m_pJoystick = &Joystick;
#endif

    if (options->fm_ym3812)
    {
        if (OnNT())
            MAME32App.m_pFMSynth = &NTFMSynth;
        else
            MAME32App.m_pFMSynth = &FMSynth;
    }
    else
        MAME32App.m_pFMSynth = NULL;

#ifdef MAME_NET

#if 0
    if (bUseNetwork == TRUE )
	{
        MAME32App.m_pNetwork = &Network;
	}
    else
	{
        MAME32App.m_pNetwork = NULL;
	}

    /* TODO: show chat screen */
#else
	MAME32App.m_pNetwork = NULL;
#endif
	/* init network thread */
    if (bUseNetwork)
    {
		if (net_game_init() != 0)
		{
            goto error;
		}
        else
		{
            bNetworkInitialized = TRUE;
		}
    }

	/* TODO: spin until chat screen says go */
#endif /* MAME_NET */

    MAME32App.m_hWnd = MAME32App.CreateMAMEWindow();

    if (IsWindow(MAME32App.m_hWnd) == FALSE)
        goto error;

    /* 
        Initialize everything.
    */
    uclock_init();
    if (MAME32App.m_pDisplay != NULL)
    {        
        if (MAME32App.m_pDisplay->init(options) != 0)
            goto error;
        else
            bDisplayInitialized = TRUE;
    }

    if (MAME32App.m_pSound != NULL)
    {
        if (MAME32App.m_pSound->init(options) != 0)
            goto error;
        else
            bSoundInitialized = TRUE;        
    }

    if (MAME32App.m_pKeyboard != NULL)
    {
        if (MAME32App.m_pKeyboard->init(options) != 0)
            goto error;
        else
            bKeyboardInitialized = TRUE;        
    }

    if (MAME32App.m_pJoystick != NULL)
    {
        if (MAME32App.m_pJoystick->init(options) != 0)
            goto error;
        else
            bJoystickInitialized = TRUE;
    }

    if (MAME32App.m_pTrak != NULL)
    {
        if (MAME32App.m_pTrak->init(options) != 0)
            goto error;
        else
            bTrakInitialized = TRUE;
    }

    if (MAME32App.m_pFMSynth != NULL)
    {
        if (MAME32App.m_pFMSynth->init(options) != 0)
            goto error;
        else
            bFMSynthInitialized = TRUE;
    }

    MAME32App.m_bIsInitialized = TRUE;

    osd_set_mastervolume(- options->volume);
    return 0;

error:

    if (bFMSynthInitialized  == TRUE) MAME32App.m_pFMSynth->exit();
    if (bTrakInitialized     == TRUE) MAME32App.m_pTrak->exit();
    if (bJoystickInitialized == TRUE) MAME32App.m_pJoystick->exit();
    if (bKeyboardInitialized == TRUE) MAME32App.m_pKeyboard->exit();
    if (bSoundInitialized    == TRUE) MAME32App.m_pSound->exit();
    if (bDisplayInitialized  == TRUE) MAME32App.m_pDisplay->exit();
#ifdef MAME_NET
    //if (bNetInitialized)  osd_net_game_exit();
    if (MAME32App.m_pNetwork  != NULL) net_exit( NET_QUIT_ABRT );
#endif /* MAME_NET */
    uclock_exit();

    if (IsWindow(MAME32App.m_hWnd))
        DestroyWindow(MAME32App.m_hWnd);

    return 1;
}

void osd_exit(void)
{
    MAME32App.m_bIsInitialized = FALSE;
    MAME32App.m_bDone = FALSE;

    if (MAME32App.m_pFMSynth  != NULL) MAME32App.m_pFMSynth->exit();
    if (MAME32App.m_pTrak     != NULL) MAME32App.m_pTrak->exit();
    if (MAME32App.m_pJoystick != NULL) MAME32App.m_pJoystick->exit();
    if (MAME32App.m_pKeyboard != NULL) MAME32App.m_pKeyboard->exit();
    if (MAME32App.m_pSound    != NULL) MAME32App.m_pSound->exit();
    if (MAME32App.m_pDisplay  != NULL) MAME32App.m_pDisplay->exit();
#ifdef MAME_NET
    if (MAME32App.m_pNetwork  != NULL) net_exit( NET_QUIT_QUIT );
#endif /* MAME_NET */

    uclock_exit();

    if (IsWindow(MAME32App.m_hWnd))
        DestroyWindow(MAME32App.m_hWnd);
}

/***************************************************************************
    Display
 ***************************************************************************/

struct osd_bitmap* osd_new_bitmap(int width, int height, int depth)
{
    return MAME32App.m_pDisplay->new_bitmap(width, height, depth);
}

void osd_free_bitmap(struct osd_bitmap* pBitmap)
{
    MAME32App.m_pDisplay->free_bitmap(pBitmap);
}

struct osd_bitmap *osd_create_display(int width, int height, int depth, int attributes)
{
    return MAME32App.m_pDisplay->create_display(width, height, depth, attributes);
}

int osd_set_display(int width,int height,int attributes)
{
    return MAME32App.m_pDisplay->set_display(width, height, attributes);
}

void osd_close_display(void)
{
    MAME32App.m_pDisplay->close_display();
}

void osd_allocate_colors(unsigned int totalcolors, const unsigned char *palette, unsigned short *pens)
{
    MAME32App.m_pDisplay->allocate_colors(totalcolors, palette, pens);
}

void osd_modify_pen(int pen, unsigned char red, unsigned char green, unsigned char blue)
{
    MAME32App.m_pDisplay->modify_pen(pen, red, green, blue);
}

void osd_get_pen(int pen, unsigned char* red, unsigned char* green, unsigned char* blue)
{
    MAME32App.m_pDisplay->get_pen(pen, red, green, blue);
}

void osd_update_video_and_audio(void)
{
    OSDDisplay.update_display();
    MAME32App.m_pSound->update_audio();

    if (MAME32App.m_bIsPaused)
    {
        int paused = MAME32App.m_bPaused;

        if (! paused)
            osd_pause(TRUE);

        do {
            MAME32App.ProcessMessages();
        } while (MAME32App.m_bIsPaused);

        if (! paused)
            osd_pause(FALSE);
    }
}

void osd_clearbitmap(struct osd_bitmap *bitmap)
{
    MAME32App.m_pDisplay->clearbitmap(bitmap);
}

void osd_mark_dirty(int x1, int y1, int x2, int y2, int ui)
{
    MAME32App.m_pDisplay->mark_dirty(x1, y1, x2, y2, ui);
}

void osd_led_w(int led, int on)
{
    MAME32App.m_pDisplay->led_w(led, on);
}

int osd_skip_this_frame()
{
    return OSDDisplay.skip_this_frame();
}

void osd_set_gamma(float _gamma)
{
    MAME32App.m_pDisplay->set_gamma(_gamma);
}

float osd_get_gamma(void)
{
    return MAME32App.m_pDisplay->get_gamma();
}

void osd_set_brightness(int brightness)
{
    MAME32App.m_pDisplay->set_brightness(brightness);
}

int osd_get_brightness(void)
{
    return MAME32App.m_pDisplay->get_brightness();
}

void osd_save_snapshot(void)
{
    MAME32App.m_pDisplay->save_snapshot();
}

/***************************************************************************
    Sound
 ***************************************************************************/

void osd_play_sample(int channel, signed char *data, int len, int freq, int volume, int loop)
{
    MAME32App.m_pSound->play_sample(channel, data, len, freq, volume, loop);
}

void osd_play_sample_16(int channel, signed short *data, int len, int freq, int volume, int loop)
{
    MAME32App.m_pSound->play_sample_16(channel, data, len, freq, volume, loop);
}

void osd_play_streamed_sample(int channel, signed char *data, int len, int freq, int volume, int pan)
{
    MAME32App.m_pSound->play_streamed_sample(channel, data, len, freq, volume, pan);
}

void osd_play_streamed_sample_16(int channel, signed short *data, int len, int freq, int volume, int pan)
{
    MAME32App.m_pSound->play_streamed_sample_16(channel, data, len, freq, volume, pan);
}

void  osd_set_sample_freq(int channel, int freq)
{
    MAME32App.m_pSound->set_sample_freq(channel, freq);
}

void  osd_set_sample_volume(int channel, int volume)
{
    MAME32App.m_pSound->set_sample_volume(channel, volume);
}

void osd_stop_sample(int channel)
{
    MAME32App.m_pSound->stop_sample(channel);
}

void osd_restart_sample(int channel)
{
    MAME32App.m_pSound->restart_sample(channel);
}

void osd_set_mastervolume(int attenuation)
{
    if (attenuation > 0)
        attenuation = 0;
    if (attenuation < -32)
        attenuation = -32;

    MAME32App.m_pSound->set_mastervolume(attenuation);
}

int osd_get_mastervolume()
{
    return MAME32App.m_pSound->get_mastervolume();
}

void osd_sound_enable(int enable)
{
    MAME32App.m_pSound->sound_enable(enable);
}

int osd_get_sample_status(int channel)
{
    return MAME32App.m_pSound->get_sample_status(channel);
}

void osd_opl_control(int chip, int reg)
{
    if (MAME32App.m_pFMSynth)
        MAME32App.m_pFMSynth->opl_control(chip, reg);
}

void osd_opl_write(int chip, int data)
{
    if (MAME32App.m_pFMSynth)
        MAME32App.m_pFMSynth->opl_write(chip, data);
}

/***************************************************************************
    Keyboard
 ***************************************************************************/

/*
  return a list of all available keys (see input.h)
*/
const struct KeyboardInfo *osd_get_key_list(void)
{
   return MAME32App.m_pKeyboard->get_key_list();
}

/*
  inptport.c defines some general purpose defaults for key bindings. They may be
  further adjusted by the OS dependant code to better match the available keyboard,
  e.g. one could map pause to the Pause key instead of P, or snapshot to PrtScr
  instead of F12. Of course the user can further change the settings to anything
  he/she likes.
  This function is called on startup, before reading the configuration from disk.
  Scan the list, and change the keys you want.
*/
void osd_customize_inputport_defaults(struct ipd *defaults)
{
   MAME32App.m_pKeyboard->customize_inputport_defaults(defaults);
}

/*
  tell whether the specified key is pressed or not. keycode is the OS dependant
  code specified in the list returned by osd_customize_inputport_defaults().
*/
int osd_is_key_pressed(int keycode)
{
   return MAME32App.m_pKeyboard->is_key_pressed(keycode);
}

/*
  wait for the user to press a key. This function is not required to do anything,
  it is only here so we can avoid bogging down multitasking systems while using
  the debugger. If you don't want to or can't support this function you can just
  return immediately.
*/
int osd_wait_keypress(void)
{
   return MAME32App.m_pKeyboard->wait_keypress();
}

void osd_pause(int paused)
{
    static float orig_brt;

    /* cmk 990721 broken */
    if (paused)
    {
        orig_brt = osd_get_brightness();
		osd_set_brightness(orig_brt * 0.65);

        MAME32App.m_pDisplay->update_display();
        MAME32App.m_pSound->update_audio();
    }
    else
    {
		osd_set_brightness(orig_brt);
    }
    /*MAME32App.m_pKeyboard->pause(paused);*/
}

/***************************************************************************
    Joystick
 ***************************************************************************/

const struct JoystickInfo *osd_get_joy_list(void)
{
    return MAME32App.m_pJoystick->get_joy_list();
}

void osd_poll_joysticks(void)
{
    MAME32App.m_pJoystick->poll_joysticks();
}

int osd_is_joy_pressed(int joycode)
{
    if (!MAME32App.m_bIsInitialized)
        return 0;

    return MAME32App.m_pJoystick->is_joy_pressed(joycode);
}

/* Do we need to calibrate the joystick at all? */
int osd_joystick_needs_calibration(void)
{
    return 0;
}

/* Preprocessing for joystick calibration. Returns 0 on success */
void osd_joystick_start_calibration(void)
{
    assert(FALSE);
}

/* Prepare the next calibration step. Return a description of this step. */
/* (e.g. "move to upper left") */
char *osd_joystick_calibrate_next(void)
{
    assert(FALSE);
    return " ";
}

/* Get the actual joystick calibration data for the current position */
void osd_joystick_calibrate(void)
{
    assert(FALSE);
}

/* Postprocessing (e.g. saving joystick data to config) */
void osd_joystick_end_calibration(void)
{
    assert(FALSE);
}

void osd_analogjoy_read(int player, int *analog_x, int *analog_y)
{
    MAME32App.m_pJoystick->analogjoy_read(player, analog_x, analog_y);
}

/***************************************************************************
    Trakball
 ***************************************************************************/

void osd_trak_read(int player, int *deltax, int *deltay)
{
    if (MAME32App.m_bUseAIMouse == TRUE)
    {
        if (MAME32App.m_pTrak == NULL)
        {
            *deltax = *deltay = 0;
            return;
        }

        MAME32App.m_pTrak->trak_read(player, deltax, deltay);
        return;
    }

    if (MAME32App.m_pJoystick == NULL)
    {
        *deltax = *deltay = 0;
        return;
    }

    *deltax = MAME32App.m_pJoystick->standard_analog_read(player, X_AXIS);
    *deltay = MAME32App.m_pJoystick->standard_analog_read(player, Y_AXIS);
}

/***************************************************************************
    Files
 ***************************************************************************/

int osd_faccess(const char *filename, int filetype)
{
    return File.faccess(filename, filetype);
}

void *osd_fopen(const char *gamename,const char *filename,int filetype,int write)
{
    return File.fopen(gamename,filename,filetype,write);
}

int osd_fread(void *file,void *buffer,int length)
{
    return File.fread(file,buffer,length);
}

int osd_fread_swap(void *file,void *buffer,int length)
{
	int i;
	unsigned char *buf;
	unsigned char temp;
	int res;


	res = osd_fread(file,buffer,length);

	buf = buffer;
	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	return res;
}

int osd_fwrite(void *file,const void *buffer,int length)
{
    return File.fwrite(file,buffer,length);
}

int osd_fwrite_swap(void *file,const void *buffer,int length)
{
	int i;
	unsigned char *buf;
	unsigned char temp;
	int res;


	buf = (unsigned char *)buffer;
	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	res = osd_fwrite(file,buffer,length);

	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	return res;
}

int osd_fread_scatter(void *file,void *buffer,int length,int increment)
{
	return File.fread_scatter(file, buffer, length, increment);
}

int osd_fseek(void *file,int offset,int whence)
{
    return File.fseek(file,offset,whence);
}

void osd_fclose(void *file)
{
    File.fclose(file);
}

int osd_fchecksum(const char *gamename, const char *filename, unsigned int *length, unsigned int *sum)
{
    return File.fchecksum(gamename, filename, length, sum);
}

int osd_ftell(void *file)
{
    return File.ftell(file);
}

int osd_fsize(void *file)
{
    return File.fsize(file);
}

unsigned int osd_fcrc(void *file)
{
    return File.fcrc(file);
}

int osd_display_loading_rom_message(const char *name,int current,int total)
{
   int retval;

   if (name != NULL)
      retval = UpdateLoadProgress(current-1,total);
   else
      retval = UpdateLoadProgress(total,total);

   return retval;
}

/***************************************************************************
    Config
 ***************************************************************************/

int osd_get_config_samplerate(int def_samplerate)
{
/*  
    char tmp[64];

    sprintf(tmp,"%d",GetDefaultOptions()->sample_rate);
    ErrorMsg("debug: %s", tmp);
*/
    return GetDefaultOptions()->sample_rate;
}

int osd_get_config_samplebits(int def_samplebits)
{
    return def_samplebits;
}

int osd_get_config_frameskip(int def_frameskip)
{
    return def_frameskip;
}

void osd_set_config(int def_samplerate, int def_samplebits)
{

}

void osd_save_config(int frameskip, int samplerate, int samplebits)
{

}

#ifdef MAME_NET

/***************************************************************************
    Network
 ***************************************************************************/

int osd_net_init(void)
{
    if (!MAME32App.m_pNetwork)
    {
        MAME32App.m_pNetwork = &Network;
    }
	return MAME32App.m_pNetwork->init();
}

int osd_net_game_init(void)
{
    if (!MAME32App.m_pNetwork)
    {
        MAME32App.m_pNetwork = &Network;
    }
    return MAME32App.m_pNetwork->game_init();
}

int osd_net_game_exit(void)
{
    if (!MAME32App.m_pNetwork)
	    return 0;
    else
        return MAME32App.m_pNetwork->game_exit();
}

int osd_net_exit(void)
{
    if (!MAME32App.m_pNetwork)
	    return 0;
    else
        return MAME32App.m_pNetwork->exit();
}

int osd_net_send(int port, unsigned char buf[], int *size)
{
	return MAME32App.m_pNetwork->send( port, buf, size );
}

int osd_net_recv(int port, unsigned char buf[], int *size)
{
	return MAME32App.m_pNetwork->recv( port, buf, size );
}

int osd_net_sync(void)
{
    if (MAME32App.m_pNetwork == NULL)
        return -1;
    else
	    return MAME32App.m_pNetwork->sync();
}

int osd_net_input_sync(void)
{
	return 0;
}

int osd_net_add_player(void)
{
	if ( MAME32App.m_pNetwork == NULL )
		return -1;
	else
		return MAME32App.m_pNetwork->add_player();
}

int osd_net_remove_player( int player )
{
	if ( MAME32App.m_pNetwork == NULL )
		return -1;
	else
		return MAME32App.m_pNetwork->remove_player( player );
}

#endif /* MAME_NET */
