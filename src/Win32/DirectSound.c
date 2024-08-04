/***************************************************************************

    M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
    Win32 Portions Copyright (C) 1997 Michael Soderstrom and Chris Kirmse
                                and Mohammad Rezaei
    
    This file is part of MAME32, and may only be used, modified and
    distributed under the terms of the MAME license, in "readme.txt".
    By continuing to use, modify or distribute this file you indicate
    that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  DirectSound.c
 ***************************************************************************/

#include "driver.h"

//#define DS_DEBUG1

#define WIN32_LEAN_AND_MEAN
#include <math.h>
#include <windows.h>
#include <mmreg.h>
#include <dsound.h>
#include "mame.h"
#include "MAME32.h"
#include "osdepend.h"
#include "M32Util.h"
#include "DirectSound.h"
/***************************************************************************
    function prototypes
 ***************************************************************************/

static int      DirectSound_init(options_type *options);
static void     DirectSound_exit(void);
static void     DirectSound_update_audio(void);
static void     DirectSound_play_sample(int channel, signed char* datao, int len, int freq, int volume, int loop);
static void     DirectSound_play_sample_16(int channel, signed short* data, int len, int freq, int volume, int loop);
static void     DirectSound_play_streamed_sample(int channel, signed char* data, int len, int freq, int volume, int pan);
static void     DirectSound_play_streamed_sample_16(int channel, signed short* data, int len, int freq, int volume, int pan);
static void     DirectSound_set_sample_freq(int channel,int freq);
static void     DirectSound_set_sample_volume(int channel,int volume);
static void     DirectSound_stop_sample(int channel);
static void     DirectSound_restart_sample(int channel);
static int      DirectSound_get_sample_status(int channel);
static void     DirectSound_set_mastervolume(int volume);
static int      DirectSound_get_mastervolume();
static void     DirectSound_sound_enable(int enable);

/*
static int FindFreeChannel();
*/
static BOOL AppCreateBasicBuffer( 
    LPDIRECTSOUND lpDirectSound, 
    LPDIRECTSOUNDBUFFER *lplpDsb, int len, int flags,int freq, int bits, int channels);
static BOOL AppWriteDataToBuffer( 
    LPDIRECTSOUNDBUFFER lpDsb,  // the DirectSound buffer
    DWORD dwOffset,             // our own write cursor
    LPBYTE lpbSoundData,        // start of our data
    DWORD dwSoundBytes);         // size of block to copy
static BOOL AppWriteDataToBufferNow( 
    LPDIRECTSOUNDBUFFER lpDsb,  // the DirectSound buffer
    LPBYTE lpbSoundData,        // start of our data
    DWORD dwSoundBytes);         // size of block to copy
BOOL AppCreatePrimaryBuffer( 
    LPDIRECTSOUND lpDirectSound, 
    LPDIRECTSOUNDBUFFER *lplpDsb, 
    LPDWORD lpdwBufferSize, 
    HWND hwnd,int freq, int bits);
static void set_freq(int channel,int freq);
static void set_vol(int channel,int volume);
static void set_pan(int channel,int pan);

/***************************************************************************
    External variables
 ***************************************************************************/

struct OSDSound DirectSound = 
{
    { DirectSound_init },                     /* init                    */
    { DirectSound_exit },                     /* exit                    */
    { DirectSound_update_audio },             /* update_audio            */
    { DirectSound_play_sample },              /* play_sample             */
    { DirectSound_play_sample_16 },           /* play_sample_16          */
    { DirectSound_play_streamed_sample },     /* play_streamed_sample    */
    { DirectSound_play_streamed_sample_16 },  /* play_streamed_sample_16 */
    { DirectSound_set_sample_freq },          /* adjust_sample_freq      */
    { DirectSound_set_sample_volume },        /* adjust_sample_volume    */
    { DirectSound_stop_sample },              /* stop_sample             */
    { DirectSound_restart_sample },           /* restart_sample          */
    { DirectSound_get_sample_status },        /* get_sample_status       */
    { DirectSound_set_mastervolume },         /* set_mastervolume        */
    { DirectSound_get_mastervolume },         /* get_mastervolume        */
    { DirectSound_sound_enable }              /* sound_enable            */
};

/***************************************************************************
    Internal structures
 ***************************************************************************/
//#define MIXER_MAX_CHANNELS   32

#define BUFFER_SECONDS 3
#define FRAMES_AHEAD 8
#define HIGH_QUALITY

#define STATUS_NORMAL 0
#define STATUS_EXTEND 1000
#define STATUS_DROP 10
#define STATUS_RESTART 20


enum VoiceType { Sample, Stream, None };

struct Voice
{
    enum VoiceType          type;
    BOOL                    playing;
    int                     volume;
    int                     length;
    int                     allocated;
    LPDIRECTSOUNDBUFFER    ds_buffer;
    int                     pos;
    int                     bits;
    int                     loop;


    int                     buffer_len;
    int                     freq;
    unsigned char           last_data_byte;
    signed short            last_data_short;
	int						frame_len;
    int                     numChannels;
    int                     pan;
    int                     last_insert;

#ifdef DS_DEBUG1
    int             frame;
	DWORD			start_time;
	DWORD			last_time;
#endif
};
/***************************************************************************
    Internal variables
 ***************************************************************************/
// 44100/5
#define SILENCE8_LENGTH 8820
#define SILENCE16_LENGTH 8820

static HANDLE hDLL;
static struct Voice voices[MIXER_MAX_CHANNELS];
static struct Voice stopped_voices[MIXER_MAX_CHANNELS];
static LPDIRECTSOUND lpDirectSound=NULL;
static int attenuation=0;
static int numChannels;
static int video_fps;
static unsigned char silence8[SILENCE8_LENGTH];
static signed short silence16[SILENCE16_LENGTH];


#ifdef DS_DEBUG1
static FILE *debug_fd[MIXER_MAX_CHANNELS];
#endif

#ifdef RECORD
static FILE *fd[MIXER_MAX_CHANNELS];
#endif


