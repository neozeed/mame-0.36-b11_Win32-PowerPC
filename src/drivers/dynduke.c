/***************************************************************************

	Dynamite Duke						(c) 1989 Seibu Kaihatsu/Fabtek
	The Double Dynamites				(c) 1989 Seibu Kaihatsu/Fabtek


	To access test mode, reset with both start buttons held.

	Coins are controlled by the sound cpu, and the sound cpu is encrypted!
	You need to play with sound off at the moment to coin up.

	The background layer is 5bpp and I'm not 100% sure the colours are
	correct on it, although the layer is 5bpp the palette data is 4bpp.
	My current implementation looks pretty good though I've never seen
	the real game.

	Emulation by Bryan McPhail, mish@tendril.force9.net

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "cpu/z80/z80.h"

int dynduke_background_r(int offset);
int dynduke_foreground_r(int offset);
void dynduke_background_w(int offset,int data);
void dynduke_foreground_w(int offset,int data);
void dynduke_text_w(int offset,int data);
void dynduke_gfxbank_w(int offset,int data);
int dynduke_vh_start(void);
void dynduke_control_w(int offset, int data);
void dynduke_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void dynduke_paletteram_w(int offset, int data);

static unsigned char *dynduke_shared_ram,*sound_shared_ram;
extern unsigned char *dynduke_back_data,*dynduke_fore_data,*dynduke_scroll_ram,*dynduke_control_ram;

/***************************************************************************/

static int dynduke_shared_r(int offset) { return dynduke_shared_ram[offset]; }
static void dynduke_shared_w(int offset,int data) { dynduke_shared_ram[offset]=data; }

static int dynduke_soundcpu_r(int offset)
{
	int erg,orig;
	orig=sound_shared_ram[offset];

	/* Small kludge to allows coins with sound off */
	if (offset==4 && (!Machine->sample_rate) && (readinputport(4)&1)==1) return 1;

	switch (offset)
	{
		case 0x04:{erg=sound_shared_ram[6];sound_shared_ram[6]=0;break;} /* just 1 time */
		case 0x06:{erg=0xa0;break;}
		case 0x0a:{erg=0;break;}
		default: erg=sound_shared_ram[offset];
	}
	return erg;
}

static void dynduke_soundcpu_w(int offset, int data)
{
	sound_shared_ram[offset]=data;

	if (offset==0xc && data!=0xff) {
		cpu_cause_interrupt(2,0xdf); /* RST 18 */
	}
}

static int maincpu_data_r(int offset)
{
	return sound_shared_ram[offset<<1];
}

static void maincpu_data_w(int offset, int data)
{
	sound_shared_ram[offset<<1]=data;
}

static void sound_bank_w(int offset,int data)
{
	unsigned char *RAM = Machine->memory_region[2];

	if (data&1) { cpu_setbank(1,&RAM[0x0000]); }
	else { cpu_setbank(1,&RAM[0x10000]); }
}

static void sound_clear_w(int offset,int data)
{
	sound_shared_ram[0]=data;
}

/******************************************************************************/

