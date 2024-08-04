/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  MidasSound.c

 ***************************************************************************/
#include "driver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mame.h"
#include "MAME32.h"
#include "osdepend.h"
#include "MidasSound.h"
#include "M32Util.h"
#include "midasdll.h"

//#define MIXER_MAX_CHANNELS            16
#define STREAM_BUFFER_LENGTH 4096

/***************************************************************************
    function prototypes
 ***************************************************************************/

static int      MidasSound_init(options_type *options);
static void     MidasSound_exit(void);
static void     MidasSound_update_audio(void);
static void     MidasSound_play_sample(int channel, signed char* data, int len, int freq, int volume, int loop);
static void     MidasSound_play_sample_16(int channel, signed short* data, int len, int freq, int volume, int loop);
static void     MidasSound_play_streamed_sample(int channel, signed char* data, int len, int freq, int volume, int pan);
static void     MidasSound_play_streamed_sample_16(int channel, signed short* data, int len, int freq, int volume, int pan);
static void     MidasSound_set_sample_freq(int channel,int freq);
static void     MidasSound_set_sample_volume(int channel,int volume);
static void     MidasSound_stop_sample(int channel);
static void     MidasSound_restart_sample(int channel);
static int      MidasSound_get_sample_status(int channel);
static void     MidasSound_set_mastervolume(int attenuation);
static int      MidasSound_get_mastervolume(void);
static void     MidasSound_sound_enable(int enable);

static MIDASsample MIDASsetRawSample(char *buffer, long len, int sampleType, int loopSample);

/***************************************************************************
    External variables
 ***************************************************************************/

struct OSDSound MIDASSound = 
{
    { MidasSound_init },                    /* init                    */
    { MidasSound_exit },                    /* exit                    */
    { MidasSound_update_audio },            /* update_audio            */
    { MidasSound_play_sample },             /* play_sample             */
    { MidasSound_play_sample_16 },          /* play_sample_16          */
    { MidasSound_play_streamed_sample },    /* play_streamed_sample    */
    { MidasSound_play_streamed_sample_16 }, /* play_streamed_sample_16 */
    { MidasSound_set_sample_freq },         /* adjust_sample_freq      */
    { MidasSound_set_sample_volume },       /* adjust_sample_volume    */
    { MidasSound_stop_sample },             /* stop_sample             */
    { MidasSound_restart_sample },          /* restart_sample          */
    { MidasSound_get_sample_status },       /* get_sample_status       */
    { MidasSound_set_mastervolume },        /* set_mastervolume        */
    { MidasSound_get_mastervolume },        /* get_mastervolume        */
    { MidasSound_sound_enable }             /* sound_enable            */
};

/***************************************************************************
    Internal structures
 ***************************************************************************/

enum VoiceType { Sample, Stream, None };

struct Voice
{
    enum VoiceType          m_eType;
    BOOL                    m_bPlaying;
    unsigned int            m_nVolume;
    
    MIDASstreamHandle       m_hStream;

    MIDASsamplePlayHandle   m_hSample;
    MIDASsample             m_Sample;
    unsigned int            m_nFrequency;
    unsigned int            m_nMidasChannel;
};

struct tSound_private
{
    BOOL                m_bInitialized;
    int                 m_nAttenuation;
    DWORD               m_nMasterVolume;
    struct Voice        m_Voices[MIXER_MAX_CHANNELS];
};

/***************************************************************************
    Internal variables
 ***************************************************************************/

static struct tSound_private    This;
  
/***************************************************************************
    External OSD functions  
 ***************************************************************************/