/***************************************************************************
    External OSD functions  
 ***************************************************************************/

typedef HRESULT (WINAPI *dsc_proc)(GUID FAR *lpGUID,LPDIRECTSOUND FAR *lplpDS,
                                   IUnknown FAR *pUnkOuter);

static int DirectSound_init(options_type *options)
{
    int i;
    HRESULT       hr;
    LPDIRECTSOUNDBUFFER primary=NULL;
    DWORD buffersize;
    UINT error_mode;
    dsc_proc dsc;

    /* Turn off error dialog for this call */
    error_mode = SetErrorMode(0);
    hDLL = LoadLibrary("dsound.dll");
    SetErrorMode(error_mode);

    if (hDLL == NULL)
       return 1;

    dsc = (dsc_proc)GetProcAddress(hDLL,"DirectSoundCreate");
    if (dsc == NULL)
        return 1;

    memset(silence8,0x80,SILENCE8_LENGTH);
    memset(silence16,0x0,SILENCE16_LENGTH*2);

    hr = dsc(NULL, &lpDirectSound, NULL);

    if (FAILED(hr))
    {
       ErrorMsg("Unable to initialize DirectSound");
       return 1;
    }

	attenuation = 0;

    if (!AppCreatePrimaryBuffer(lpDirectSound,&primary,&buffersize,
        MAME32App.m_hWnd,Machine->sample_rate,Machine->sample_bits))
    {
        ErrorMsg("Couldn't get primary buffer");
    }
    else
    {
        IDirectSoundBuffer_Play(primary,0,0,DSBPLAY_LOOPING);
        IDirectSoundBuffer_Release(primary);
    }

    hr = lpDirectSound->lpVtbl->SetCooperativeLevel(
                      lpDirectSound, MAME32App.m_hWnd, DSSCL_PRIORITY);
	numChannels = 1;
	if (options->stereo) numChannels = 2;
    for(i=0;i<MIXER_MAX_CHANNELS;i++)
    {
        voices[i].type = None;
        voices[i].playing=0;
        voices[i].volume = 0;
        voices[i].length = 0;
        voices[i].allocated = 0;
        voices[i].freq = 0;
        voices[i].pos = 0;
        voices[i].ds_buffer = NULL;
        voices[i].buffer_len = 0;
        voices[i].bits = 0;
        voices[i].loop = 0;
		voices[i].frame_len = 0;
        voices[i].numChannels = 0;
        voices[i].pan = MIXER_PAN_CENTER;
        voices[i].last_insert = 0;
#ifdef DS_DEBUG1
        voices[i].frame = 0;
#endif
    }
	video_fps = Machine->drv->frames_per_second;
#ifdef DS_DEBUG1
	{
        {
            char fname[100];
			for(i=0;i<MIXER_MAX_CHANNELS;i++)
			{
	            sprintf(fname,"dsdebug.%d",i);  
		        debug_fd[i]=fopen(fname,"a+b");
			}
        }
	}
#endif  

    
	
	return 0;
}
/*
The DirectSound mixer produces the best sound quality if all your 
application's sounds use the same wave format and the hardware output 
format is matched to the format of the sounds. If this is done, 
the mixer need not perform any format conversion.

Your application can change the hardware output format by creating 
a primary sound buffer and calling the IDirectSoundBuffer::SetFormat 
method. Note that this primary buffer is for control purposes only; 
creating it is not the same as obtaining write access to the primary 
buffer as described under Access to the Primary Buffer, and you do 
not need the DSSCL_WRITEPRIMARY cooperative level. However, you do 
need a cooperative level of DSSCL_PRIORITY or higher in order to call 
the SetFormat method. DirectSound will restore the hardware format 
to the format specified in the last call every time the application 
gains the input focus.}
*/

static void DirectSound_exit(void)
{
    int i;

    if (hDLL == NULL)
       return;

    for(i=0;i<MIXER_MAX_CHANNELS;i++)
    {
        if (voices[i].allocated  && voices[i].ds_buffer!=NULL)
        {
            if (voices[i].type == Stream)
            {
                IDirectSoundBuffer_Stop(voices[i].ds_buffer);

#ifdef RECORD
                fclose(fd[i]);
#endif
#ifdef DS_DEBUG1
				fclose(debug_fd[i]);
#endif

            }
            IDirectSound_Release(voices[i].ds_buffer);
        }
    }

    if (lpDirectSound!=NULL) 
    {
        IDirectSound_Release(lpDirectSound);
        lpDirectSound=NULL;
    }
}

static void DirectSound_update_audio(void)
{
}

static void DirectSound_play_sample(int channel, signed char *data, int len, int freq, int volume, int loop)
{
    int i;
    unsigned char *pBuffer;

#ifdef DS_DEBUG1
    fprintf(debug_fd[channel],"8 bit sample at channel %d len %d freq %d",channel,len,freq);
#endif


    if (voices[channel].allocated)
    {
        DWORD status;

        IDirectSoundBuffer_GetStatus(voices[channel].ds_buffer,&status);
        if ((status & DSBSTATUS_PLAYING) && voices[channel].loop)
        {
            IDirectSoundBuffer_Stop(voices[channel].ds_buffer);
        }
        if (len!=voices[channel].length || (status & DSBSTATUS_BUFFERLOST)
            || voices[channel].type!=Sample)
        {
            IDirectSoundBuffer_Release(voices[channel].ds_buffer);
            goto RESTART;
        }
        IDirectSoundBuffer_SetCurrentPosition(voices[channel].ds_buffer,0);
    }
    else
    {
RESTART:
        if (AppCreateBasicBuffer(lpDirectSound,&(voices[channel].ds_buffer),
            len,0/*DSBCAPS_STATIC*/,freq,8,1)==FALSE)
        {
            ErrorMsg("Counldn't allocate sound buffer");
            return;
        }
        voices[channel].length=len;
        voices[channel].freq=freq;
        voices[channel].buffer_len = len;
        voices[channel].allocated=1;
        voices[channel].type = Sample;
        voices[channel].bits = 8;
        voices[channel].loop = loop;
    }

    pBuffer = (unsigned char *) malloc(len * sizeof(unsigned char)+8);
    for (i = 0; i < len; i++)
    {
        pBuffer[i] = (*data) - 128;
        data++;
    }
    DirectSound_set_sample_freq(channel,freq);
    DirectSound_set_sample_volume(channel,volume);
    AppWriteDataToBuffer(voices[channel].ds_buffer,0,pBuffer,len);
    IDirectSoundBuffer_Play(voices[channel].ds_buffer,0,0,(loop>0)*DSBPLAY_LOOPING);
    free(pBuffer);
}

