/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  SealSound.c

 ***************************************************************************/
#include "driver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <conio.h>
#include "MAME32.h"
#include "osdepend.h"
#include "SealSound.h"
#include "M32Util.h"
#include "audio.h"
#include "mame.h"    /* for Machine */
#include "Display.h" /* for Throttled */

#if AUDIO_SYSTEM_VERSION < 0x106
#pragma message ("Using SEAL version older than 1.06.")
#endif

//#define MIXER_MAX_CHANNELS            16

/***************************************************************************
    function prototypes
 ***************************************************************************/

static int      SealSound_init(options_type *options);
static void     SealSound_exit(void);
static void     SealSound_update_audio(void);
static void     SealSound_play_sample(int channel, signed char *data,int len,int freq,int volume,int loop);
static void     SealSound_play_sample_16(int channel, signed short *data,int len,int freq,int volume,int loop);
static void     SealSound_play_streamed_sample(int channel, signed char *data,int len,int freq,int volume, int pan);
static void     SealSound_play_streamed_sample_16(int channel, signed short *data,int len,int freq,int volume, int pan);
//static void     SealSound_adjust_sample(int channel,int freq,int volume);
static void     SealSound_set_sample_freq(int channel,int freq);
static void     SealSound_set_sample_volume(int channel,int volume);
static void     SealSound_stop_sample(int channel);
static void     SealSound_restart_sample(int channel);
static int      SealSound_get_sample_status(int channel);
static void     SealSound_set_mastervolume(int volume);
static int      SealSound_get_mastervolume();
static void     SealSound_sound_enable(int enable);

static void     play_sample(int channel, signed char *data, int len, int freq, int volume, int loop, int bits);
static void     play_streamed_sample(int channel, signed char *data, int len, int freq, int volume, int pan, int bits);

/***************************************************************************
    External variables
 ***************************************************************************/

struct OSDSound SealSound = 
{
    { SealSound_init },                     /* init                    */
    { SealSound_exit },                     /* exit                    */
    { SealSound_update_audio },             /* update_audio            */
    { SealSound_play_sample },              /* play_sample             */
    { SealSound_play_sample_16 },           /* play_sample_16          */
    { SealSound_play_streamed_sample },     /* play_streamed_sample    */
    { SealSound_play_streamed_sample_16 },  /* play_streamed_sample_16 */
    //{ SealSound_adjust_sample },            /* adjust_sample           */
    { SealSound_set_sample_freq },          /* adjust_sample_freq      */
    { SealSound_set_sample_volume },        /* adjust_sample_volume    */
    { SealSound_stop_sample },              /* stop_sample             */
    { SealSound_restart_sample },           /* restart_sample          */
    { SealSound_get_sample_status },        /* get_sample_status       */
    { SealSound_set_mastervolume },         /* set_mastervolume        */
    { SealSound_get_mastervolume },         /* get_mastervolume        */
    { SealSound_sound_enable }              /* sound_enable            */
};

/***************************************************************************
    Internal structures
 ***************************************************************************/

struct tSound_private
{
    BOOL            m_bInitialized;

    HAC             m_hVoice[MIXER_MAX_CHANNELS];
    LPAUDIOWAVE     m_lpWave[MIXER_MAX_CHANNELS];
    int             m_nVolume[MIXER_MAX_CHANNELS];
    BOOL            m_bPlaying[MIXER_MAX_CHANNELS];
    int             m_c[MIXER_MAX_CHANNELS];

    int             m_nMasterVolume;
    int             m_nAttenuation;
    float           m_fAttenuation_mult;
    BOOL            m_bPollStream;
};

/***************************************************************************
    Internal variables
 ***************************************************************************/

static struct tSound_private This;
  
/***************************************************************************
    External OSD functions  
 ***************************************************************************/

