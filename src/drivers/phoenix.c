/***************************************************************************
Note:
   pleiads is using another sound driver, sndhrdw\pleiads.c
 Andrew Scott (ascott@utkux.utcc.utk.edu)

Phoenix memory map

0000-3fff 16Kb Program ROM
4000-43ff 1Kb Video RAM Charset A (4340-43ff variables)
4800-4bff 1Kb Video RAM Charset B (4b40-4bff variables)
5000-53ff 1Kb Video Control write-only (mirrored)
5800-5bff 1Kb Video Scroll Register (mirrored)
6000-63ff 1Kb Sound Control A (mirrored)
6800-6bff 1Kb Sound Control B (mirrored)
7000-73ff 1Kb 8bit Game Control read-only (mirrored)
7800-7bff 1Kb 8bit Dip Switch read-only (mirrored)

memory mapped ports:

read-only:
7000-73ff IN
7800-7bff DSW

 * IN (all bits are inverted)
 * bit 7 : Shield
 * bit 6 : Left
 * bit 5 : Right
 * bit 4 : Fire
 * bit 3 : -
 * bit 2 : Start 2
 * bit 1 : Start 1
 * bit 0 : Coin

 * DSW
 * bit 7 : VBlank
 * bit 6 : free play (pleiads only)
 * bit 5 : attract sound 0 = off 1 = on (pleiads only?)
 * bit 4 : coins per play  0 = 1 coin  1 = 2 coins
 * bit 3 :\ bonus
 * bit 2 :/ 00 = 3000  01 = 4000  10 = 5000  11 = 6000
 * bit 1 :\ number of lives
 * bit 0 :/ 00 = 3  01 = 4  10 = 5  11 = 6

***************************************************************************/

#include "driver.h"


int  phoenix_paged_ram_r (int offset);
void phoenix_paged_ram_w (int offset,int data);
void phoenix_videoreg_w (int offset,int data);
void phoenix_scroll_w (int offset,int data);
int  phoenix_input_port_0_r (int offset);
void phoenix_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
int  phoenix_vh_start(void);
void phoenix_vh_stop(void);
void phoenix_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

void phoenix_sound_control_a_w(int offset, int data);
void phoenix_sound_control_b_w(int offset, int data);
int phoenix_sh_start(const struct MachineSound *msound);
void phoenix_sh_update(void);

void pleiads_sound_control_a_w(int offset, int data);
void pleiads_sound_control_b_w(int offset, int data);
int pleiads_sh_start(const struct MachineSound *msound);
void pleiads_sh_update(void);


static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x3fff, MRA_ROM },
	{ 0x4000, 0x4fff, phoenix_paged_ram_r },	/* 2 pages selected by Bit 0 of videoregister */
	{ 0x7000, 0x73ff, phoenix_input_port_0_r },	/* IN0 */
	{ 0x7800, 0x7bff, input_port_1_r },	        /* DSW */
	{ -1 }	/* end of table */
};


#define WRITEMEM(GAMENAME)										\
																\
static struct MemoryWriteAddress GAMENAME##_writemem[] =		\
{																\
	{ 0x0000, 0x3fff, MWA_ROM },								\
	{ 0x4000, 0x4fff, phoenix_paged_ram_w },  /* 2 pages selected by Bit 0 of the video register */	\
	{ 0x5000, 0x53ff, phoenix_videoreg_w },						\
	{ 0x5800, 0x5bff, phoenix_scroll_w },	/* the game sometimes writes at mirror addresses */		\
	{ 0x6000, 0x63ff, GAMENAME##_sound_control_a_w },			\
	{ 0x6800, 0x6bff, GAMENAME##_sound_control_b_w },			\
	{ -1 }	/* end of table */	   								\
};

WRITEMEM(phoenix)
WRITEMEM(pleiads)



INPUT_PORTS_START( phoenix )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x0c, "6000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END

INPUT_PORTS_START( phoenixa )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x0c, "6000" )
	/* Coinage is backwards from phoenix (Amstar) */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


INPUT_PORTS_START( phoenixt )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x0c, "6000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END

INPUT_PORTS_START( phoenix3 )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x0c, "6000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


INPUT_PORTS_START( pleiads )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	   /* Protection. See 0x0552 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x0c, "6000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	2,	/* 2 bits per pixel */
	{ 256*8*8, 0 },	/* the two bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },	/* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,    0, 16 },
	{ 1, 0x1000, &charlayout, 16*4, 16 },
	{ -1 } /* end of array */
};