static void DirectSound_play_sample_16(int channel, signed short *data, int len, int freq, int volume, int loop)
{

#ifdef DS_DEBUG1
    fprintf(debug_fd[channel],"16 bit sample at channel %d len %d freq %d",channel,len,freq);
#endif


    if (voices[channel].allocated)
    {
        DWORD status;

        IDirectSoundBuffer_GetStatus(voices[channel].ds_buffer,&status);
        if ((status & DSBSTATUS_PLAYING) && voices[channel].loop)
        {
            IDirectSoundBuffer_Stop(voices[channel].ds_buffer);
        }
        if (len!=voices[channel].length || (status & DSBSTATUS_BUFFERLOST)
            || voices[channel].type!=Sample)
        {
            IDirectSoundBuffer_Release(voices[channel].ds_buffer);
            goto RESTART;
        }
        IDirectSoundBuffer_SetCurrentPosition(voices[channel].ds_buffer,0);
    }
    else
    {
RESTART:
        if (AppCreateBasicBuffer(lpDirectSound,&(voices[channel].ds_buffer),
            len,0/*DSBCAPS_STATIC*/,freq,16,1)==FALSE)
        {
            ErrorMsg("Counldn't allocate sound buffer");
            return;
        }
        voices[channel].length=len;
        voices[channel].freq=freq;
        voices[channel].buffer_len = len;
        voices[channel].allocated=1;
        voices[channel].type = Sample;
        voices[channel].bits = 16;
        voices[channel].loop = loop;
    }

    DirectSound_set_sample_freq(channel,freq);
    DirectSound_set_sample_volume(channel,volume);
    AppWriteDataToBuffer(voices[channel].ds_buffer,0,(unsigned char *)data,len);
    IDirectSoundBuffer_Play(voices[channel].ds_buffer,0,0,(loop>0)*DSBPLAY_LOOPING);
}

/*
checkposition:
 STATUS_NORMAL means normal
 STATUS_EXTEND+(insert multipier) means extend the frame
 STATUS_DROP means drop the frame
 STATUS_RESTART means restart - no longer used
*/

static int check_position(int channel, int is_16_bit)
{
        int current_status = STATUS_NORMAL;
#ifdef DS_DEBUG1
	    LARGE_INTEGER   tick_count;

        QueryPerformanceCounter(&tick_count);
#endif
    {
        int buffer_len = voices[channel].buffer_len;
        int dist;
        int multiplier = is_16_bit? 1 : 2;
        DWORD play_pos;
        DWORD write_pos;
        

        IDirectSoundBuffer_GetCurrentPosition(voices[channel].ds_buffer,
            &play_pos,&write_pos);

        dist = voices[channel].pos - play_pos;
        if (dist < 0 && (int)play_pos+(FRAMES_AHEAD*voices[channel].frame_len*multiplier) > buffer_len
            && voices[channel].pos < (FRAMES_AHEAD*voices[channel].frame_len*multiplier)) 
        {
            dist += buffer_len;
        }


        /* dist is the distance between the last written byte and current play position 
           it should ideally be (FRAMES_AHEAD/2-1)*voices[channel.frame_len
        */
        if (dist<0)
        {
            /* oops playing past where we're supposed to */
            int insert;
            voices[channel].pos = (write_pos+voices[channel].frame_len) % buffer_len;
            insert = (FRAMES_AHEAD+1)*voices[channel].frame_len/2;
            current_status = STATUS_EXTEND+insert;

#ifdef DS_DEBUG1
            fprintf(debug_fd[channel],"playing past at frame %d delta time %d, 16bit? %d forced reposition\n",
                voices[channel].frame,tick_count.LowPart-voices[channel].start_time, is_16_bit);
#endif

        }
        else if (dist<(FRAMES_AHEAD/2-1)*voices[channel].frame_len)
        { 
            /* not enough data, so let's extend this frame */
            int insert;
                insert = (FRAMES_AHEAD/2-1)*voices[channel].frame_len-dist;
            current_status = STATUS_EXTEND+insert;
#ifdef DS_DEBUG1
            fprintf(debug_fd[channel],"extending frame %d by %d delta time %d, 16bit? %d\n",
                voices[channel].frame,insert,tick_count.LowPart-voices[channel].start_time,is_16_bit);
#endif

        }
        //else if (dist>(FRAMES_AHEAD/2+1)*voices[channel].frame_len)
        else if (dist>(FRAMES_AHEAD-1)*voices[channel].frame_len)
        { 
            /* too much data, not enough time: we'll drop this frame */
            current_status = STATUS_DROP;
#ifdef DS_DEBUG1
            fprintf(debug_fd[channel],"dropping frame %d delta time %d, 16bit? %d\n",
                voices[channel].frame,tick_count.LowPart-voices[channel].start_time, is_16_bit);
#endif


        }
        else
        {
            current_status = STATUS_NORMAL;
            /* normal case */
        }
    }
    return current_status;
}