static struct MemoryReadAddress readmem[] =
{
	{ 0x00000, 0x07fff, MRA_RAM },
	{ 0x0a000, 0x0afff, dynduke_shared_r },
	{ 0x0b000, 0x0b000, input_port_0_r },
	{ 0x0b001, 0x0b001, input_port_1_r },
	{ 0x0b002, 0x0b002, input_port_2_r },
	{ 0x0b003, 0x0b003, input_port_3_r },
	{ 0x0d000, 0x0d00f, dynduke_soundcpu_r },
	{ 0xa0000, 0xfffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x00000, 0x06fff, MWA_RAM },
	{ 0x07000, 0x07fff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x08000, 0x080ff, MWA_RAM, &dynduke_scroll_ram },
	{ 0x0a000, 0x0afff, dynduke_shared_w, &dynduke_shared_ram },
	{ 0x0b000, 0x0b007, dynduke_control_w, &dynduke_control_ram },
	{ 0x0c000, 0x0c7ff, dynduke_text_w, &videoram },
	{ 0x0d000, 0x0d00f, dynduke_soundcpu_w, &sound_shared_ram },
	{ 0xa0000, 0xfffff, MWA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress sub_readmem[] =
{
	{ 0x00000, 0x05fff, MRA_RAM },
	{ 0x06000, 0x067ff, dynduke_background_r },
	{ 0x06800, 0x06fff, dynduke_foreground_r },
	{ 0x07000, 0x07fff, paletteram_r },
	{ 0x08000, 0x08fff, dynduke_shared_r },
	{ 0xc0000, 0xfffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sub_writemem[] =
{
	{ 0x00000, 0x05fff, MWA_RAM },
	{ 0x06000, 0x067ff, dynduke_background_w, &dynduke_back_data },
	{ 0x06800, 0x06fff, dynduke_foreground_w, &dynduke_fore_data },
	{ 0x07000, 0x07fff, dynduke_paletteram_w, &paletteram },
	{ 0x08000, 0x08fff, dynduke_shared_w },
	{ 0x0a000, 0x0a001, dynduke_gfxbank_w },
	{ 0x0c000, 0x0c001, MWA_NOP },
	{ 0xc0000, 0xfffff, MWA_ROM },
	{ -1 }	/* end of table */
};

/******************************************************************************/

#if 0
static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },
	{ 0x2000, 0x27ff, MRA_RAM },
	{ 0x4008, 0x4008, YM3812_status_port_0_r },
	{ 0x4010, 0x4011, maincpu_data_r }, /* Soundlatch (16 bits) */
	{ 0x4013, 0x4013, input_port_4_r }, /* Coins */
	{ 0x6000, 0x6000, OKIM6295_status_0_r },
	{ 0x8000, 0xffff, MRA_BANK1 },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },
	{ 0x2000, 0x27ff, MWA_RAM },
	{ 0x4000, 0x4000, sound_clear_w }, /* Or command ack?? */
	{ 0x4002, 0x4002, MWA_NOP }, /* RST 10h interrupt ack */
	{ 0x4003, 0x4003, MWA_NOP }, /* RST 18h interrupt ack */
	{ 0x4007, 0x4007, sound_bank_w },
	{ 0x4008, 0x4008, YM3812_control_port_0_w },
	{ 0x4009, 0x4009, YM3812_write_port_0_w },
	{ 0x4018, 0x401f, maincpu_data_w },
	{ 0x6000, 0x6000, OKIM6295_data_0_w },
	{ -1 }	/* end of table */
};
#endif

/******************************************************************************/

INPUT_PORTS_START( dynduke )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START	/* Dip switch A */
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x06, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "3 Coins/1 Credits" )
	PORT_DIPSETTING(    0x00, "5 Coins/1 Credits" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x18, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/5 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/6 Credits" )
	PORT_DIPSETTING(    0x00, "Free Play" )
	PORT_DIPNAME( 0x20, 0x20, "Starting Coin" )
	PORT_DIPSETTING(    0x20, "normal" )
	PORT_DIPSETTING(    0x00, "X 2" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* Dip switch B */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty?" )
	PORT_DIPSETTING(    0x30, "Normal" )
	PORT_DIPSETTING(    0x20, "Easy" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x00, "Very Hard" )
	PORT_DIPNAME( 0x40, 0x40, "Continue?" )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Demo Sound?" )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x80, "On" )

	PORT_START	/* Coins */
	PORT_BIT_IMPULSE( 0x01, IP_ACTIVE_HIGH, IPT_COIN1, 1 )
	PORT_BIT_IMPULSE( 0x02, IP_ACTIVE_HIGH, IPT_COIN2, 1 )
INPUT_PORTS_END

/******************************************************************************/

static struct GfxLayout charlayout =
{
	8,8,		/* 8*8 characters */
	1024,
	4,			/* 4 bits per pixel */
	{ 4,0,(0x10000*8)+4,0x10000*8 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static struct GfxLayout spritelayout =
{
  16,16,	/* 16*16 tiles */
  0x4000,
  4,		/* 4 bits per pixel */
  { 12, 8, 4, 0 },
  {
    0,1,2,3, 16,17,18,19,
	512+0,512+1,512+2,512+3,
	512+8+8,512+9+8,512+10+8,512+11+8,
  },
  {
	0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
  },
  1024
};

static struct GfxLayout bg_layout =
{
	16,16,
	0x2000,
	5,
	{ 0x100000*8+4, 0x80000*8+4,0x80000*8,4,0 },
	{
		0,1,2,3,8,9,10,11,
		256+0,256+1,256+2,256+3,256+8,256+9,256+10,256+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16
	},
	512
};

static struct GfxLayout fg_layout =
{
	16,16,
	0x2000,
	4,
	{ 0x80000*8+4, 0x80000*8, 4, 0 },
	{
		0,1,2,3,8,9,10,11,
		256+0,256+1,256+2,256+3,256+8,256+9,256+10,256+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16
	},
	512
};

static struct GfxDecodeInfo dynduke_gfxdecodeinfo[] =
{
	{ 1, 0x000000, &charlayout,   1280, 16 },
	{ 1, 0x020000, &bg_layout,    2048, 32 }, /* Really 0 */
	{ 1, 0x1a0000, &fg_layout,     512, 16 },
	{ 1, 0x2a0000, &spritelayout,  768, 32 },
	{ -1 } /* end of array */
};

/******************************************************************************/

/* handler called by the 3812 emulator when the internal timers cause an IRQ */
static void YM3812_irqhandler(int linestate)
{
	cpu_irq_line_vector_w(2,0,0xd7); /* RST 10h */
	cpu_set_irq_line(2,0,linestate);
}

static struct YM3812interface ym3812_interface =
{
	1,
	4000000,
	{ 50 },
	{ YM3812_irqhandler }
};

static struct OKIM6295interface okim6295_interface =
{
	1,
	{ 8000 },
	{ 4 },
	{ 41 }
};

static int dynduke_interrupt(void)
{
	return 0xc8/4;	/* VBL */
}

static void dynduke_eof_callback(void)
{
	buffer_spriteram_w(0,0); /* Could be a memory location instead */
}

static struct MachineDriver dynduke_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_V30, /* NEC V30-8 CPU */
			16000000, /* Guess */
			0,
			readmem,writemem,0,0,
			dynduke_interrupt,1
		},
		{
			CPU_V30, /* NEC V30-8 CPU */
			16000000, /* Guess */
			3,
			sub_readmem,sub_writemem,0,0,
			dynduke_interrupt,1
		},
#if 0
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,
			2,
			sound_readmem,sound_writemem,0,0,
			ignore_interrupt,0
		}
#endif
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	60,	/* CPU interleave  */
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },

	dynduke_gfxdecodeinfo,
	2048+1024, 2048+1024,
	0,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_BUFFERS_SPRITERAM,
	dynduke_eof_callback,
	dynduke_vh_start,
	0,
	dynduke_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_YM3812,
			&ym3812_interface
		},
		{
			SOUND_OKIM6295,
			&okim6295_interface
		}
	}
};