static struct CustomSound_interface phoenix_custom_interface =
{
	phoenix_sh_start,
	0,
	phoenix_sh_update
};

static struct CustomSound_interface pleiads_custom_interface =
{
	pleiads_sh_start,
	0,
	pleiads_sh_update
};

#define MACHINE_DRIVER(GAMENAME)									\
																	\
static struct MachineDriver GAMENAME##_machine_driver =				\
{																	\
	/* basic machine hardware */									\
	{																\
		{															\
			CPU_8080,												\
			3072000,	/* 3 Mhz ? */								\
			0,														\
			readmem,GAMENAME##_writemem,0,0,						\
			ignore_interrupt,1										\
		}															\
	},																\
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */	\
	1,	/* single CPU, no need for interleaving */					\
	0,																\
																	\
	/* video hardware */											\
	32*8, 32*8, { 0*8, 31*8-1, 0*8, 26*8-1 },						\
	gfxdecodeinfo,													\
	256,16*4+16*4,													\
	phoenix_vh_convert_color_prom,									\
																	\
	VIDEO_TYPE_RASTER,												\
	0,																\
	phoenix_vh_start,												\
	phoenix_vh_stop,												\
	phoenix_vh_screenrefresh,										\
																	\
	/* sound hardware */											\
	0,0,0,0,														\
	{																\
		{															\
			SOUND_CUSTOM,											\
			&GAMENAME##_custom_interface							\
		}															\
	}																\
};


MACHINE_DRIVER(phoenix)
MACHINE_DRIVER(pleiads)



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( phoenix )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic45",         0x0000, 0x0800, 0x9f68086b )
	ROM_LOAD( "ic46",         0x0800, 0x0800, 0x273a4a82 )
	ROM_LOAD( "ic47",         0x1000, 0x0800, 0x3d4284b9 )
	ROM_LOAD( "ic48",         0x1800, 0x0800, 0xcb5d9915 )
	ROM_LOAD( "ic49",         0x2000, 0x0800, 0xa105e4e7 )
	ROM_LOAD( "ic50",         0x2800, 0x0800, 0xac5e9ec1 )
	ROM_LOAD( "ic51",         0x3000, 0x0800, 0x2eab35b4 )
	ROM_LOAD( "ic52",         0x3800, 0x0800, 0xaff8e9c5 )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic23",         0x0000, 0x0800, 0x3c7e623f )
	ROM_LOAD( "ic24",         0x0800, 0x0800, 0x59916d3b )
	ROM_LOAD( "ic39",         0x1000, 0x0800, 0x53413e8f )
	ROM_LOAD( "ic40",         0x1800, 0x0800, 0x0be2ba91 )

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "ic40_b.bin",   0x0000, 0x0100, 0x79350b25 )	/* palette low bits */
	ROM_LOAD( "ic41_a.bin",   0x0100, 0x0100, 0xe176b768 )	/* palette high bits */
ROM_END

ROM_START( phoenixa )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic45.k1",      0x0000, 0x0800, 0xc7a9b499 )
	ROM_LOAD( "ic46.k2",      0x0800, 0x0800, 0xd0e6ae1b )
	ROM_LOAD( "ic47.k3",      0x1000, 0x0800, 0x64bf463a )
	ROM_LOAD( "ic48.k4",      0x1800, 0x0800, 0x1b20fe62 )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, 0x1a1ce0d0 )
	ROM_LOAD( "ic50",         0x2800, 0x0800, 0xac5e9ec1 )
	ROM_LOAD( "ic51",         0x3000, 0x0800, 0x2eab35b4 )
	ROM_LOAD( "ic52",         0x3800, 0x0800, 0xaff8e9c5 )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic23",         0x0000, 0x0800, 0x3c7e623f )
	ROM_LOAD( "ic24",         0x0800, 0x0800, 0x59916d3b )
	ROM_LOAD( "phoenixc.39",  0x1000, 0x0800, 0xbb0525ed )
	ROM_LOAD( "phoenixc.40",  0x1800, 0x0800, 0x4178aa4f )

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "ic40_b.bin",   0x0000, 0x0100, 0x79350b25 )	/* palette low bits */
	ROM_LOAD( "ic41_a.bin",   0x0100, 0x0100, 0xe176b768 )	/* palette high bits */
ROM_END