static void DirectSound_play_streamed_sample(int channel, signed char *data, int len, int freq, int volume, int pan)
{
    int i;
    int status;
    int end_byte = data[len-1]-128;


    unsigned char *pBuffer;



    if (voices[channel].allocated)
    {
        int buffer_len = voices[channel].buffer_len;
        int safety = 0;
//      IDirectSoundBuffer_Stop(voices[channel].ds_buffer);
//		if (len!=voices[channel].length) ErrorMsg("Start %d Now %d",voices[channel].length,len);



#ifdef DS_DEBUG1
	    LARGE_INTEGER   tick_count;

        QueryPerformanceCounter(&tick_count);

		if (len < voices[channel].frame_len-1 || len > voices[channel].frame_len+1)
        {
            fprintf(debug_fd[channel],"Unexpected len %d at frame %d delta time %d\n",
                len,voices[channel].frame,tick_count.LowPart-voices[channel].start_time);
        }
	    voices[channel].frame++;	
#endif

#ifdef RECORD
    fwrite(data,sizeof(unsigned char),len,fd[channel]);
#endif  

        if (voices[channel].pan!=pan) set_pan(channel,pan);
        if (volume!=voices[channel].volume)
            set_vol(channel,volume);

#define RAMP_LENGTH 10

        /* dist is the distance between the last written byte and current play position 
           it should ideally be (FRAMES_AHEAD/2-1)*voices[channel.frame_len
        */
        status = check_position(channel,0);
        switch(status)
        {
            case STATUS_DROP:
            {
                int i;
                unsigned char temp;
                int last,next,now;

                temp=data[len-1]-128;
                len = RAMP_LENGTH;

                pBuffer = (unsigned char *) malloc(len* sizeof(unsigned char)+8);
                last=voices[channel].last_data_byte;
                next=temp;
                for(i=0;i<len;i++)
                {
                    now=(next-last)*(i+1)/len+last;
                    pBuffer[i]=now;
                }
                voices[channel].last_insert = 0;
                break;
            }

            case STATUS_NORMAL:
                pBuffer = (unsigned char *) malloc(len* sizeof(unsigned char)+8);
                for (i = 0; i < len; i++)
                {
                    pBuffer[i] = (*data) - 128;
                    data++;
                }
                voices[channel].last_insert = 0;
                break;
            

            default: // STATUS_EXTEND
            {

                int insert = status - STATUS_EXTEND;
                int i;

                if (voices[channel].last_insert+insert > voices[channel].frame_len/2)
                { // we're running the risk of playing past the buffered stuff.
                    safety = voices[channel].frame_len+voices[channel].last_insert+insert;
                }
                voices[channel].last_insert = insert;

                pBuffer = (unsigned char *) malloc((len+insert+safety)*
                    sizeof(char)+8);
                for(i=0;i<len;i++)
                {
                    int last;
                    last = *data - 128;
                    pBuffer[i] = last;
                    data++;
                }
                memset(pBuffer+len,end_byte,insert+safety); // silence
                len += insert;


                break;
            }
        }
        AppWriteDataToBuffer(voices[channel].ds_buffer,voices[channel].pos,pBuffer,len+safety);
        voices[channel].pos=(voices[channel].pos+len)%(buffer_len);
        //memset(silence8,pBuffer[len-1],SILENCE8_LENGTH);
        //AppWriteDataToBuffer(voices[channel].ds_buffer,voices[channel].pos,silence8,SILENCE8_LENGTH);

        
    }
    else
    {
//        voices[channel].volume=volume;
        voices[channel].length=len;
        voices[channel].freq=freq;
        voices[channel].buffer_len=freq*BUFFER_SECONDS;
        voices[channel].allocated=1;
        voices[channel].bits = 8;
        voices[channel].type = Stream;
        voices[channel].loop = 1;
		voices[channel].frame_len = numChannels*voices[channel].bits*freq/8/video_fps;
        voices[channel].numChannels = numChannels;
#ifdef DS_DEBUG1
        {
		    LARGE_INTEGER   tick_count;

	        QueryPerformanceCounter(&tick_count);
			fprintf(debug_fd[channel],"channel %d started %d %d frame length %d expected channels %d\n",channel,(int)tick_count.HighPart,
					(int)tick_count.LowPart,voices[channel].frame_len,numChannels);
			voices[channel].start_time = tick_count.LowPart;
			voices[channel].last_time = tick_count.LowPart;
            voices[channel].frame=0;

        }
#endif  
        if (len<voices[channel].frame_len-1)
        {
            // we're getting too few bytes each frame, must be a fake stereo signal
            if (voices[channel].frame_len/2 == len)
            {
                voices[channel].numChannels = 1;
                voices[channel].frame_len /= 2;
#ifdef DS_DEBUG1
                fprintf(debug_fd[channel],"fake stereo with frame_len %d\n",voices[channel].frame_len);
#endif
            }
            else
            {
                // wtf now?
#ifdef DS_DEBUG1
                fprintf(debug_fd[channel],"ERROR: frame_len %d, actual length %d\n",voices[channel].frame_len,len);
#endif
            }
        }
        else if (len>voices[channel].frame_len+1)
        {
                // wtf now?
#ifdef DS_DEBUG1
                fprintf(debug_fd[channel],"ERROR: frame_len %d, actual length %d\n",voices[channel].frame_len,len);
#endif
        }

        AppCreateBasicBuffer(lpDirectSound,&(voices[channel].ds_buffer),
            voices[channel].buffer_len,0,freq,8,voices[channel].numChannels);

#ifdef RECORD
        {
            char fname[100];

            sprintf(fname,"audio%d.raw",channel);  
            fd[channel]=fopen(fname,"wb");
            fwrite(data,sizeof(unsigned char),len,fd[channel]);
        }
#endif  


        pBuffer = (unsigned char *) malloc(voices[channel].frame_len* sizeof(unsigned char)*FRAMES_AHEAD+8);
        memset(pBuffer,0x80,FRAMES_AHEAD*voices[channel].frame_len/2);
        for (i = (FRAMES_AHEAD/2)*voices[channel].frame_len; i < (FRAMES_AHEAD/2)*voices[channel].frame_len+len; i++)
        {
            pBuffer[i] = (*data) - 128;
            data++;
        }
        memset(pBuffer+(FRAMES_AHEAD/2+1)*voices[channel].frame_len, pBuffer[i - 1],
            (FRAMES_AHEAD/2-1)*voices[channel].frame_len);
        set_vol(channel,volume);
		set_pan(channel,pan);

        AppWriteDataToBuffer(voices[channel].ds_buffer,0,pBuffer,(FRAMES_AHEAD)*voices[channel].frame_len);

        IDirectSoundBuffer_Play(voices[channel].ds_buffer,0,0,DSBPLAY_LOOPING);
        voices[channel].pos=(FRAMES_AHEAD/2+1)*len;
        memset(silence8,pBuffer[(FRAMES_AHEAD)*voices[channel].frame_len-1],SILENCE8_LENGTH);
        AppWriteDataToBuffer(voices[channel].ds_buffer,voices[channel].pos,silence8,SILENCE8_LENGTH);



		

    
    }

    voices[channel].last_data_byte = pBuffer[len-1];
    free(pBuffer);
}

