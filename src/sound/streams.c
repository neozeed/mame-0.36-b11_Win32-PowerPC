/***************************************************************************

  streams.c

  Handle general purpose audio streams

***************************************************************************/

#include "driver.h"
#include <math.h>


static int stream_joined_channels[MIXER_MAX_CHANNELS];
static void *stream_buffer[MIXER_MAX_CHANNELS];
static int stream_buffer_len[MIXER_MAX_CHANNELS];
static int stream_sample_rate[MIXER_MAX_CHANNELS];
static int stream_sample_bits[MIXER_MAX_CHANNELS];
static int stream_buffer_pos[MIXER_MAX_CHANNELS];
static int stream_sample_length[MIXER_MAX_CHANNELS];	/* in usec */
static int stream_param[MIXER_MAX_CHANNELS];
static void (*stream_callback[MIXER_MAX_CHANNELS])(int param,void *buffer,int length);
static void (*stream_callback_multi[MIXER_MAX_CHANNELS])(int param,void **buffer,int length);

static int memory[MIXER_MAX_CHANNELS];
static int r1[MIXER_MAX_CHANNELS];
static int r2[MIXER_MAX_CHANNELS];
static int r3[MIXER_MAX_CHANNELS];
static int c[MIXER_MAX_CHANNELS];

/*
signal >--R1--+--R2--+
              |      |
              C      R3---> amp
              |      |
             GND    GND
*/

/* R1, R2, R3 in Ohm; C in pF */
/* set C = 0 to disable the filter */
void set_RC_filter(int channel,int R1,int R2,int R3,int C)
{
	r1[channel] = R1;
	r2[channel] = R2;
	r3[channel] = R3;
	c[channel] = C;
}

void apply_RC_filter_8(int channel,signed char *buf,int len,int sample_rate)
{
	float R1,R2,R3,C;
	float Req;
	int K;
	int i;


	if (c[channel] == 0) return;	/* filter disabled */

	R1 = r1[channel];
	R2 = r2[channel];
	R3 = r3[channel];
	C = (float)c[channel] * 1E-12;	/* convert pF to F */

	/* Cut Frequency = 1/(2*Pi*Req*C) */

	Req = (R1*(R2+R3))/(R1+R2+R3);

	K = 0x10000 * exp(-1 / (Req * C) / sample_rate);

	buf[0] = buf[0] + (memory[channel] - buf[0]) * K / 0x10000;

	for (i = 1;i < len;i++)
		buf[i] = buf[i] + (buf[i-1] - buf[i]) * K / 0x10000;

	memory[channel] = buf[len-1];
}

void apply_RC_filter_16(int channel,signed short *buf,int len,int sample_rate)
{
	float R1,R2,R3,C;
	float Req;
	int K;
	int i;


	if (c[channel] == 0) return;	/* filter disabled */

	R1 = r1[channel];
	R2 = r2[channel];
	R3 = r3[channel];
	C = (float)c[channel] * 1E-12;	/* convert pF to F */

	/* Cut Frequency = 1/(2*Pi*Req*C) */

	Req = (R1*(R2+R3))/(R1+R2+R3);

	K = 0x10000 * exp(-1 / (Req * C) / sample_rate);

	buf[0] = buf[0] + (memory[channel] - buf[0]) * K / 0x10000;

	for (i = 1;i < len;i++)
		buf[i] = buf[i] + (buf[i-1] - buf[i]) * K / 0x10000;

	memory[channel] = buf[len-1];
}



int streams_sh_start(void)
{
	int i;


	for (i = 0;i < MIXER_MAX_CHANNELS;i++)
	{
		stream_joined_channels[i] = 1;
		stream_buffer[i] = 0;
	}

	return 0;
}


void streams_sh_stop(void)
{
	int i;


	for (i = 0;i < MIXER_MAX_CHANNELS;i++)
	{
		free(stream_buffer[i]);
		stream_buffer[i] = 0;
	}
}