ROM_START( phoenixt )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, 0x5b8c55a8 )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, 0xdbc942fa )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, 0xcbbb8839 )
	ROM_LOAD( "phoenix.48",   0x1800, 0x0800, 0xcb65eff8 )
	ROM_LOAD( "phoenix.49",   0x2000, 0x0800, 0xc8a5d6d6 )
	ROM_LOAD( "ic50",         0x2800, 0x0800, 0xac5e9ec1 )
	ROM_LOAD( "ic51",         0x3000, 0x0800, 0x2eab35b4 )
	ROM_LOAD( "phoenix.52",   0x3800, 0x0800, 0xb9915263 )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic23",         0x0000, 0x0800, 0x3c7e623f )
	ROM_LOAD( "ic24",         0x0800, 0x0800, 0x59916d3b )
	ROM_LOAD( "ic39",         0x1000, 0x0800, 0x53413e8f )
	ROM_LOAD( "ic40",         0x1800, 0x0800, 0x0be2ba91 )

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "ic40_b.bin",   0x0000, 0x0100, 0x79350b25 )	/* palette low bits */
	ROM_LOAD( "ic41_a.bin",   0x0100, 0x0100, 0xe176b768 )	/* palette high bits */
ROM_END

ROM_START( phoenix3 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "phoenix3.45",  0x0000, 0x0800, 0xa362cda0 )
	ROM_LOAD( "phoenix3.46",  0x0800, 0x0800, 0x5748f486 )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, 0xcbbb8839 )
	ROM_LOAD( "phoenix3.48",  0x1800, 0x0800, 0xb5d97a4d )
	ROM_LOAD( "ic49",         0x2000, 0x0800, 0xa105e4e7 )
	ROM_LOAD( "ic50",         0x2800, 0x0800, 0xac5e9ec1 )
	ROM_LOAD( "ic51",         0x3000, 0x0800, 0x2eab35b4 )
	ROM_LOAD( "phoenix3.52",  0x3800, 0x0800, 0xd2c5c984 )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic23",         0x0000, 0x0800, 0x3c7e623f )
	ROM_LOAD( "ic24",         0x0800, 0x0800, 0x59916d3b )
	ROM_LOAD( "ic39",         0x1000, 0x0800, 0x53413e8f )
	ROM_LOAD( "ic40",         0x1800, 0x0800, 0x0be2ba91 )

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "ic40_b.bin",   0x0000, 0x0100, 0x79350b25 )	/* palette low bits */
	ROM_LOAD( "ic41_a.bin",   0x0100, 0x0100, 0xe176b768 )	/* palette high bits */
ROM_END

ROM_START( phoenixc )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, 0x5b8c55a8 )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, 0xdbc942fa )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, 0xcbbb8839 )
	ROM_LOAD( "phoenixc.48",  0x1800, 0x0800, 0x5ae0b215 )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, 0x1a1ce0d0 )
	ROM_LOAD( "ic50",         0x2800, 0x0800, 0xac5e9ec1 )
	ROM_LOAD( "ic51",         0x3000, 0x0800, 0x2eab35b4 )
	ROM_LOAD( "phoenixc.52",  0x3800, 0x0800, 0x8424d7c4 )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic23",         0x0000, 0x0800, 0x3c7e623f )
	ROM_LOAD( "ic24",         0x0800, 0x0800, 0x59916d3b )
	ROM_LOAD( "phoenixc.39",  0x1000, 0x0800, 0xbb0525ed )
	ROM_LOAD( "phoenixc.40",  0x1800, 0x0800, 0x4178aa4f )

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "ic40_b.bin",   0x0000, 0x0100, 0x79350b25 )	/* palette low bits */
	ROM_LOAD( "ic41_a.bin",   0x0100, 0x0100, 0xe176b768 )	/* palette high bits */
ROM_END