static void DirectSound_play_streamed_sample_16(int channel, signed short *data, int len, int freq, int volume, int pan)
{
    int i;
    signed short *pBuffer=NULL;
    int end_short = data[len/2-1];

    if (voices[channel].allocated)
    {
        int buffer_len = voices[channel].buffer_len;
        int status;
        int safety = 0;

#ifdef RECORD
        fwrite(data,sizeof(unsigned char),len,fd[channel]);
#endif
#ifdef DS_DEBUG1
        voices[channel].frame++;
#endif
        pBuffer=data;
		if (voices[channel].pan!=pan) set_pan(channel,pan);
        if (volume!=voices[channel].volume)
            set_vol(channel,volume);
#undef RAMP_LENGTH
#define RAMP_LENGTH 30

        status = check_position(channel, 1);
        switch(status)
        {
            case STATUS_DROP:
            {
                int i;
                int last,next,now;

                pBuffer = (signed short *) malloc(RAMP_LENGTH* sizeof(signed short)+8);
                last=voices[channel].last_data_short;
                next=data[len/2-1];
                for(i=0;i<RAMP_LENGTH;i++)
                {
                    now=(next-last)*(i+1)/RAMP_LENGTH+last;
                    pBuffer[i]=now;
                }
                len=RAMP_LENGTH;
                voices[channel].last_insert = 0;
                break;
            }
            case STATUS_NORMAL:
                voices[channel].last_insert = 0;
                break;
            default: // STATUS_EXTEND
            {
                int insert = status - STATUS_EXTEND;
                int i;

                if (insert % 2) insert++;

                if (voices[channel].last_insert+insert > voices[channel].frame_len/2)
                { // we're running the risk of playing past the buffered stuff.
                    safety = voices[channel].frame_len+voices[channel].last_insert+insert;
                }
                safety += 4-((insert+safety)%4);
                voices[channel].last_insert = insert;
                
                pBuffer = (signed short *) malloc((len+insert+safety)*
                    sizeof(char)+8);
                memcpy(pBuffer,data,len);
                {
                    unsigned int *tmp;
                    unsigned int value;
                    tmp = (unsigned int *)&(pBuffer[len/2]);
                    value = (((unsigned int)end_short) << 16) | ((unsigned int)end_short);
                    for(i=0;i<(insert+safety)/4;i++)
                    {
                        *tmp = value;  //silence
                        tmp++;
                    }
                }
                len += insert;
                break;
            }
        }
        
        AppWriteDataToBuffer(voices[channel].ds_buffer,voices[channel].pos,(char *)pBuffer,(len+safety)*sizeof(char));
        voices[channel].pos=(voices[channel].pos+len*sizeof(char))%(buffer_len);
        voices[channel].pos-=voices[channel].pos%2;
        //for(i=0;i<SILENCE16_LENGTH;i++) silence16[i] = pBuffer[len/2-1];
        //AppWriteDataToBuffer(voices[channel].ds_buffer,voices[channel].pos,(unsigned char *)silence16,SILENCE16_LENGTH*2);

    }
    else
    {
//      ErrorMsg("ch %d len %d",channel,len);
//        voices[channel].volume=volume;
        voices[channel].length=len;
        voices[channel].freq=freq;
        voices[channel].buffer_len=freq*BUFFER_SECONDS*2;
        voices[channel].allocated=1;
        voices[channel].type = Stream;
        voices[channel].bits = 16;
        voices[channel].loop = 1;
		voices[channel].frame_len = numChannels*voices[channel].bits*freq/8/video_fps;
        voices[channel].numChannels = numChannels;
#ifdef DS_DEBUG1
        {
		    LARGE_INTEGER   tick_count;

	        QueryPerformanceCounter(&tick_count);
			fprintf(debug_fd[channel],"16 bit channel %d freq %d started %d %d frame length %d expected channels %d\n",
                    channel,freq,(int)tick_count.HighPart,
					(int)tick_count.LowPart,voices[channel].frame_len,numChannels);
			voices[channel].start_time = tick_count.LowPart;
			voices[channel].last_time = tick_count.LowPart;
            voices[channel].frame=0;

        }
#endif  
        if (len<voices[channel].frame_len-1 && numChannels == 2)
        {
            // we're getting too few bytes each frame, must be a fake stereo signal
            if (voices[channel].frame_len/2 == len)
            {
                voices[channel].numChannels = 1;
                voices[channel].frame_len /= 2;
#ifdef DS_DEBUG1
                fprintf(debug_fd[channel],"fake stereo with frame_len %d\n",voices[channel].frame_len);
#endif
            }
            else
            {
                // wtf now?
#ifdef DS_DEBUG1
                fprintf(debug_fd[channel],"ERROR: frame_len %d, actual length %d\n",voices[channel].frame_len,len);
#endif
            }
        }
        else if (len>voices[channel].frame_len+1)
        {
                // wtf now?
#ifdef DS_DEBUG1
                fprintf(debug_fd[channel],"ERROR: frame_len %d, actual length %d\n",voices[channel].frame_len,len);
#endif
        }
        if (AppCreateBasicBuffer(lpDirectSound,&(voices[channel].ds_buffer),
            voices[channel].buffer_len,0,freq,16,voices[channel].numChannels)==FALSE)
        {
            ErrorMsg("Could not get 16 bit BasicBuffer channel %d",channel);
        }
        set_vol(channel,volume);
		set_pan(channel,pan);

#ifdef RECORD
        {
            char fname[100];

            sprintf(fname,"audio%d.raw",channel);  
            fd[channel]=fopen(fname,"wb");
            fwrite(data,sizeof(unsigned char),len,fd[channel]);
        }
#endif  


        pBuffer = (signed short *) malloc(voices[channel].frame_len* sizeof(char)*FRAMES_AHEAD+8);
        memset(pBuffer,0,FRAMES_AHEAD*voices[channel].frame_len*sizeof(char));
        memcpy(pBuffer+(FRAMES_AHEAD/2)*voices[channel].frame_len/2,data,sizeof(char)*len);
        AppWriteDataToBuffer(voices[channel].ds_buffer,voices[channel].pos,(char *)pBuffer,len*sizeof(char));
        IDirectSoundBuffer_Play(voices[channel].ds_buffer,0,0,DSBPLAY_LOOPING);
        voices[channel].pos=(FRAMES_AHEAD/2+1)*voices[channel].frame_len*sizeof(char);
        for(i=0;i<SILENCE16_LENGTH;i++) silence16[i] = pBuffer[len/2-1];
        AppWriteDataToBuffer(voices[channel].ds_buffer,voices[channel].pos,(unsigned char *)silence16,SILENCE16_LENGTH*2);
    }

    voices[channel].last_data_short = pBuffer[len/2-1];
    if (pBuffer!=data && pBuffer!=NULL) free(pBuffer);
}