/*
    put here anything you need to do when the program is started. Return 0 if 
    initialization was successful, nonzero otherwise.
*/
static int SealSound_init(options_type *options)
{
    int     i;
    int     soundcard = options->seal_index;

    This.m_bInitialized         = FALSE;
    This.m_nAttenuation         = 0;
    This.m_fAttenuation_mult    = 1.0f;
    This.m_bPollStream          = TRUE;
	This.m_nMasterVolume        = 256;

    for (i = 0; i < MIXER_MAX_CHANNELS; i++)
    {
        This.m_hVoice[i]    = NULL;
        This.m_lpWave[i]    = NULL;
        This.m_nVolume[i]   = 0;
        This.m_bPlaying[i]  = FALSE;
        This.m_c[i]         = 0;
    }

    {
        AUDIOINFO   info;
        AUDIOCAPS   caps;

        /* initialize SEAL audio library */
        if (AInitialize() == AUDIO_ERROR_NONE)
        {
            if (soundcard == -1)
                soundcard = 0;

            if (AGetAudioNumDevs() <= (UINT)soundcard)
                soundcard = AGetAudioNumDevs();

            if (soundcard == 0) /* silence */
                /* update the Machine structure to show that sound is disabled */
                Machine->sample_rate = 0;
            else
            {
                /*
                    If windows wave device is selected, don't poll stream
                    audio because SEAL will lock up in the for(;;) loop.
                */
                if (AGetAudioDevCaps(soundcard, &caps) == AUDIO_ERROR_NONE)
                {
                    if (caps.wProductId == AUDIO_PRODUCT_WINDOWS)
                        This.m_bPollStream = FALSE;
                }

                /* open audio device */
                /*info.nDeviceId = AUDIO_DEVICE_MAPPER;*/
                info.nDeviceId      = soundcard;
                info.nSampleRate    = Machine->sample_rate;

                /* always use 16 bit mixing if possible - better quality and same speed of 8 bit */
                if (options->stereo)
                    info.wFormat = AUDIO_FORMAT_16BITS | AUDIO_FORMAT_STEREO;
                else
                    info.wFormat = AUDIO_FORMAT_16BITS | AUDIO_FORMAT_MONO;

                /*
                    AOpenAudio() seems to get the active window,
                    so make our window active.
                */ 
                SetActiveWindow(MAME32App.m_hWnd);
                
                if (AOpenAudio(&info) != AUDIO_ERROR_NONE)
                {
                    ErrorMsg("audio initialization failed\n");
                    return 1;
                }

                AGetAudioDevCaps(info.nDeviceId, &caps);
                /* update the Machine structure to reflect the actual sample rate */
                Machine->sample_rate = info.nSampleRate;

                /* open and allocate voices, allocate waveforms */
                if (AOpenVoices(MIXER_MAX_CHANNELS) != AUDIO_ERROR_NONE)
                {
                    ErrorMsg("voices initialization failed\n");
                    return 1;
                }

                for (i = 0; i < MIXER_MAX_CHANNELS; i++)
                {
                    if (ACreateAudioVoice(&This.m_hVoice[i]) != AUDIO_ERROR_NONE)
                    {
                        ErrorMsg("voice #%d creation failed\n",i);
                        return 1;
                    }

                    ASetVoicePanning(This.m_hVoice[i], 128);

                    This.m_lpWave[i] = 0;
                }
            
                ASetAudioMixerValue(AUDIO_MIXER_MASTER_VOLUME, This.m_nMasterVolume);
            }
        }
    }

    This.m_bInitialized = TRUE;

    return 0;
}

/*
    put here cleanup routines to be executed when the program is terminated.
*/
static void SealSound_exit(void)
{
    if (This.m_bInitialized == FALSE)
        return;

    if (Machine->sample_rate != 0)
    {
        int n;

        /* stop and release voices */
        for (n = 0; n < MIXER_MAX_CHANNELS; n++)
        {
            AStopVoice(This.m_hVoice[n]);
            ADestroyAudioVoice(This.m_hVoice[n]);
            if (This.m_lpWave[n] != 0)
            {
                ADestroyAudioData(This.m_lpWave[n]);
                free(This.m_lpWave[n]);
                This.m_lpWave[n] = 0;
            }
        }
        ACloseVoices();
        ACloseAudio();
    }

    This.m_bInitialized = FALSE;
}

static void SealSound_update_audio(void)
{
}

static void SealSound_play_sample(int channel, signed char *data, int len, int freq, int volume, int loop)
{
    play_sample(channel, data, len, freq, volume, loop, 8);
}

static void SealSound_play_sample_16(int channel, signed short *data, int len, int freq, int volume, int loop)
{
    play_sample(channel, (signed char *)data, len, freq, volume, loop, 16);
}

