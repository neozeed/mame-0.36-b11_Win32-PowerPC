/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997-98 Michael Soderstrom and Chris Kirmse
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef __SOUND_H__
#define __SOUND_H__

struct OSDSound
{
    int     (*init)(options_type *options);
    void    (*exit)(void);
    void    (*update_audio)(void);
    void    (*play_sample)(int channel, signed char *data,int len,int freq,int volume,int loop);
    void    (*play_sample_16)(int channel, signed short *data,int len,int freq,int volume,int loop);
    void    (*play_streamed_sample)(int channel, signed char *data,int len,int freq,int volume, int pan);
    void    (*play_streamed_sample_16)(int channel, signed short *data,int len,int freq,int volume, int pan);
    void    (*set_sample_freq)(int channel,int freq);
    void    (*set_sample_volume)(int channel,int volume);
    void    (*stop_sample)(int channel);
    void    (*restart_sample)(int channel);
    int     (*get_sample_status)(int channel);
    void    (*set_mastervolume)(int volume);
    int     (*get_mastervolume)();
    void    (*sound_enable)(int enable);
};

extern struct OSDSound Sound;

#endif