void streams_sh_update(void)
{
	int channel,i;


	if (Machine->sample_rate == 0) return;

	/* update all the output buffers */
	for (channel = 0;channel < MIXER_MAX_CHANNELS;channel += stream_joined_channels[channel])
	{
		if (stream_buffer[channel])
		{
			int newpos;
			int buflen;


			newpos = stream_buffer_len[channel];

			buflen = newpos - stream_buffer_pos[channel];

			if (stream_joined_channels[channel] > 1)
			{
				void *buf[MIXER_MAX_CHANNELS];


				if (buflen > 0)
				{
					if (stream_sample_bits[channel] == 16)
					{
						for (i = 0;i < stream_joined_channels[channel];i++)
							buf[i] = &((short *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
					}
					else
					{
						for (i = 0;i < stream_joined_channels[channel];i++)
							buf[i] = &((char *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
					}

					(*stream_callback_multi[channel])(stream_param[channel],buf,buflen);
				}

				for (i = 0;i < stream_joined_channels[channel];i++)
					stream_buffer_pos[channel+i] = 0;

				if (stream_sample_bits[channel] == 16)
				{
					for (i = 0;i < stream_joined_channels[channel];i++)
						apply_RC_filter_16(channel+i,stream_buffer[channel+i],stream_buffer_len[channel+i],stream_sample_rate[channel+i]);
				}
				else
				{
					for (i = 0;i < stream_joined_channels[channel];i++)
						apply_RC_filter_8(channel+i,stream_buffer[channel+i],stream_buffer_len[channel+i],stream_sample_rate[channel+i]);
				}
			}
			else
			{
				if (buflen > 0)
				{
					void *buf;


					if (stream_sample_bits[channel] == 16)
						buf = &((short *)stream_buffer[channel])[stream_buffer_pos[channel]];
					else
						buf = &((char *)stream_buffer[channel])[stream_buffer_pos[channel]];

					(*stream_callback[channel])(stream_param[channel],buf,buflen);
				}

				stream_buffer_pos[channel] = 0;

				if (stream_sample_bits[channel] == 16)
					apply_RC_filter_16(channel,stream_buffer[channel],stream_buffer_len[channel],stream_sample_rate[channel]);
				else
					apply_RC_filter_8(channel,stream_buffer[channel],stream_buffer_len[channel],stream_sample_rate[channel]);
			}
		}
	}

	for (channel = 0;channel < MIXER_MAX_CHANNELS;channel += stream_joined_channels[channel])
	{
		if (stream_buffer[channel])
		{
			if (stream_sample_bits[channel] == 16)
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					mixer_play_streamed_sample_16(channel+i,
							stream_buffer[channel+i],2*stream_buffer_len[channel+i],
							stream_sample_rate[channel+i]);
			}
			else
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					mixer_play_streamed_sample(channel+i,
							stream_buffer[channel+i],stream_buffer_len[channel+i],
							stream_sample_rate[channel+i]);
			}
		}
	}
}


int stream_init(const char *name,int default_mixing_level,
		int sample_rate,int sample_bits,
		int param,void (*callback)(int param,void *buffer,int length))
{
	int channel;


	channel = mixer_allocate_channel(default_mixing_level);

	stream_joined_channels[channel] = 1;

	mixer_set_name(channel,name);

	stream_buffer_len[channel] = sample_rate / Machine->drv->frames_per_second;
	/* adjust sample rate to make it a multiple of buffer_len */
	sample_rate = stream_buffer_len[channel] * Machine->drv->frames_per_second;

	if ((stream_buffer[channel] = malloc((sample_bits/8)*stream_buffer_len[channel])) == 0)
		return -1;

	stream_sample_rate[channel] = sample_rate;
	stream_sample_bits[channel] = sample_bits;
	stream_buffer_pos[channel] = 0;
	if (sample_rate)
		stream_sample_length[channel] = 1000000 / sample_rate;
	else
		stream_sample_length[channel] = 0;
	stream_param[channel] = param;
	stream_callback[channel] = callback;
	set_RC_filter(channel,0,0,0,0);

	return channel;
}


int stream_init_multi(int channels,const char **names,const int *default_mixing_levels,
		int sample_rate,int sample_bits,
		int param,void (*callback)(int param,void **buffer,int length))
{
	int channel,i;


	channel = mixer_allocate_channels(channels,default_mixing_levels);

	stream_joined_channels[channel] = channels;

	for (i = 0;i < channels;i++)
	{
		mixer_set_name(channel+i,names[i]);

		stream_buffer_len[channel+i] = sample_rate / Machine->drv->frames_per_second;
		/* adjust sample rate to make it a multiple of buffer_len */
		sample_rate = stream_buffer_len[channel+i] * Machine->drv->frames_per_second;

		if ((stream_buffer[channel+i] = malloc((sample_bits/8)*stream_buffer_len[channel+i])) == 0)
			return -1;

		stream_sample_rate[channel+i] = sample_rate;
		stream_sample_bits[channel+i] = sample_bits;
		stream_buffer_pos[channel+i] = 0;
		if (sample_rate)
			stream_sample_length[channel+i] = 1000000 / sample_rate;
		else
			stream_sample_length[channel+i] = 0;
	}

	stream_param[channel] = param;
	stream_callback_multi[channel] = callback;
	set_RC_filter(channel,0,0,0,0);

	return channel;
}


/* min_interval is in usec */
void stream_update(int channel,int min_interval)
{
	int newpos;
	int buflen;


	if (stream_buffer[channel] == 0)
		return;

	/* get current position based on the timer */
	newpos = sound_scalebufferpos(stream_buffer_len[channel]);

	buflen = newpos - stream_buffer_pos[channel];

	if (buflen * stream_sample_length[channel] > min_interval)
	{
		if (stream_joined_channels[channel] > 1)
		{
			void *buf[MIXER_MAX_CHANNELS];
			int i;


			if (stream_sample_bits[channel] == 16)
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					buf[i] = &((short *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
			}
			else
			{
				for (i = 0;i < stream_joined_channels[channel];i++)
					buf[i] = &((char *)stream_buffer[channel+i])[stream_buffer_pos[channel+i]];
			}

			profiler_mark(PROFILER_SOUND);
			(*stream_callback_multi[channel])(stream_param[channel],buf,buflen);
			profiler_mark(PROFILER_END);

			for (i = 0;i < stream_joined_channels[channel];i++)
				stream_buffer_pos[channel+i] += buflen;
		}
		else
		{
			void *buf;


			if (stream_sample_bits[channel] == 16)
				buf = &((short *)stream_buffer[channel])[stream_buffer_pos[channel]];
			else
				buf = &((char *)stream_buffer[channel])[stream_buffer_pos[channel]];

			profiler_mark(PROFILER_SOUND);
			(*stream_callback[channel])(stream_param[channel],buf,buflen);
			profiler_mark(PROFILER_END);

			stream_buffer_pos[channel] += buflen;
		}
	}
}

int stream_get_sample_bits(int channel)
{
	if (stream_buffer[channel])
		return stream_sample_bits[channel];
	else return 0;
}

int stream_get_sample_rate(int channel)
{
	if (stream_buffer[channel])
		return stream_sample_rate[channel];
	else return 0;
}

void *stream_get_buffer(int channel)
{
	if (stream_buffer[channel])
		return stream_buffer[channel];
	else return 0;
}

int stream_get_buffer_len(int channel)
{
	if (stream_buffer[channel])
		return stream_buffer_len[channel];
	else return 0;
}