ROM_START( pleiads )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic47.r1",      0x0000, 0x0800, 0x960212c8 )
	ROM_LOAD( "ic48.r2",      0x0800, 0x0800, 0xb254217c )
	ROM_LOAD( "ic47.bin",     0x1000, 0x0800, 0x87e700bb ) /* IC 49 on real board */
	ROM_LOAD( "ic48.bin",     0x1800, 0x0800, 0x2d5198d0 ) /* IC 50 on real board */
	ROM_LOAD( "ic51.r5",      0x2000, 0x0800, 0x49c629bc )
	ROM_LOAD( "ic50.bin",     0x2800, 0x0800, 0xf1a8a00d ) /* IC 52 on real board */
	ROM_LOAD( "ic53.r7",      0x3000, 0x0800, 0xb5f07fbc )
	ROM_LOAD( "ic52.bin",     0x3800, 0x0800, 0xb1b5a8a6 ) /* IC 54 on real board */

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic23.bin",     0x0000, 0x0800, 0x4e30f9e7 ) /* IC 45 on real board */
	ROM_LOAD( "ic24.bin",     0x0800, 0x0800, 0x5188fc29 ) /* IC 44 on real board */
	ROM_LOAD( "ic39.bin",     0x1000, 0x0800, 0x85866607 ) /* IC 27 on real board */
	ROM_LOAD( "ic40.bin",     0x1800, 0x0800, 0xa841d511 ) /* IC 26 on real board */

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "7611-5.26",   0x0000, 0x0100, 0x7a1bcb1e )	/* palette low bits */
	ROM_LOAD( "7611-5.33",   0x0100, 0x0100, 0xe38eeb83 )	/* palette high bits */
ROM_END

ROM_START( pleiadbl )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic45.bin",     0x0000, 0x0800, 0x93fc2958 )
	ROM_LOAD( "ic46.bin",     0x0800, 0x0800, 0xe2b5b8cd )
	ROM_LOAD( "ic47.bin",     0x1000, 0x0800, 0x87e700bb )
	ROM_LOAD( "ic48.bin",     0x1800, 0x0800, 0x2d5198d0 )
	ROM_LOAD( "ic49.bin",     0x2000, 0x0800, 0x9dc73e63 )
	ROM_LOAD( "ic50.bin",     0x2800, 0x0800, 0xf1a8a00d )
	ROM_LOAD( "ic51.bin",     0x3000, 0x0800, 0x6f56f317 )
	ROM_LOAD( "ic52.bin",     0x3800, 0x0800, 0xb1b5a8a6 )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic23.bin",     0x0000, 0x0800, 0x4e30f9e7 )
	ROM_LOAD( "ic24.bin",     0x0800, 0x0800, 0x5188fc29 )
	ROM_LOAD( "ic39.bin",     0x1000, 0x0800, 0x85866607 )
	ROM_LOAD( "ic40.bin",     0x1800, 0x0800, 0xa841d511 )

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "7611-5.26",   0x0000, 0x0100, 0x7a1bcb1e )	/* palette low bits */
	ROM_LOAD( "7611-5.33",   0x0100, 0x0100, 0xe38eeb83 )	/* palette high bits */
ROM_END

ROM_START( pleiadce )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "pleiades.47",  0x0000, 0x0800, 0x711e2ba0 )
	ROM_LOAD( "pleiades.48",  0x0800, 0x0800, 0x93a36943 )
	ROM_LOAD( "ic47.bin",     0x1000, 0x0800, 0x87e700bb )
	ROM_LOAD( "pleiades.50",  0x1800, 0x0800, 0x5a9beba0 )
	ROM_LOAD( "pleiades.51",  0x2000, 0x0800, 0x1d828719 )
	ROM_LOAD( "ic50.bin",     0x2800, 0x0800, 0xf1a8a00d )
	ROM_LOAD( "pleiades.53",  0x3000, 0x0800, 0x037b319c )
	ROM_LOAD( "pleiades.54",  0x3800, 0x0800, 0xca264c7c )

	ROM_REGION_DISPOSE(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pleiades.45",  0x0000, 0x0800, 0x8dbd3785 )
	ROM_LOAD( "pleiades.44",  0x0800, 0x0800, 0x0db3e436 )
	ROM_LOAD( "ic39.bin",     0x1000, 0x0800, 0x85866607 )
	ROM_LOAD( "ic40.bin",     0x1800, 0x0800, 0xa841d511 )

	ROM_REGIONX( 0x0200, REGION_PROMS )
	ROM_LOAD( "7611-5.26",   0x0000, 0x0100, 0x7a1bcb1e )	/* palette low bits */
	ROM_LOAD( "7611-5.33",   0x0100, 0x0100, 0xe38eeb83 )	/* palette high bits */
ROM_END



static int hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x438a],"\x00\x00\x0f",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x4388],4);

			/* copy the high score to the screen */
			/*       // I suppose noone can do such an HISCORE!!! ;)
			phoenix_videoram2_w(0x0221, (RAM[0x4388] >> 4)+0x20);
			phoenix_videoram2_w(0x0201, (RAM[0x4388] & 0xf)+0x20);
			*/
			RAM[0x49e1] = (RAM[0x4389] >> 4) + 0x20;
			RAM[0x49c1] = (RAM[0x4389] & 0xf) + 0x20;
			RAM[0x49a1] = (RAM[0x438a] >> 4) + 0x20;
			RAM[0x4981] = (RAM[0x438a] & 0xf) + 0x20;
			RAM[0x4961] = (RAM[0x438b] >> 4) + 0x20;
			RAM[0x4941] = (RAM[0x438b] & 0xf) + 0x20;
			osd_fclose(f);
		}

		return 1;
	}
	else return 0; /* we can't load the hi scores yet */
}