/***************************************************************************/

ROM_START( dynduke )
	ROM_REGION(0x100000) /* Region 0 - v30 main cpu */
	ROM_LOAD_V20_ODD ("dd1.cd8",   0x0a0000, 0x10000, 0xa5e2a95a )
	ROM_LOAD_V20_EVEN("dd2.cd7",   0x0a0000, 0x10000, 0x7e51af22 )
	ROM_LOAD_V20_ODD ("dd3.ef8",   0x0c0000, 0x20000, 0xa56f8692 )
	ROM_LOAD_V20_EVEN("dd4.ef7",   0x0c0000, 0x20000, 0xee4b87b3 )

	ROM_REGION_DISPOSE(0x4a0000)	/* Region 1 - Graphics */
	ROM_LOAD( "dd9.jk5",	0x000000, 0x04000, 0xf2bc9af4 ) /* chars */
	ROM_LOAD( "dd10.jk3",	0x010000, 0x04000, 0xc2a9f19b )

	ROM_LOAD( "dd.a2",		0x020000, 0x40000, 0x598f343f ) /* background */
	ROM_LOAD( "dd.b2",		0x060000, 0x40000, 0x41a9088d )
	ROM_LOAD( "dd.c2",		0x0a0000, 0x40000, 0xcc341b42 )
	ROM_LOAD( "dd.d2",		0x0e0000, 0x40000, 0x4752b4d7 )
	ROM_LOAD( "dd.de3",		0x120000, 0x40000, 0x44a4cb62 )
	ROM_LOAD( "dd.ef3",		0x160000, 0x40000, 0xaa8aee1a )

	ROM_LOAD( "dd.mn3",		0x1a0000, 0x40000, 0x2ee0ca98 ) /* foreground */
	ROM_LOAD( "dd.mn4",		0x1e0000, 0x40000, 0x6c71e2df )
	ROM_LOAD( "dd.n45",		0x220000, 0x40000, 0x85d918e1 )
	ROM_LOAD( "dd.mn5",		0x260000, 0x40000, 0xe71e34df )

	ROM_LOAD_GFX_EVEN(  "dd.n1", 0x2a0000, 0x40000, 0xcf1db927 ) /* sprites */
	ROM_LOAD_GFX_ODD (  "dd.n2", 0x2a0000, 0x40000, 0x5328150f )
	ROM_LOAD_GFX_EVEN(  "dd.m1", 0x320000, 0x40000, 0x80776452 )
	ROM_LOAD_GFX_ODD (  "dd.m2", 0x320000, 0x40000, 0xff61a573 )
	ROM_LOAD_GFX_EVEN(  "dd.e1", 0x3a0000, 0x40000, 0x84a0b87c )
	ROM_LOAD_GFX_ODD (  "dd.e2", 0x3a0000, 0x40000, 0xa9585df2 )
	ROM_LOAD_GFX_EVEN(  "dd.f1", 0x420000, 0x40000, 0x9aed24ba )
	ROM_LOAD_GFX_ODD (  "dd.f2", 0x420000, 0x40000, 0x3eb5783f )

	ROM_REGION(0x18000)	 /* Region 2 - 64k code for sound Z80 */
	ROM_LOAD( "dd8.w8", 0x000000, 0x08000, 0x3c29480b )
	ROM_CONTINUE(       0x010000, 0x08000 )

	ROM_REGION(0x100000) /* Region 3 - v30 sub cpu */
	ROM_LOAD_V20_ODD ("dd5.p8",  0x0e0000, 0x10000, 0x883d319c )
	ROM_LOAD_V20_EVEN("dd6.p7",  0x0e0000, 0x10000, 0xd94cb4ff )

	ROM_REGION(0x10000)	 /* Region 4 - ADPCM samples */
	ROM_LOAD( "dd7.x10", 0x000000, 0x10000, 0x9cbc7b41 )