static void SealSound_play_streamed_sample(int channel, signed char *data, int len, int freq, int volume, int pan)
{
    play_streamed_sample(channel, data, len, freq, volume, pan, 8);
}

static void SealSound_play_streamed_sample_16(int channel, signed short *data, int len, int freq, int volume, int pan)
{
    play_streamed_sample(channel, (signed char *)data, len, freq, volume, pan, 16);
}

#if 0
static void SealSound_adjust_sample(int channel, int freq, int volume)
{
    assert(This.m_bInitialized == TRUE);

    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return;

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    if (freq != -1)
        ASetVoiceFrequency(This.m_hVoice[channel], freq);
    if (volume != -1)
    {
#if AUDIO_SYSTEM_VERSION >= 0x106
        ASetVoiceVolume(This.m_hVoice[channel], (UINT)(volume * 64 / 100));
#else
        This.m_nVolume[channel] = volume * 64 / 100;
        ASetVoiceVolume(This.m_hVoice[channel], (UINT)(This.m_fAttenuation_mult * This.m_nVolume[channel]));
#endif
    }
}
#endif

static void SealSound_set_sample_freq(int channel,int freq)
{
    assert(This.m_bInitialized == TRUE);

    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return;

    if (freq != -1)
        ASetVoiceFrequency(This.m_hVoice[channel], freq);
}

static void SealSound_set_sample_volume(int channel,int volume)
{
    assert(This.m_bInitialized == TRUE);

    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return;

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    if (volume != -1)
    {
#if AUDIO_SYSTEM_VERSION >= 0x106
        ASetVoiceVolume(This.m_hVoice[channel], (UINT)(volume * 64 / 100));
#else
        This.m_nVolume[channel] = volume * 64 / 100;
        ASetVoiceVolume(This.m_hVoice[channel], (UINT)(This.m_fAttenuation_mult * This.m_nVolume[channel]));
#endif
    }
}

static void SealSound_stop_sample(int channel)
{
    assert(This.m_bInitialized == TRUE);

    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return;

    AStopVoice(This.m_hVoice[channel]);
}

static void SealSound_restart_sample(int channel)
{
    assert(This.m_bInitialized == TRUE);

    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return;

    AStartVoice(This.m_hVoice[channel]);
}

static int SealSound_get_sample_status(int channel)
{
    int     stopped = 0;

    assert(This.m_bInitialized == TRUE);

    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return -1;

    AGetVoiceStatus(This.m_hVoice[channel], &stopped);
    return stopped;
}

static void SealSound_set_mastervolume(int attenuation)
{
#if AUDIO_SYSTEM_VERSION >= 0x106

    float volume;

    assert(This.m_bInitialized == TRUE);
    
    This.m_nAttenuation = attenuation;
    volume = 256.0f;	/* range is 0-256 */
    while (attenuation++ < 0)
        volume /= 1.122018454f;   /* = (10 ^ (1/20)) = 1dB */
    This.m_nMasterVolume = (int)volume;
	ASetAudioMixerValue(AUDIO_MIXER_MASTER_VOLUME, This.m_nMasterVolume);

#else

    int channel;

    assert(This.m_bInitialized == TRUE);
    
    This.m_nAttenuation = attenuation;
    This.m_fAttenuation_mult = 1.0f;
    while (attenuation++ < 0)
        This.m_fAttenuation_mult /= 1.122018454f;   /* = (10 ^ (1/20)) = 1dB */
    for (channel = 0; channel < MIXER_MAX_CHANNELS; channel++)
        ASetVoiceVolume(This.m_hVoice[channel], (UINT)(This.m_fAttenuation_mult * This.m_nVolume[channel]));

#endif
}

static int SealSound_get_mastervolume()
{
   return This.m_nAttenuation;
}

static void SealSound_sound_enable(int enable)
{

#if AUDIO_SYSTEM_VERSION >= 0x106

	if (enable)
	{
		ASetAudioMixerValue(AUDIO_MIXER_MASTER_VOLUME, This.m_nMasterVolume);
	}
	else
	{
		ASetAudioMixerValue(AUDIO_MIXER_MASTER_VOLUME, 0);
	}

#else

	static int orig_attenuation;

	if (enable)
	{
		osd_set_mastervolume(orig_attenuation);
	}
	else
	{
		if (osd_get_mastervolume() != -40)
		{
			orig_attenuation = osd_get_mastervolume();
			osd_set_mastervolume(-32);
		}
	}

#endif
}