static void set_freq(int channel,int freq)
{
    if (freq!=-1)
	{
		IDirectSoundBuffer_SetFrequency(voices[channel].ds_buffer,freq);
		voices[channel].freq=freq;
	}
}

static void set_vol(int channel,int volume)
{
    HRESULT dsval;
    long vol;
    double fvol;
	int temp_vol;
    
    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 100 / 255;

    if (volume != -1)
	{
//crappy old dx?!
#ifndef DSBVOLUME_MIN
#define DSBPAN_LEFT                 -10000
#define DSBPAN_CENTER               0
#define DSBPAN_RIGHT                10000

#define DSBVOLUME_MIN               -10000
#define DSBVOLUME_MAX               0
#endif
		fvol = 1-pow(((double)100-((double)volume))
			/(double)100,(double)20);
		vol = (long)((DSBVOLUME_MIN)+fvol*((DSBVOLUME_MAX)-(DSBVOLUME_MIN))+attenuation*100);
		if (vol < (DSBVOLUME_MIN))
			vol = DSBVOLUME_MIN;
        if (vol > (DSBVOLUME_MAX))
            vol = DSBVOLUME_MAX;

		dsval=IDirectSoundBuffer_SetVolume(voices[channel].ds_buffer,vol);

		if (dsval!=DS_OK)
		{
			ErrorMsg("Could not set volume channel %d",channel);
		}
		temp_vol=vol;
		voices[channel].volume=volume;
	}
}


static void set_pan(int channel, int pan)
{
    HRESULT dsval;

    if (pan == MIXER_PAN_CENTER)
        dsval = IDirectSoundBuffer_SetPan(voices[channel].ds_buffer, 0);
    else
    if (pan == MIXER_PAN_LEFT)
        dsval = IDirectSoundBuffer_SetPan(voices[channel].ds_buffer, DSBPAN_LEFT);
    else
    if (pan == MIXER_PAN_RIGHT)
        dsval = IDirectSoundBuffer_SetPan(voices[channel].ds_buffer, DSBPAN_RIGHT);

	if (dsval != DS_OK)
	{
		ErrorMsg("Could not set pan on  channel %d",channel);
	}

}

void DirectSound_set_sample_freq(int channel,int freq)
{
    if (voices[channel].allocated)
    {
        if (freq != -1)
            set_freq(channel,freq);
    }
}

void DirectSound_set_sample_volume(int channel,int volume)
{
    if (voices[channel].allocated)
    {
        if (volume != -1)
            set_vol(channel,volume);
    }
}

static void DirectSound_stop_sample(int channel)
{
    if (!(voices[channel].allocated)) return;
    IDirectSoundBuffer_Stop(voices[channel].ds_buffer);

#ifdef DS_DEBUG1
	{
		LARGE_INTEGER   tick_count;

	    QueryPerformanceCounter(&tick_count);

		fprintf(debug_fd[channel],"stopped %d\n",tick_count.LowPart);	
	}
#endif

}

static void DirectSound_restart_sample(int channel)
{
//    ErrorMsg("restart ch %d",channel);

}

static int DirectSound_get_sample_status(int channel)
{
    int stopped=1;
    DWORD status;

    if (voices[channel].allocated)
    {
        stopped = 0;
        switch(voices[channel].type)
        {
        case Sample:
            IDirectSoundBuffer_GetStatus(voices[channel].ds_buffer,&status);
            stopped=!(status & DSBSTATUS_PLAYING);
            break;
        default:
            stopped = 0;
            break;
        }
    }
    return stopped;
}

static void DirectSound_set_mastervolume(int volume)
{
    int i;

//ErrorMsg("Master");    
	attenuation=volume;
    for(i=0;i<MIXER_MAX_CHANNELS;i++)
    {
        if (voices[i].allocated) set_vol(i,voices[i].volume);
    }
}

static int DirectSound_get_mastervolume()
{
	return attenuation;
}