ROM_END

ROM_START( dbldyn )
	ROM_REGION(0x100000) /* Region 0 - v30 main cpu */
	ROM_LOAD_V20_ODD ("dd1.cd8", 0x0a0000, 0x10000, 0xa5e2a95a )
	ROM_LOAD_V20_EVEN("dd2.cd7", 0x0a0000, 0x10000, 0x7e51af22 )
	ROM_LOAD_V20_ODD ("3.8e",    0x0c0000, 0x20000, 0x9b785028 )
	ROM_LOAD_V20_EVEN("4.7e",    0x0c0000, 0x20000, 0x0d0f6350 )

	ROM_REGION_DISPOSE(0x4a0000)	/* Region 1 - Graphics */
	ROM_LOAD( "9.5k",	0x004000, 0x4000, 0x16bec703 ) /* chars */
	ROM_CONTINUE(       0x000000, 0x4000 )
	ROM_CONTINUE(       0x008000, 0x8000 )
	ROM_LOAD( "10.4k",	0x014000, 0x4000, 0x719f909d )
	ROM_CONTINUE(       0x010000, 0x4000 )
	ROM_CONTINUE(       0x008000, 0x8000 )

	ROM_LOAD( "dd.a2",		0x020000, 0x40000, 0x598f343f ) /* background */
	ROM_LOAD( "dd.b2",		0x060000, 0x40000, 0x41a9088d )
	ROM_LOAD( "dd.c2",		0x0a0000, 0x40000, 0xcc341b42 )
	ROM_LOAD( "dd.d2",		0x0e0000, 0x40000, 0x4752b4d7 )
	ROM_LOAD( "dd.de3",		0x120000, 0x40000, 0x44a4cb62 )
	ROM_LOAD( "dd.ef3",		0x160000, 0x40000, 0xaa8aee1a )

	ROM_LOAD( "dd.mn3",		0x1a0000, 0x40000, 0x2ee0ca98 ) /* foreground */
	ROM_LOAD( "dd.mn4",		0x1e0000, 0x40000, 0x6c71e2df )
	ROM_LOAD( "dd.n45",		0x220000, 0x40000, 0x85d918e1 )
	ROM_LOAD( "dd.mn5",		0x260000, 0x40000, 0xe71e34df )

	ROM_LOAD_GFX_EVEN(  "dd.n1", 0x2a0000, 0x40000, 0xcf1db927 ) /* sprites */
	ROM_LOAD_GFX_ODD (  "dd.n2", 0x2a0000, 0x40000, 0x5328150f )
	ROM_LOAD_GFX_EVEN(  "dd.m1", 0x320000, 0x40000, 0x80776452 )
	ROM_LOAD_GFX_ODD (  "dd.m2", 0x320000, 0x40000, 0xff61a573 )
	ROM_LOAD_GFX_EVEN(  "dd.e1", 0x3a0000, 0x40000, 0x84a0b87c )
	ROM_LOAD_GFX_ODD (  "dd.e2", 0x3a0000, 0x40000, 0xa9585df2 )
	ROM_LOAD_GFX_EVEN(  "dd.f1", 0x420000, 0x40000, 0x9aed24ba )
	ROM_LOAD_GFX_ODD (  "dd.f2", 0x420000, 0x40000, 0x3eb5783f )

	ROM_REGION(0x18000)	 /* Region 2 - 64k code for sound Z80 */
	ROM_LOAD( "8.8w", 0x000000, 0x08000, 0xf4066081 )
	ROM_CONTINUE(     0x010000, 0x08000 )

	ROM_REGION(0x100000) /* Region 3 - v30 sub cpu */
	ROM_LOAD_V20_ODD ("5.8p",  0x0e0000, 0x10000, 0xea56d719 )
	ROM_LOAD_V20_EVEN("6.7p",  0x0e0000, 0x10000, 0x9ffa0ecd )

	ROM_REGION(0x10000)	 /* Region 4 - ADPCM samples */
	ROM_LOAD( "dd7.x10", 0x000000, 0x10000, 0x9cbc7b41 )