struct tSealSoundDevices* SealSound_GetSoundDevices()
{
    unsigned int i;
    AUDIOCAPS    caps;
    static struct tSealSoundDevices SoundDevices;

    if (0 < SoundDevices.m_nNumDevices)
        return &SoundDevices;

    /*
        "It’s not required to initialize the audio system for
        applications running in Windows 95 or Windows NT."
        But it doesn't hurt.
    */

    if (AInitialize() == AUDIO_ERROR_NONE)
    {
        for (i = 0; i < AGetAudioNumDevs(); i++)
        {
            if (i == MAXSOUNDDEVICES)
                break;

            if (AGetAudioDevCaps(i, &caps) == AUDIO_ERROR_NONE)
            {
                strcpy(SoundDevices.m_Devices[i].m_pName, caps.szProductName);
                SoundDevices.m_Devices[i].m_nDeviceID  = i;
                SoundDevices.m_Devices[i].m_wProductID = caps.wProductId;
                SoundDevices.m_nNumDevices++;
            }
        }
    }
    else
    {
        SoundDevices.m_nNumDevices = 0;
    }

    return &SoundDevices;
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static void play_sample(int channel, signed char *data, int len, int freq, int volume, int loop, int bits)
{
    assert(This.m_bInitialized == TRUE);

    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return;

	/* backwards compatibility with old 0-255 volume range */
	if (volume > 100) volume = volume * 25 / 255;

    if (This.m_lpWave[channel] != 0
    &&  This.m_lpWave[channel]->dwLength != (DWORD)len)
    {
        AStopVoice(This.m_hVoice[channel]);
        ADestroyAudioData(This.m_lpWave[channel]);
        free(This.m_lpWave[channel]);
        This.m_lpWave[channel] = 0;
    }

    if (This.m_lpWave[channel] == 0)
    {
        This.m_lpWave[channel] = (LPAUDIOWAVE)malloc(sizeof(AUDIOWAVE));
        if (This.m_lpWave[channel] == 0)
            return;

        if (loop)
            This.m_lpWave[channel]->wFormat = (bits == 8 ? AUDIO_FORMAT_8BITS : AUDIO_FORMAT_16BITS) |
                                               AUDIO_FORMAT_MONO | AUDIO_FORMAT_LOOP;
        else
            This.m_lpWave[channel]->wFormat = (bits == 8 ? AUDIO_FORMAT_8BITS : AUDIO_FORMAT_16BITS) |
                                               AUDIO_FORMAT_MONO;
        
        This.m_lpWave[channel]->nSampleRate = Machine->sample_rate; /* freq? */
        This.m_lpWave[channel]->dwLength    = len;
        This.m_lpWave[channel]->dwLoopStart = 0;
        This.m_lpWave[channel]->dwLoopEnd   = len;

        if (ACreateAudioData(This.m_lpWave[channel]) != AUDIO_ERROR_NONE)
        {
            free(This.m_lpWave[channel]);
            This.m_lpWave[channel] = 0;
            return;
        }
    }
    else
    {
        if (loop)
            This.m_lpWave[channel]->wFormat = (bits == 8 ? AUDIO_FORMAT_8BITS : AUDIO_FORMAT_16BITS) |
                                               AUDIO_FORMAT_MONO | AUDIO_FORMAT_LOOP;
        else
            This.m_lpWave[channel]->wFormat = (bits == 8 ? AUDIO_FORMAT_8BITS : AUDIO_FORMAT_16BITS) |
                                               AUDIO_FORMAT_MONO;
    }

    memcpy(This.m_lpWave[channel]->lpData, data, len);

    /* upload the data to the audio DRAM local memory */
    AWriteAudioData(This.m_lpWave[channel], 0, len);
    APlayVoice(This.m_hVoice[channel], This.m_lpWave[channel]);
    ASetVoiceFrequency(This.m_hVoice[channel], freq);

#if AUDIO_SYSTEM_VERSION >= 0x106
    ASetVoiceVolume(This.m_hVoice[channel], (UINT)(volume * 64 / 100));
#else
    This.m_nVolume[channel] = volume * 64 / 100;
    ASetVoiceVolume(This.m_hVoice[channel], (UINT)(This.m_fAttenuation_mult * This.m_nVolume[channel]));
#endif
}

static void play_streamed_sample(int channel, signed char *data, int len, int freq, int volume, int pan, int bits)
{
    assert(This.m_bInitialized == TRUE);
    
    if (Machine->sample_rate == 0 || MIXER_MAX_CHANNELS <= channel)
        return;

	/* backwards compatibility with old 0-255 volume range */
	if (volume > 100) volume = volume * 25 / 255;

	if (pan != MIXER_PAN_CENTER) volume /= 2;

    if (This.m_bPlaying[channel] == FALSE)
    {
        if (This.m_lpWave[channel] != 0)
        {
            AStopVoice(This.m_hVoice[channel]);
            ADestroyAudioData(This.m_lpWave[channel]);
            free(This.m_lpWave[channel]);
            This.m_lpWave[channel] = 0;
        }

        This.m_lpWave[channel] = (LPAUDIOWAVE)malloc(sizeof(AUDIOWAVE));
        if (This.m_lpWave[channel] == 0)
            return;

        This.m_lpWave[channel]->wFormat     = (bits == 8 ? AUDIO_FORMAT_8BITS : AUDIO_FORMAT_16BITS) |
                                              AUDIO_FORMAT_MONO | AUDIO_FORMAT_LOOP;
        This.m_lpWave[channel]->nSampleRate = Machine->sample_rate; /* freq ? */
        This.m_lpWave[channel]->dwLength    = 3 * len;
        This.m_lpWave[channel]->dwLoopStart = 0;
        This.m_lpWave[channel]->dwLoopEnd   = 3 * len;

        if (ACreateAudioData(This.m_lpWave[channel]) != AUDIO_ERROR_NONE)
        {
            free(This.m_lpWave[channel]);
            This.m_lpWave[channel] = 0;
            return;
        }

        memset(This.m_lpWave[channel]->lpData, 0x00, 3 * len);
        memcpy(This.m_lpWave[channel]->lpData + 2 * len, data, len);

        /* upload the data to the audio DRAM local memory */
        AWriteAudioData(This.m_lpWave[channel], 0, 3 * len);
        APrimeVoice(This.m_hVoice[channel], This.m_lpWave[channel]);
        ASetVoiceFrequency(This.m_hVoice[channel], freq);
        AStartVoice(This.m_hVoice[channel]);
        
        This.m_bPlaying[channel] = TRUE;
        This.m_c[channel] = 0;
    }
    else
    {
        /*
            If you are using DirectSound emulation
            this code will bring your machine to its knees!
            So avoid it if necessary.
        */
        if (This.m_bPollStream == TRUE
        &&  Display_Throttled() == TRUE) /* sync with audio only when speed throttling is not turned off */
        {
            LONG pos;

            for (;;)
            {
                AGetVoicePosition(This.m_hVoice[channel], &pos);

                if (bits == 16)
                    pos *= 2;

                if (This.m_c[channel] == 0 && len <= pos)
                    break;
                if (This.m_c[channel] == 1 && (pos < len || 2 * len <= pos))
                    break;
                if (This.m_c[channel] == 2 && pos < 2 * len)
                    break;
            }
        }

        memcpy(&This.m_lpWave[channel]->lpData[len * This.m_c[channel]], data, len);
        AWriteAudioData(This.m_lpWave[channel], len * This.m_c[channel], len);
        This.m_c[channel]++;
        if (This.m_c[channel] == 3)
            This.m_c[channel] = 0;
    }

#if AUDIO_SYSTEM_VERSION >= 0x106
    ASetVoiceVolume(This.m_hVoice[channel], (UINT)(volume * 64 / 100));
#else
    This.m_nVolume[channel] = volume * 64 / 100;
    ASetVoiceVolume(This.m_hVoice[channel], (UINT)(This.m_fAttenuation_mult * This.m_nVolume[channel]));
#endif

    if (pan == MIXER_PAN_CENTER)
        ASetVoicePanning(This.m_hVoice[channel], 128);
    else
    if (pan == MIXER_PAN_LEFT)
        ASetVoicePanning(This.m_hVoice[channel], 0);
    else
    if (pan == MIXER_PAN_RIGHT)
        ASetVoicePanning(This.m_hVoice[channel], 255);
}