static void DirectSound_sound_enable(int enable)
{
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
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static BOOL AppCreateBasicBuffer( 
    LPDIRECTSOUND lpDirectSound, 
    LPDIRECTSOUNDBUFFER *lplpDsb, int len, int flags,int freq, int bits, int channels) 
{ 
    PCMWAVEFORMAT pcmwf; 
    DSBUFFERDESC dsbdesc; 
    HRESULT hr; 
    // Set up wave format structure. 
    memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT)); 
    pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM; 

    pcmwf.wf.nChannels = channels;
    pcmwf.wf.nSamplesPerSec = freq; 

    pcmwf.wBitsPerSample = bits; 
    pcmwf.wf.nBlockAlign = pcmwf.wBitsPerSample*pcmwf.wf.nChannels/8;
    pcmwf.wf.nAvgBytesPerSec = 
        pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign; 
    // Set up DSBUFFERDESC structure. 
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out. 
    dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
    // Need default controls (pan, volume, frequency). 
    dsbdesc.dwFlags = DSBCAPS_CTRLDEFAULT | flags/* | DSBCAPS_STATIC */; 
    // buffer. 
    dsbdesc.dwBufferBytes = len;
    /* BUFFER_SECONDS * pcmwf.wf.nAvgBytesPerSec; */
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf; 
    // Create buffer. 
    hr = lpDirectSound->lpVtbl->CreateSoundBuffer(lpDirectSound, 
        &dsbdesc, lplpDsb, NULL); 
    if(DS_OK == hr) { 
        return TRUE; 
    } else { 
        if (hr == DSERR_ALLOCATED )
        {
            ErrorMsg("ERR_ALLOCATED");
        }
        else if ( hr == DSERR_BADFORMAT)
        {
            ErrorMsg("DSERR_BADFORMAT");
        }
        else if ( hr == DSERR_INVALIDPARAM)
        {
            ErrorMsg("DSERR_INVALIDPARAM");
        }
        else if ( hr == DSERR_NOAGGREGATION)
        {
            ErrorMsg("DSERR_NOAGGREGATION");
        }
        else if ( hr == DSERR_OUTOFMEMORY)
        {
            ErrorMsg("DSERR_OUTOFMEMORY");
        }
        else if ( hr == DSERR_UNINITIALIZED)
        {
            ErrorMsg("DSERR_UNINITIALIZED");
        }
        else if ( hr == DSERR_UNSUPPORTED)
        {
            ErrorMsg("DSERR_UNSUPPORTED");
        }

        // Failed. 
        *lplpDsb = NULL; 
        return FALSE; 
    } 
} 
/* 
When you create a sound buffer, you can indicate that a buffer 
is static by specifying the DSBCAPS_STATIC flag. If you do not specify 
this flag, the buffer is a streaming buffer. For more information, 
see Static and Streaming Sound Buffers.
*/


static BOOL AppWriteDataToBuffer( 
    LPDIRECTSOUNDBUFFER lpDsb,  // the DirectSound buffer
    DWORD dwOffset,             // our own write cursor
    LPBYTE lpbSoundData,        // start of our data
    DWORD dwSoundBytes)         // size of block to copy
{ 
    LPVOID lpvPtr1; 
    DWORD dwBytes1; 
    LPVOID lpvPtr2; 
    DWORD dwBytes2; 
    HRESULT hr; 
    // Obtain memory address of write block. This will be in two parts
    // if the block wraps around.
    hr = lpDsb->lpVtbl->Lock(lpDsb, dwOffset, dwSoundBytes, &lpvPtr1, 
        &dwBytes1, &lpvPtr2, &dwBytes2, 0); 
#ifdef DEBUG
    if (hr == DSERR_INVALIDCALL)
    {
        ErrorMsg("DSERR_INVALIDCALL");
    }
    if (hr == DSERR_PRIOLEVELNEEDED)
    {
        ErrorMsg("DSERR_PRIOLEVELNEEDED");
    }
    if (hr == DSERR_INVALIDPARAM)
    {
        ErrorMsg("DSERR_INVALIDPARAM");
    }
#endif
    // If DSERR_BUFFERLOST is returned, restore and retry lock. 
    if(DSERR_BUFFERLOST == hr) { 
        lpDsb->lpVtbl->Restore(lpDsb); 
        hr = lpDsb->lpVtbl->Lock(lpDsb, dwOffset, dwSoundBytes, 
            &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0 /*DSBLOCK_FROMWRITECURSOR*/); 
    } 
    if(DS_OK == hr) { 
        // Write to pointers. 
        CopyMemory(lpvPtr1, lpbSoundData, dwBytes1); 
        if(NULL != lpvPtr2) { 
            CopyMemory(lpvPtr2, lpbSoundData+dwBytes1, dwBytes2); 

        } 
        // Release the data back to DirectSound. 
        hr = lpDsb->lpVtbl->Unlock(lpDsb, lpvPtr1, dwBytes1, lpvPtr2, 
            dwBytes2); 
        if(DS_OK == hr) { 
            // Success. 
            return TRUE; 
        } 
    } 
    else
    {
        ErrorMsg("Could not lock DirectSound buffer");
    }
    // Lock, Unlock, or Restore failed. 
    return FALSE; 
} 