ROM_END

/***************************************************************************/

/* NOTICE!  This is not currently correct, this table works for the first
128 bytes, but goes wrong after that.  I suspect the bytes in this table
are shifted according to an address line.  I have not confirmed the pattern
repeats after 128 bytes, it may be more...

To help with decryption - compare this to the Raiden sound cpu program,
the first 0x1000 bytes at least should be identical.

There is also a 0xff fill at the end of the rom.

*/
static void dynduke_decrypt(void)
{
	unsigned char *RAM = Machine->memory_region[2];
	int xor_table[128]={
		0x00,0x00,0x10,0x10,0x08,0x00,0x00,0x18,
		0x00,0x00,0x10,0x10,0x08,0x08,0x18,0x18,

		0x00,0x00,0x00,0x10,0x08,0x08,0x18,0x18,
		0x00,0x00,0x00,0x10,0x08,0x08,0x18,0x18,

		0x00,0x00,0x10,0x10,0x08,0x08,0x18,0x18,
		0x00,0x00,0x10,0x10,0x08,0x08,0x18,0x18,

		0x00,0x00,0x10,0x10,0x08,0x08,0x18,0x18,
		0x00,0x00,0x10,0x10,0x08,0x08,0x18,0x18,

		0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,
		0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,

		0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,
		0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,

		0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,
		0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,

		0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,
		0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,
	};
	int i;

	for (i=0; i<0x18000; i++)
		RAM[i]=RAM[i]^xor_table[i%128];
}

/***************************************************************************/

struct GameDriver driver_dynduke =
{
	__FILE__,
	0,
	"dynduke",
	"Dynamite Duke",
	"1989",
	"Seibu Kaihatsu (Fabtek license)",
	"Bryan McPhail",
	0,
	&dynduke_machine_driver,
	0,

	rom_dynduke,
	dynduke_decrypt, 0,
	0, 0,
	input_ports_dynduke,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NO_SOUND,

	0, 0
};

struct GameDriver driver_dbldyn =
{
	__FILE__,
	&driver_dynduke,
	"dbldyn",
	"The Double Dynamites",
	"1989",
	"Seibu Kaihatsu (Fabtek license)",
	"Bryan McPhail",
	0,
	&dynduke_machine_driver,
	0,

	rom_dbldyn,
	dynduke_decrypt, 0,
	0, 0,
	input_ports_dynduke,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NO_SOUND,

	0, 0
};