static unsigned long get_score(unsigned char *score)
{
     return (score[3])+(256*score[2])+((unsigned long)(65536)*score[1])+((unsigned long)(65536)*256*score[0]);
}

static void hisave(void)
{
   unsigned long score1,score2,hiscore;
   void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


   score1 = get_score(&RAM[0x4380]);
   score2 = get_score(&RAM[0x4384]);
   hiscore = get_score(&RAM[0x4388]);

   if (score1 > hiscore) RAM += 0x4380;         /* check consistency */
   else if (score2 > hiscore) RAM += 0x4384;
   else RAM += 0x4388;

   if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
   {
      osd_fwrite(f,&RAM[0],4);
      osd_fclose(f);
   }
}


#define CREDITS   "Richard Davies\nBrad Oliver\nMirko Buffoni\nNicola Salmoria\nShaun Stephenson\nAndrew Scott\nTim Lindquist (color info)\nMarco Cassili"

struct GameDriver driver_phoenix =
{
	__FILE__,
	0,
	"phoenix",
	"Phoenix (Amstar)",
	"1980",
	"Amstar",
	CREDITS,
	0,
	&phoenix_machine_driver,
	0,

	rom_phoenix,
	0, 0,
	0,
	0,

	input_ports_phoenix,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver driver_phoenixa =
{
	__FILE__,
	&driver_phoenix,
	"phoenixa",
	"Phoenix (Centuri)",
	"1980",
	"Amstar (Centuri license)",
	CREDITS,
	0,
	&phoenix_machine_driver,
	0,

	rom_phoenixa,
	0, 0,
	0,
	0,

	input_ports_phoenixa,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver driver_phoenixt =
{
	__FILE__,
	&driver_phoenix,
	"phoenixt",
	"Phoenix (Taito)",
	"1980",
	"Taito",
	CREDITS,
	0,
	&phoenix_machine_driver,
	0,

	rom_phoenixt,
	0, 0,
	0,
	0,

	input_ports_phoenixt,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver driver_phoenix3 =
{
	__FILE__,
	&driver_phoenix,
	"phoenix3",
	"Phoenix (T.P.N.)",
	"1980",
	"bootleg",
	CREDITS,
	0,
	&phoenix_machine_driver,
	0,

	rom_phoenix3,
	0, 0,
	0,
	0,

	input_ports_phoenix3,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver driver_phoenixc =
{
	__FILE__,
	&driver_phoenix,
	"phoenixc",
	"Phoenix (IRECSA, G.G.I Corp)",
	"1981",
	"bootleg?",
	CREDITS,
	0,
	&phoenix_machine_driver,
	0,

	rom_phoenixc,
	0, 0,
	0,
	0,

	input_ports_phoenixt,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};


struct GameDriver driver_pleiads =
{
	__FILE__,
	0,
	"pleiads",
	"Pleiads (Tehkan)",
	"1981",
	"Tehkan",
	CREDITS,
	0,
	&pleiads_machine_driver,
	0,

	rom_pleiads,
	0, 0,
	0,
	0,

	input_ports_pleiads,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver driver_pleiadbl =
{
	__FILE__,
	&driver_pleiads,
	"pleiadbl",
	"Pleiads (bootleg)",
	"1981",
	"bootleg",
	CREDITS,
	0,
	&pleiads_machine_driver,
	0,

	rom_pleiadbl,
	0, 0,
	0,
	0,

	input_ports_pleiads,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver driver_pleiadce =
{
	__FILE__,
	&driver_pleiads,
	"pleiadce",
	"Pleiads (Centuri)",
	"1981",
	"Tehkan (Centuri license)",
	CREDITS,
	0,
	&pleiads_machine_driver,
	0,

	rom_pleiadce,
	0, 0,
	0,
	0,

	input_ports_pleiads,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};