/*
    put here anything you need to do when the program is started. Return 0 if 
    initialization was successful, nonzero otherwise.
*/
static int MidasSound_init(options_type *options)
{
    int     i;
    BOOL    bResult;
    DWORD   dwOutputMode;
    DWORD   dwMixRate;
    DWORD   dwMixBufLen;
    DWORD   dwMixBufBlocks;
    DWORD   dwFilterMode;
    DWORD   dwMixingMode;
    DWORD   dwDSoundMode;
    DWORD   dwDSoundBufLen;

    This.m_bInitialized     = FALSE;
    This.m_nAttenuation     = 0;
    This.m_nMasterVolume    = 400;
    for (i = 0; i < MIXER_MAX_CHANNELS; i++)
    {
        This.m_Voices[i].m_eType            = None;
        This.m_Voices[i].m_bPlaying         = FALSE;
        This.m_Voices[i].m_nVolume          = 0;
        This.m_Voices[i].m_hStream          = NULL;
        This.m_Voices[i].m_hSample          = 0;
        This.m_Voices[i].m_Sample           = 0;
        This.m_Voices[i].m_nFrequency       = 0;
        This.m_Voices[i].m_nMidasChannel    = 0;
    }

    /* This function must be called before ANY other MIDAS function. */
    bResult = MIDASstartup();
    if (bResult == FALSE)
        goto error;

    /*
        "The output mode should normally be set to a 16-bit mode,
        as using 8-bit modes does not decrease CPU usage, only sound quality.
        Using mono output instead of stereo, however, can decrease
        CPU usage by up to 50%."
    */
    if (options->stereo)
        dwOutputMode    = MIDAS_MODE_16BIT_STEREO;
    else
        dwOutputMode    = MIDAS_MODE_16BIT_MONO;

    dwMixRate       = Machine->sample_rate;
    dwMixBufLen     = 200; /* MIDAS Default of 500 has too much latency. */
    dwMixBufBlocks  = MIDASgetOption(MIDAS_OPTION_MIXBUFBLOCKS);
    dwFilterMode    = MIDASgetOption(MIDAS_OPTION_FILTER_MODE);
    dwMixingMode    = MIDASgetOption(MIDAS_OPTION_MIXING_MODE);
    dwDSoundMode    = MIDAS_DSOUND_STREAM;
    dwDSoundBufLen  = MIDASgetOption(MIDAS_OPTION_DSOUND_BUFLEN);

    /*
        All MIDAS configuration options must be set before MIDASinit() is called.
    */
    MIDASsetOption(MIDAS_OPTION_DSOUND_HWND,    (DWORD)MAME32App.m_hWnd);
    MIDASsetOption(MIDAS_OPTION_OUTPUTMODE,     dwOutputMode);
    MIDASsetOption(MIDAS_OPTION_MIXRATE,        dwMixRate);
    MIDASsetOption(MIDAS_OPTION_MIXBUFLEN,      dwMixBufLen);
    MIDASsetOption(MIDAS_OPTION_MIXBUFBLOCKS,   dwMixBufBlocks);
    MIDASsetOption(MIDAS_OPTION_FILTER_MODE,    dwFilterMode);
    MIDASsetOption(MIDAS_OPTION_MIXING_MODE,    dwMixingMode);
    MIDASsetOption(MIDAS_OPTION_DSOUND_MODE,    dwDSoundMode);
    MIDASsetOption(MIDAS_OPTION_DSOUND_BUFLEN,  dwDSoundBufLen);

    bResult = MIDASinit();
    if (bResult == FALSE)
        goto error;

    bResult = MIDASstartBackgroundPlay(60);
    if (bResult == FALSE)
        goto error;

    bResult = MIDASopenChannels(MIXER_MAX_CHANNELS);
    if (bResult == FALSE)
        goto error;
    
    /* update the Machine structure to reflect the actual sample rate */
    Machine->sample_rate = MIDASgetOption(MIDAS_OPTION_MIXRATE);

/*
    printf("MIDAS Sound Options:\n");
    printf("outputmode:    %d\n", MIDASgetOption(MIDAS_OPTION_OUTPUTMODE));
    printf("mixrate:       %d\n", MIDASgetOption(MIDAS_OPTION_MIXRATE));
    printf("mixbuflen:     %d\n", MIDASgetOption(MIDAS_OPTION_MIXBUFLEN));
    printf("mixbufblocks:  %d\n", MIDASgetOption(MIDAS_OPTION_MIXBUFBLOCKS));
    printf("filtermode:    %d\n", MIDASgetOption(MIDAS_OPTION_FILTER_MODE));
    printf("mixingmode:    %d\n", MIDASgetOption(MIDAS_OPTION_MIXING_MODE));
    printf("dsoundmode:    %d\n", MIDASgetOption(MIDAS_OPTION_DSOUND_MODE));
    printf("dsoundbuflen:  %d\n", MIDASgetOption(MIDAS_OPTION_DSOUND_BUFLEN));
*/

    MIDASsetAmplification(This.m_nMasterVolume);

    This.m_bInitialized = TRUE;
    return 0;

error:
    ErrorMsg("MIDAS: %s", MIDASgetErrorMessage(MIDASgetLastError()));
    /* update the Machine structure to show that sound is disabled */
    Machine->sample_rate = 0;
    return 1;
}

