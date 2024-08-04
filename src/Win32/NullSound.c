/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  NullSound.c

 ***************************************************************************/

#include "mame.h"
#include "osdepend.h"
#include "mame32.h"
#include "NullSound.h"

/***************************************************************************
    function prototypes
 ***************************************************************************/

static int      NullSound_init(options_type *options);
static void     NullSound_exit(void);
static void     NullSound_update_audio(void);
static void     NullSound_play_sample(int channel, signed char* data, int len, int freq, int volume, int loop);
static void     NullSound_play_sample_16(int channel, signed short* data, int len, int freq, int volume, int loop);
static void     NullSound_play_streamed_sample(int channel, signed char* data, int len, int freq, int volume, int pan);
static void     NullSound_play_streamed_sample_16(int channel, signed short* data, int len, int freq, int volume, int pan);
static void     NullSound_set_sample_freq(int channel,int freq);
static void     NullSound_set_sample_volume(int channel,int volume);
static void     NullSound_stop_sample(int channel);
static void     NullSound_restart_sample(int channel);
static int      NullSound_get_sample_status(int channel);
static void     NullSound_set_mastervolume(int volume);
static int      NullSound_get_mastervolume();
static void     NullSound_sound_enable(int enable);

/***************************************************************************
    External variables
 ***************************************************************************/

struct OSDSound NullSound = 
{
    { NullSound_init },                     /* init                    */
    { NullSound_exit },                     /* exit                    */
    { NullSound_update_audio },             /* update_audio            */
    { NullSound_play_sample },              /* play_sample             */
    { NullSound_play_sample_16 },           /* play_sample_16          */
    { NullSound_play_streamed_sample },     /* play_streamed_sample    */
    { NullSound_play_streamed_sample_16 },  /* play_streamed_sample_16 */
    { NullSound_set_sample_freq },          /* adjust_sample_freq      */
    { NullSound_set_sample_volume },        /* adjust_sample_volume    */
    { NullSound_stop_sample },              /* stop_sample             */
    { NullSound_restart_sample },           /* restart_sample          */
    { NullSound_get_sample_status },        /* get_sample_status       */
    { NullSound_set_mastervolume },         /* set_mastervolume        */
    { NullSound_get_mastervolume },         /* get_mastervolume        */
    { NullSound_sound_enable }              /* sound_enable            */
};

/***************************************************************************
    Internal structures
 ***************************************************************************/

/***************************************************************************
    Internal variables
 ***************************************************************************/

static unsigned int m_nVolume;

/***************************************************************************
    External OSD functions  
 ***************************************************************************/

static int NullSound_init(options_type *options)
{
    /* update the Machine structure to show that sound is disabled */
    Machine->sample_rate = 0;
    m_nVolume = 0;
    return 0;
}

static void NullSound_exit(void)
{
}

static void NullSound_update_audio(void)
{
}

static void NullSound_play_sample(int channel, signed char *data, int len, int freq, int volume, int loop)
{
}

static void NullSound_play_sample_16(int channel, signed short *data, int len, int freq, int volume, int loop)
{
}

static void NullSound_play_streamed_sample(int channel, signed char *data, int len, int freq, int volume, int pan)
{
}

static void NullSound_play_streamed_sample_16(int channel, signed short *data, int len, int freq, int volume, int pan)
{
}

static void NullSound_set_sample_freq(int channel,int freq)
{
}

static void NullSound_set_sample_volume(int channel,int volume)
{
}

static void NullSound_stop_sample(int channel)
{
}

static void NullSound_restart_sample(int channel)
{
}

static int NullSound_get_sample_status(int channel)
{
    return 0;
}

static void NullSound_set_mastervolume(int volume)
{
    m_nVolume = volume;
}

static int NullSound_get_mastervolume()
{
    return m_nVolume;
}

static void NullSound_sound_enable(int enable)
{

}

/***************************************************************************
    Internal functions
 ***************************************************************************/