static BOOL AppWriteDataToBufferNow( 
    LPDIRECTSOUNDBUFFER lpDsb,  // the DirectSound buffer
    LPBYTE lpbSoundData,        // start of our data
    DWORD dwSoundBytes)         // size of block to copy
{ 
    LPVOID lpvPtr1; 
    DWORD dwBytes1; 
    LPVOID lpvPtr2; 
    DWORD dwBytes2; 
    HRESULT hr; 
    // Obtain memory address of write block. This will be in two parts
    // if the block wraps around.
    hr = lpDsb->lpVtbl->Lock(lpDsb, 0, dwSoundBytes, &lpvPtr1, 
        &dwBytes1, &lpvPtr2, &dwBytes2, DSBLOCK_FROMWRITECURSOR); 
#ifdef DEBUG
    if (hr == DSERR_INVALIDCALL)
    {
        ErrorMsg("DSERR_INVALIDCALL");
    }
    if (hr == DSERR_PRIOLEVELNEEDED)
    {
        ErrorMsg("DSERR_PRIOLEVELNEEDED");
    }
    if (hr == DSERR_INVALIDPARAM)
    {
        ErrorMsg("DSERR_INVALIDPARAM");
    }
#endif
    // If DSERR_BUFFERLOST is returned, restore and retry lock. 
    if(DSERR_BUFFERLOST == hr) { 
        lpDsb->lpVtbl->Restore(lpDsb); 
        hr = lpDsb->lpVtbl->Lock(lpDsb, 0, dwSoundBytes, 
            &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, DSBLOCK_FROMWRITECURSOR); 
    } 
    if(DS_OK == hr) { 
        // Write to pointers. 
        CopyMemory(lpvPtr1, lpbSoundData, dwBytes1); 
        if(NULL != lpvPtr2) { 
            CopyMemory(lpvPtr2, lpbSoundData+dwBytes1, dwBytes2); 

        } 
        // Release the data back to DirectSound. 
        hr = lpDsb->lpVtbl->Unlock(lpDsb, lpvPtr1, dwBytes1, lpvPtr2, 
            dwBytes2); 
        if(DS_OK == hr) { 
            // Success. 
            return TRUE; 
        } 
    } 
    else
    {
        ErrorMsg("Could not lock DirectSound buffer");
    }
    // Lock, Unlock, or Restore failed. 
    return FALSE; 
} 
 
/*
To retrieve and set the volume at which a buffer is played, your 
application can use the IDirectSoundBuffer::GetVolume and 
IDirectSoundBuffer::SetVolume methods. Setting the volume on the 
primary sound buffer changes the waveform-audio volume of the sound card.

Similarly, by calling the IDirectSoundBuffer::GetFrequency and 
IDirectSoundBuffer::SetFrequency methods, you can retrieve and 
set the frequency at which audio samples play. You cannot change 
the frequency of the primary buffer.
*/

BOOL AppCreatePrimaryBuffer( 
    LPDIRECTSOUND lpDirectSound, 
    LPDIRECTSOUNDBUFFER *lplpDsb, 
    LPDWORD lpdwBufferSize, 
    HWND hwnd,int freq, int bits) 
{ 
    DSBUFFERDESC dsbdesc; 
    DSBCAPS dsbcaps; 
    HRESULT hr; 

    WAVEFORMATEX wfex;
    // Set up wave format structure. 
    memset(&wfex, 0, sizeof(WAVEFORMATEX)); 
    wfex.wFormatTag = WAVE_FORMAT_PCM; 
#ifdef HIGH_QUALITY
    wfex.nChannels = 2; /* stereo */
    wfex.nSamplesPerSec = 44100; 

    wfex.wBitsPerSample = 16; 
#else
    wfex.nChannels = 1; /* mono */
    wfex.nSamplesPerSec = freq; 

    wfex.wBitsPerSample = bits; 
#endif
    wfex.nBlockAlign = wfex.wBitsPerSample * wfex.nChannels /
        8; 
  
    wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
    wfex.cbSize = 0;

    // Set up DSBUFFERDESC structure. 
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out. 
    dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER; 
    // Buffer size is determined by sound hardware. 
    dsbdesc.dwBufferBytes = 0; 
    dsbdesc.lpwfxFormat = NULL; // Must be NULL for primary buffers. 
 
    // Obtain write-primary cooperative level. 

    hr = lpDirectSound->lpVtbl->SetCooperativeLevel(lpDirectSound, 
        hwnd, DSSCL_PRIORITY ); 
    if (DS_OK == hr) { 
        // Succeeded. Try to create buffer. 
        hr = lpDirectSound->lpVtbl->CreateSoundBuffer(lpDirectSound, 
            &dsbdesc, lplpDsb, NULL); 
        if (DS_OK == hr) { 
            hr = lpDirectSound->lpVtbl->SetCooperativeLevel(lpDirectSound, 
                hwnd, DSSCL_PRIORITY ); 
            if (DS_OK == hr) {
            // Succeeded. Set primary buffer to desired format.
                hr = (*lplpDsb)->lpVtbl->SetFormat(*lplpDsb, &wfex);
                if (DS_OK == hr) { 

                    // If you want to know the buffer size, call GetCaps. 
                        dsbcaps.dwSize = sizeof(DSBCAPS); 
                    (*lplpDsb)->lpVtbl->GetCaps(*lplpDsb, &dsbcaps); 
                    *lpdwBufferSize = dsbcaps.dwBufferBytes; 
                    return TRUE; 
                } 
                else
                {
                    switch(hr)
                    {
                    case DSERR_BADFORMAT :
                        ErrorMsg("format error DSERR_BADFORMAT");
                        break;
                    case DSERR_INVALIDCALL :
                        ErrorMsg("format error DSERR_INVALIDCALL");
                        break;

                    case DSERR_INVALIDPARAM :
                        ErrorMsg("format error DSERR_INVALIDPARAM");
                        break;

                    case DSERR_OUTOFMEMORY :
                        ErrorMsg("format error DSERR_OUTOFMEMORY");
                        break;

                    case DSERR_PRIOLEVELNEEDED :
                        ErrorMsg("format error DSERR_PRIOLEVELNEEDED");
                        break;

                    case DSERR_UNSUPPORTED :
                        ErrorMsg("format error DSERR_UNSUPPORTED");
                        break;
                    }
                }
            }
        } 
    } 
    // SetCooperativeLevel failed. 
    // CreateSoundBuffer, or SetFormat. 
    *lplpDsb = NULL; 
    *lpdwBufferSize = 0; 
    return FALSE; 
} 

/*
static int FindFreeChannel()
{
    int i;
    int status;

    for (i=EXTRAVOICES;i<MIXER_MAX_CHANNELS;i++)
    {
        if (!voices[i].allocated) return i;
        else
        {
            if (!voices[i].loop)
            {
                IDirectSoundBuffer_GetStatus(voices[i].ds_buffer,&status);
                if (!(status & DSBSTATUS_PLAYING))
                    return i;
            }
        }
    }
    return 0;
}   
*/