/*
    put here cleanup routines to be executed when the program is terminated.
*/
static void MidasSound_exit(void)
{
    int channel;

    if (This.m_bInitialized == FALSE)
        return;

    for (channel = 0; channel < MIXER_MAX_CHANNELS; channel++)
    {
        if (This.m_Voices[channel].m_eType == Stream)
        {
            if (This.m_Voices[channel].m_hStream != NULL)
                MIDASstopStream(This.m_Voices[channel].m_hStream);
            This.m_Voices[channel].m_hStream = NULL;
        }
        else
        {
            if (This.m_Voices[channel].m_hSample != 0)
                MIDASstopSample(This.m_Voices[channel].m_hSample);
            This.m_Voices[channel].m_hSample = 0;

            if (This.m_Voices[channel].m_Sample != 0)
                MIDASfreeSample(This.m_Voices[channel].m_Sample);
            This.m_Voices[channel].m_Sample = 0;
        }
    }

    MIDAScloseChannels();
    MIDASstopBackgroundPlay();
    MIDASclose();

    This.m_bInitialized = FALSE;
}


static void MidasSound_update_audio(void)
{
    assert(This.m_bInitialized == TRUE);
}

static void MidasSound_play_sample(int channel, signed char *data, int len, int freq, int volume, int loop)
{
    unsigned char*  pBuffer;
    BOOL            bResult;
    int             i;

    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    if (This.m_Voices[channel].m_nMidasChannel == 0)
    {
        This.m_Voices[channel].m_nMidasChannel = MIDASallocateChannel();
        if (This.m_Voices[channel].m_nMidasChannel == MIDAS_ILLEGAL_CHANNEL)
            ErrorMsg("MIDASallocateChannel Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));
    }

    if (This.m_Voices[channel].m_hSample != 0)
    {
        bResult = MIDASstopSample(This.m_Voices[channel].m_hSample);
        if (bResult == FALSE)
            ErrorMsg("MIDASstopSample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));

        This.m_Voices[channel].m_hSample = 0;
    }

    if (This.m_Voices[channel].m_Sample != 0)
    {
        bResult = MIDASfreeSample(This.m_Voices[channel].m_Sample);
        if (bResult == FALSE)
            ErrorMsg("MIDASfreeSample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));
    }

    pBuffer = (unsigned char*) malloc(len * sizeof(unsigned char));
    if (pBuffer == NULL)
    {
        ErrorMsg("malloc failed");
        return;
    }

    /*
        8 bit samples are signed in MAME, so convert.
        MIDAS_SAMPLE_8BIT_MONO:   8-bit mono sample, unsigned
    */
    for (i = 0; i < len; i++)
        pBuffer[i] = *data++ - 128;

    This.m_Voices[channel].m_nFrequency = freq;
    This.m_Voices[channel].m_nVolume    = volume * 64 / 100;
    This.m_Voices[channel].m_Sample = MIDASsetRawSample(pBuffer,
                                                        len,
                                                        MIDAS_SAMPLE_8BIT_MONO,
                                                        (loop == 1) ? MIDAS_LOOP_YES : MIDAS_LOOP_NO);
    if (This.m_Voices[channel].m_Sample == 0)
        ErrorMsg("MIDASsetRawSample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));

    free(pBuffer);

    This.m_Voices[channel].m_hSample = MIDASplaySample(
                                        This.m_Voices[channel].m_Sample,
                                        This.m_Voices[channel].m_nMidasChannel,
                                        100,
                                        This.m_Voices[channel].m_nFrequency,
                                        This.m_Voices[channel].m_nVolume,
                                        MIDAS_PAN_MIDDLE);

    if (This.m_Voices[channel].m_hSample == 0)
        ErrorMsg("MIDASplaySample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));

    This.m_Voices[channel].m_eType = Sample;
}

/* NOTE: THIS HAS NOT BEED TESTED! */
static void MidasSound_play_sample_16(int channel, signed short *data, int len, int freq, int volume, int loop)
{
    BOOL            bResult;

    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    if (This.m_Voices[channel].m_nMidasChannel == 0)
    {
        This.m_Voices[channel].m_nMidasChannel = MIDASallocateChannel();
        if (This.m_Voices[channel].m_nMidasChannel == MIDAS_ILLEGAL_CHANNEL)
            ErrorMsg("MIDASallocateChannel Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));
    }

    if (This.m_Voices[channel].m_hSample != 0)
    {
        bResult = MIDASstopSample(This.m_Voices[channel].m_hSample);
        if (bResult == FALSE)
            ErrorMsg("MIDASstopSample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));

        This.m_Voices[channel].m_hSample = 0;
    }

    if (This.m_Voices[channel].m_Sample != 0)
    {
        bResult = MIDASfreeSample(This.m_Voices[channel].m_Sample);
        if (bResult == FALSE)
            ErrorMsg("MIDASfreeSample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));
    }

    /*
        MIDAS_SAMPLE_16BIT_MONO: 16-bit mono sample, signed
    */

    This.m_Voices[channel].m_nFrequency = freq;
    This.m_Voices[channel].m_nVolume    = volume * 64 / 100;
    This.m_Voices[channel].m_Sample = MIDASsetRawSample((char *)data,
                                                        len,
                                                        MIDAS_SAMPLE_16BIT_MONO,
                                                        (loop == 1) ? MIDAS_LOOP_YES : MIDAS_LOOP_NO);
    if (This.m_Voices[channel].m_Sample == 0)
        ErrorMsg("MIDASsetRawSample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));

    This.m_Voices[channel].m_hSample = MIDASplaySample(
                                        This.m_Voices[channel].m_Sample,
                                        This.m_Voices[channel].m_nMidasChannel,
                                        100,
                                        This.m_Voices[channel].m_nFrequency,
                                        This.m_Voices[channel].m_nVolume,
                                        MIDAS_PAN_MIDDLE);

    if (This.m_Voices[channel].m_hSample == 0)
        ErrorMsg("MIDASplaySample Failed: %s", MIDASgetErrorMessage(MIDASgetLastError()));

    This.m_Voices[channel].m_eType = Sample;
}

static void MidasSound_play_streamed_sample(int channel, signed char *data, int len, int freq, int volume, int pan)
{
    static unsigned char buf[STREAM_BUFFER_LENGTH];
    int i;
    int n, r;

    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    if (This.m_Voices[channel].m_bPlaying == FALSE)
    {
        /* Stream playback buffer length in milliseconds */
        unsigned int nBufLength = (unsigned int)(8.0 * len / freq * 1000.0);

        /* MIDAS_SAMPLE_8BIT_MONO: 8-bit mono sample, unsigned */
        This.m_Voices[channel].m_hStream = MIDASplayStreamPolling(MIDAS_SAMPLE_8BIT_MONO,
                                                                  freq,
                                                                  nBufLength);

        if (This.m_Voices[channel].m_hStream == NULL)
        {
            ErrorMsg("MIDASplayStreamPolling: %s", MIDASgetErrorMessage(MIDASgetLastError()));
            return;
        }
        
        MIDASsetStreamRate(This.m_Voices[channel].m_hStream, freq);
        This.m_Voices[channel].m_bPlaying = TRUE;
        This.m_Voices[channel].m_eType    = Stream;
    }

    n = len / STREAM_BUFFER_LENGTH;
    r = len % STREAM_BUFFER_LENGTH;

    while (n--)
    {
        for (i = 0; i < STREAM_BUFFER_LENGTH / 4; i++)
            ((DWORD*)buf)[i] = *((DWORD*)data)++ ^ 0x80808080;

        MIDASfeedStreamData(This.m_Voices[channel].m_hStream, buf, STREAM_BUFFER_LENGTH, FALSE);
    }

    for (i = 0; i < (r / 4); i++)
        ((DWORD*)buf)[i] = *((DWORD*)data)++ ^ 0x80808080;

    i *= 4;
    n = r % 4;
    while (n--)
        buf[i++] = *data++ ^ 0x80;

    MIDASfeedStreamData(This.m_Voices[channel].m_hStream, buf, r, FALSE);

    This.m_Voices[channel].m_nVolume  = volume * 64 / 100;
    MIDASsetStreamVolume(This.m_Voices[channel].m_hStream,
                         This.m_Voices[channel].m_nVolume);
    if (pan == MIXER_PAN_CENTER)
        MIDASsetStreamPanning(This.m_Voices[channel].m_hStream, MIDAS_PAN_MIDDLE);
    else
    if (pan == MIXER_PAN_LEFT)
        MIDASsetStreamPanning(This.m_Voices[channel].m_hStream, MIDAS_PAN_LEFT);
    else
    if (pan == MIXER_PAN_RIGHT)
        MIDASsetStreamPanning(This.m_Voices[channel].m_hStream, MIDAS_PAN_RIGHT);
}

static void MidasSound_play_streamed_sample_16(int channel, signed short *data, int len, int freq, int volume, int pan)
{
    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    if (This.m_Voices[channel].m_bPlaying == FALSE)
    {
        /* Stream playback buffer length in milliseconds */
        unsigned int nBufLength = (unsigned int)(8.0 * (len / 2.0) / freq * 1000.0);

        /* MIDAS_SAMPLE_16BIT_MONO: 16-bit mono sample, signed */
        This.m_Voices[channel].m_hStream = MIDASplayStreamPolling(MIDAS_SAMPLE_16BIT_MONO,
                                                                  freq,
                                                                  nBufLength);

        if (This.m_Voices[channel].m_hStream == NULL)
        {
            ErrorMsg("MIDASplayStreamPolling: %s", MIDASgetErrorMessage(MIDASgetLastError()));
            return;
        }
        
        MIDASsetStreamRate(This.m_Voices[channel].m_hStream, freq);
        This.m_Voices[channel].m_bPlaying = TRUE;
        This.m_Voices[channel].m_eType    = Stream;
    }

    MIDASfeedStreamData(This.m_Voices[channel].m_hStream, (unsigned char*)data, len, FALSE);

    This.m_Voices[channel].m_nVolume  = volume * 64 / 100;
    MIDASsetStreamVolume(This.m_Voices[channel].m_hStream,
                         This.m_Voices[channel].m_nVolume);
    if (pan == MIXER_PAN_CENTER)
        MIDASsetStreamPanning(This.m_Voices[channel].m_hStream, MIDAS_PAN_MIDDLE);
    else
    if (pan == MIXER_PAN_LEFT)
        MIDASsetStreamPanning(This.m_Voices[channel].m_hStream, MIDAS_PAN_LEFT);
    else
    if (pan == MIXER_PAN_RIGHT)
        MIDASsetStreamPanning(This.m_Voices[channel].m_hStream, MIDAS_PAN_RIGHT);
}

static void MidasSound_set_sample_freq(int channel,int freq)
{
    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    if (This.m_Voices[channel].m_eType == Stream)
    {
        if (This.m_Voices[channel].m_hStream != NULL)
        {
            if (freq != -1)
                MIDASsetStreamRate(This.m_Voices[channel].m_hStream, freq);
        }
    }
    else
    {
        if (This.m_Voices[channel].m_hSample != 0)
        {
            if (freq != -1)
                MIDASsetSampleRate(This.m_Voices[channel].m_hSample, freq);
        }
    }
}

static void MidasSound_set_sample_volume(int channel,int volume)
{
    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    This.m_Voices[channel].m_nVolume = volume * 64 / 100;

    if (This.m_Voices[channel].m_eType == Stream)
    {
        if (This.m_Voices[channel].m_hStream != NULL)
        {
            if (volume != -1)
                MIDASsetStreamVolume(This.m_Voices[channel].m_hStream,
                                     This.m_Voices[channel].m_nVolume);
        }
    }
    else
    {
        if (This.m_Voices[channel].m_hSample != 0)
        {
            if (volume != -1)
                MIDASsetSampleVolume(This.m_Voices[channel].m_hSample,
                                     This.m_Voices[channel].m_nVolume);
        }
    }
}

static void MidasSound_stop_sample(int channel)
{
    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    if (This.m_Voices[channel].m_eType == Stream)
    {
        if (This.m_Voices[channel].m_hStream != NULL)
            MIDASpauseStream(This.m_Voices[channel].m_hStream);
    }
    else
    {
        if (This.m_Voices[channel].m_hSample != 0)
            MIDASstopSample(This.m_Voices[channel].m_hSample);
    }
}

static void MidasSound_restart_sample(int channel)
{
    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return;

    if (This.m_Voices[channel].m_eType == Stream)
    {
        if (This.m_Voices[channel].m_hStream != NULL)
            MIDASresumeStream(This.m_Voices[channel].m_hStream);
    }
    else
    {
        if (This.m_Voices[channel].m_hSample != 0)
        {
            MIDASplaySample(This.m_Voices[channel].m_hSample,
                            This.m_Voices[channel].m_nMidasChannel,
                            100,
                            This.m_Voices[channel].m_nFrequency,
                            This.m_Voices[channel].m_nVolume,
                            MIDAS_PAN_MIDDLE);
        }
    }
}

static int MidasSound_get_sample_status(int channel)
{
    int     stopped = 1;

    assert(This.m_bInitialized == TRUE);

    if (MIXER_MAX_CHANNELS <= channel)
        return -1;

    if (This.m_Voices[channel].m_eType == Stream)
    {
        stopped = 1;
    }
    else
    {
        if (This.m_Voices[channel].m_hSample != 0)
        {
            DWORD   status;
            status = MIDASgetSamplePlayStatus(This.m_Voices[channel].m_hSample);
            if (status == MIDAS_SAMPLE_STOPPED)
                stopped = 1;
            else
            if (status == MIDAS_SAMPLE_PLAYING)
                stopped = 0;
        }
    }
    return stopped;
}

/* attenuation in dB */
static void MidasSound_set_mastervolume(int attenuation)
{
    float volume;

    assert(This.m_bInitialized == TRUE);

    This.m_nAttenuation = attenuation;

    volume = 400.0f;
    while (attenuation++ < 0)
        volume /= 1.122018454f;   /* = (10 ^ (1/20)) = 1dB */

    This.m_nMasterVolume = (int)volume;
    MIDASsetAmplification(This.m_nMasterVolume);
}

static int MidasSound_get_mastervolume()
{
    return This.m_nAttenuation;
}

static void MidasSound_sound_enable(int enable)
{
    assert(This.m_bInitialized == TRUE);

	if (enable)
        MIDASsetAmplification(This.m_nMasterVolume);
	else
        MIDASsetAmplification(0);
}



/***************************************************************************
    Internal functions
 ***************************************************************************/

/*
    fxSetRawSample and MIDASsetRawSample are temporarily implemented here.
    According to Petteri Kangaslampi, similar functionality will be in
    the next version of MIDAS. At that time, these methods can be removed
    and mame can use the new midas dll.
*/

#if defined(cdecl)
#undef cdecl
#endif
#define cdecl __cdecl

#include "midas.h"

/****************************************************************************\
*
* Function:     int fxSetRawSample(char *buffer, long len, unsigned sampleType,
*               int loopSample, unsigned *sampleHandle)
*
* Description:  Loads a raw sample from memory and adds the sample to
*               the sound device.
*
* Input:        char *buffer            sample data
*               long len                sample data length
*               unsigned sampleType     sample type, see enum sdSampleType
*               int loopSample          1 if sample is looped, 0 if not
*               unsigned *sampleHandle  pointer to sample handle variable
*
* Returns:      MIDAS error code. The sample handle for the sample will be
*               written to *sampleHandle.
*
\****************************************************************************/

static int fxSetRawSample(char *buffer, long len, unsigned sampleType,
    int loopSample, unsigned *sampleHandle)
{
    int             error;
    static sdSample smp;

    /* Build Sound Device sample structure for the sample: */
    smp.sample       = buffer;
    smp.samplePos    = sdSmpConv;
    smp.sampleType   = sampleType;
    smp.sampleLength = len;

    if ( loopSample )
    {
        /* Loop the whole sample: */
        smp.loopMode   = sdLoop1;
        smp.loop1Start = 0;
        smp.loop1End   = len;
        smp.loop1Type  = loopUnidir;
    }
    else
    {
        /* No loop: */
        smp.loopMode   = sdLoopNone;
        smp.loop1Start = smp.loop1End = 0;
        smp.loop1Type  = loopNone;
    }

    /* No loop 2: */
    smp.loop2Start = smp.loop2End = 0;
    smp.loop2Type  = loopNone;

    /* Add the sample to the Sound Device: */
    if ((error = midasSD->AddSample(&smp, 1, sampleHandle)) != OK)
    {
        memFree(buffer);
        PASSERROR(ID_fx);
    }

    return OK;
}

/****************************************************************************\
*
* Function:     MIDASsample MIDASsetRawSample(char *buffer, long len,
*                   int sampleType, int loopSample)
*
* Description:  Loads a raw sound effect sample from memory
*
* Input:        char *buffer            sample data
*               long len                sample data length
*               int sampleType          sample type
*               int loopSample          1 if sample should be looped
*
* Returns:      MIDAS sample handle, NULL if failed
*
\****************************************************************************/

static MIDASsample MIDASsetRawSample(char *buffer, long len, int sampleType,
    int loopSample)
{
    extern int      mLastError;
    int             error;
    static unsigned sampleHandle;

    /* Load the sample: */
    if ((error = fxSetRawSample(buffer, len, sampleType, loopSample,
        &sampleHandle)) != OK )
    {
        mLastError = error;
        return 0;
    }

    return sampleHandle;
}
