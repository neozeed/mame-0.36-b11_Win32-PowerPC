/*
    Driver For DECO   ASTRO FIGHTER/TOMAHAWK 777

    Initial Version

    Lee Taylor 28/11/1997


	Astro Fighter Sets:

    The differences are minor. From newest to oldest:

	Main Set: 16Kbit ROMs
	          Green/Hollow empty fuel bar.
			  60 points for every bomb destroyed.

	Set 2:    8Kbit ROMs
			  Blue/Solid empty fuel bar.
			  60 points for every bomb destroyed.

	Set 3:    8Kbit ROMs
			  Blue/Solid empty fuel bar.
			  300 points for every seven bombs destroyed.


To Do!!
	   Figure out the correct vblank interval. The one I'm using seems to be
	   ok for Astro Fighter, but the submarine in Tomahawk flickers.
	   Maybe the video rate should 57FPS as the Burger Time games?

	   Rotation Support

Also....
        I know there must be at least one other rom set for Astro Fighter
        I have played one that stoped between waves to show the next enemy
*/

#include "driver.h"
#include "vidhrdw/generic.h"

extern unsigned char *astrof_color;
extern unsigned char *tomahawk_protection;

void astrof_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
int  astrof_vh_start(void);
void astrof_vh_stop(void);
void astrof_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void astrof_videoram_w(int offset,int data);
void tomahawk_videoram_w(int offset,int data);
void astrof_video_control1_w(int offset,int data);
void astrof_video_control2_w(int offset,int data);
void tomahawk_video_control2_w(int offset,int data);
int  tomahawk_protection_r(int offset);
void astrof_sample1_w(int offset,int data);
void astrof_sample2_w(int offset,int data);

extern struct Samplesinterface astrof_samples_interface;
extern struct Samplesinterface tomahawk_samples_interface;

static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x03ff, MRA_RAM },
	{ 0x4000, 0x5fff, MRA_RAM },
	{ 0xa000, 0xa000, input_port_0_r },
	{ 0xa001, 0xa001, input_port_1_r },	/* IN1 */
	{ 0xa003, 0xa003, tomahawk_protection_r },   // Only on Tomahawk
	{ 0xd000, 0xffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress astrof_writemem[] =
{
	{ 0x0000, 0x03ff, MWA_RAM },
	{ 0x4000, 0x5fff, astrof_videoram_w, &videoram, &videoram_size },
	{ 0x8003, 0x8003, MWA_RAM, &astrof_color },
	{ 0x8004, 0x8004, astrof_video_control1_w },
	{ 0x8005, 0x8005, astrof_video_control2_w },
	{ 0x8006, 0x8006, astrof_sample1_w },
	{ 0x8007, 0x8007, astrof_sample2_w },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress tomahawk_writemem[] =
{
	{ 0x0000, 0x03ff, MWA_RAM },
	{ 0x4000, 0x5fff, tomahawk_videoram_w, &videoram, &videoram_size },
	{ 0x8003, 0x8003, MWA_RAM, &astrof_color },
	{ 0x8004, 0x8004, astrof_video_control1_w },
	{ 0x8005, 0x8005, tomahawk_video_control2_w },
	{ 0x8006, 0x8006, MWA_NOP },                        // Sound triggers
	{ 0x8007, 0x8007, MWA_RAM, &tomahawk_protection },
	{ -1 }	/* end of table */
};



/***************************************************************************

  These games don't have VBlank interrupts.
  Interrupts are still used by the game: but they are related to coin
  slots.

***************************************************************************/
static int astrof_interrupt(void)
{
	if (readinputport(2) & 1)	/* Coin */
		return nmi_interrupt();
	else return ignore_interrupt();
}


INPUT_PORTS_START( astrof )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
/* Player 1 Controls */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
/* Player 2 Controls */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )

	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
/* 0x0c gives 2 Coins/1 Credit */

	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x20, "7000" )
	PORT_DIPSETTING(    0x30, "None" )

	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Easy" )
	PORT_DIPSETTING(    0x40, "Hard" )

	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )

	PORT_START	/* FAKE */
	/* The coin slots are not memory mapped. Coin insertion causes a NMI. */
	/* This fake input port is used by the interrupt */
	/* handler to be notified of coin insertions. We use IMPULSE to */
	/* trigger exactly one interrupt, without having to check when the */
	/* user releases the key. */
	/* The cabinet selector is not memory mapped, but just disables the */
	/* screen flip logic */
	PORT_BIT_IMPULSE( 0x01, IP_ACTIVE_HIGH, IPT_COIN1, 1 )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
INPUT_PORTS_END


INPUT_PORTS_START( tomahawk )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )

	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
/* 0x0c gives 2 Coins/1 Credit */

	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "7000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x30, "None" )

	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )  /* Only on Tomahawk 777 */
	PORT_DIPSETTING(    0x00, "Easy" )
	PORT_DIPSETTING(    0x40, "Hard" )

	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )

	PORT_START	/* FAKE */
	/* The coin slots are not memory mapped. Coin insertion causes a NMI. */
	/* This fake input port is used by the interrupt */
	/* handler to be notified of coin insertions. We use IMPULSE to */
	/* trigger exactly one interrupt, without having to check when the */
	/* user releases the key. */
	/* The cabinet selector is not memory mapped, but just disables the */
	/* screen flip logic */
	PORT_BIT_IMPULSE( 0x01, IP_ACTIVE_HIGH, IPT_COIN1, 1 )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
INPUT_PORTS_END


#define MACHINE_DRIVER(GAMENAME, NUMCOLORS) 								   \
static struct MachineDriver GAMENAME##_machine_driver =						   \
{																			   \
	/* basic machine hardware */											   \
	{																		   \
		{																	   \
			CPU_M6502,														   \
			10595000/16,	/* 0.66 Mhz */									   \
			0,																   \
			readmem,GAMENAME##_writemem,0,0,								   \
			astrof_interrupt,1												   \
		}																	   \
	},																		   \
	60, 3400,	/* frames per second, vblank duration */				       \
	1,	/* single CPU, no need for interleaving */							   \
	0,																		   \
																			   \
	/* video hardware */													   \
	256, 256,                               /* screen_width, screen_height */  \
	{ 8, 256-1-8, 8, 256-1-8 },             /* struct rectangle visible_area */\
																			   \
	0,	/* no gfxdecodeinfo - bitmapped display */							   \
	NUMCOLORS, 0,															   \
	astrof_vh_convert_color_prom,											   \
																			   \
	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,								   \
	0,																		   \
	astrof_vh_start,														   \
	astrof_vh_stop,															   \
	astrof_vh_screenrefresh,												   \
																			   \
	/* sound hardware */													   \
	0, 0, 0, 0,																   \
	{						 												   \
		{																	   \
			SOUND_SAMPLES,		 											   \
			&GAMENAME##_samples_interface	 								   \
		}																	   \
	}																		   \
};

MACHINE_DRIVER(astrof,   16)
MACHINE_DRIVER(tomahawk, 32)



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( astrof )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "afii.6",       0xd000, 0x0800, 0xd6cd13a4 )
	ROM_LOAD( "afii.5",       0xd800, 0x0800, 0x6fd3c4df )
	ROM_LOAD( "afii.4",       0xe000, 0x0800, 0x9612dae3 )
	ROM_LOAD( "afii.3",       0xe800, 0x0800, 0x5a0fef42 )
	ROM_LOAD( "afii.2",       0xf000, 0x0800, 0x69f8a4fc )
	ROM_LOAD( "afii.1",       0xf800, 0x0800, 0x322c09d2 )

	ROM_REGIONX( 0x0020, REGION_PROMS )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, 0x61329fd1 )
ROM_END

ROM_START( astrof2 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "kei2",         0xd000, 0x0400, 0x9f0bd355 )
	ROM_LOAD( "keii",         0xd400, 0x0400, 0x71f229f0 )
	ROM_LOAD( "kei0",         0xd800, 0x0400, 0x88114f7c )
	ROM_LOAD( "af579.08",     0xdc00, 0x0400, 0x9793c124 )
	ROM_LOAD( "ke8",          0xe000, 0x0400, 0x08e44b12 )
	ROM_LOAD( "ke7",          0xe400, 0x0400, 0x8a42d62c )
	ROM_LOAD( "ke6",          0xe800, 0x0400, 0x3e9aa743 )
	ROM_LOAD( "ke5",          0xec00, 0x0400, 0x712a4557 )
	ROM_LOAD( "ke4",          0xf000, 0x0400, 0xad06f306 )
	ROM_LOAD( "ke3",          0xf400, 0x0400, 0x680b91b4 )
	ROM_LOAD( "ke2",          0xf800, 0x0400, 0x2c4cab1a )
	ROM_LOAD( "af583.00",     0xfc00, 0x0400, 0xf699dda3 )

	ROM_REGIONX( 0x0020, REGION_PROMS )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, 0x61329fd1 )
ROM_END

ROM_START( astrof3 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "kei2",         0xd000, 0x0400, 0x9f0bd355 )
	ROM_LOAD( "keii",         0xd400, 0x0400, 0x71f229f0 )
	ROM_LOAD( "kei0",         0xd800, 0x0400, 0x88114f7c )
	ROM_LOAD( "ke9",          0xdc00, 0x0400, 0x29cbaea6 )
	ROM_LOAD( "ke8",          0xe000, 0x0400, 0x08e44b12 )
	ROM_LOAD( "ke7",          0xe400, 0x0400, 0x8a42d62c )
	ROM_LOAD( "ke6",          0xe800, 0x0400, 0x3e9aa743 )
	ROM_LOAD( "ke5",          0xec00, 0x0400, 0x712a4557 )
	ROM_LOAD( "ke4",          0xf000, 0x0400, 0xad06f306 )
	ROM_LOAD( "ke3",          0xf400, 0x0400, 0x680b91b4 )
	ROM_LOAD( "ke2",          0xf800, 0x0400, 0x2c4cab1a )
	ROM_LOAD( "kei",          0xfc00, 0x0400, 0xfce4718d )

	ROM_REGIONX( 0x0020, REGION_PROMS )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, 0x61329fd1 )
ROM_END

ROM_START( tomahawk )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "l8-1",         0xdc00, 0x0400, 0x7c911661 )
	ROM_LOAD( "l7-1",         0xe000, 0x0400, 0xadeffb69 )
	ROM_LOAD( "l6-1",         0xe400, 0x0400, 0x9116e59d )
	ROM_LOAD( "l5-1",         0xe800, 0x0400, 0x01e4c7c4 )
	ROM_LOAD( "l4-1",         0xec00, 0x0400, 0xd9f69cb0 )
	ROM_LOAD( "l3-1",         0xf000, 0x0400, 0x7ce7183f )
	ROM_LOAD( "l2-1",         0xf400, 0x0400, 0x43fea29d )
	ROM_LOAD( "l1-1",         0xf800, 0x0400, 0xf2096ba9 )
	ROM_LOAD( "l0-1",         0xfc00, 0x0400, 0x42edbc28 )

	ROM_REGIONX( 0x0020, REGION_PROMS )
	ROM_LOAD( "t777.clr",     0x0000, 0x0020, 0xd6a528fd )
ROM_END

ROM_START( tomahaw5 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "thawk.l8",     0xdc00, 0x0400, 0xb01dab4b )
	ROM_LOAD( "thawk.l7",     0xe000, 0x0400, 0x3a6549e8 )
	ROM_LOAD( "thawk.l6",     0xe400, 0x0400, 0x863e47f7 )
	ROM_LOAD( "thawk.l5",     0xe800, 0x0400, 0xde0183bc )
	ROM_LOAD( "thawk.l4",     0xec00, 0x0400, 0x11e9c7ea )
	ROM_LOAD( "thawk.l3",     0xf000, 0x0400, 0xec44d388 )
	ROM_LOAD( "thawk.l2",     0xf400, 0x0400, 0xdc0a0f54 )
	ROM_LOAD( "thawk.l1",     0xf800, 0x0400, 0x1d9dab9c )
	ROM_LOAD( "thawk.l0",     0xfc00, 0x0400, 0xd21a1eba )

	ROM_REGIONX( 0x0020, REGION_PROMS )
	ROM_LOAD( "t777.clr",     0x0000, 0x0020, 0xd6a528fd )
ROM_END


static int astrof_hiload(void)
{
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);
	static int firsttime = 0;

	/* check if the hi score table has already been initialized */
	/* the high score table is intialized to all 0, so first of all */
	/* we dirty it, then we wait for it to be cleared again */

	if (firsttime == 0)
	{
		memset(&RAM[0x0084],0xff,2);
		firsttime = 1;
	}

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x0084],"\x00\x00",2) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x0084],2);
			osd_fclose(f);
		}
		firsttime = 0 ;
		return 1;
	}
	else return 0;   /* we can't load the hi scores yet */
}

static void astrof_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x0084],2);
		osd_fclose(f);
	}
}

static int tomahawk_hiload(void)
{
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);
	static int firsttime = 0;

	/* check if the hi score table has already been initialized */
	/* the high score table is intialized to all 0, so first of all */
	/* we dirty it, then we wait for it to be cleared again */

	if (firsttime == 0)
	{
		memset(&RAM[0x000d],0xff,2);
		firsttime = 1;
	}

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x000d],"\x00\x00",2) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x000d],2);
			osd_fclose(f);
		}
		firsttime = 0 ;

		return 1;
	}
	else return 0;   /* we can't load the hi scores yet */
}

static void tomahawk_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x000d],2);
		osd_fclose(f);
	}
}



#define CREDITS "Lee Taylor\ndedicated to Lucy Anne Taylor\nZsolt Vasvari"

struct GameDriver driver_astrof =
{
	__FILE__,
	0,
	"astrof",
	"Astro Fighter (set 1)",
	"1980",
	"Data East",
	CREDITS,
	0,
	&astrof_machine_driver,
	0,

	rom_astrof,
	0, 0,
	0,
	0,

	input_ports_astrof,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	astrof_hiload, astrof_hisave
};

struct GameDriver driver_astrof2 =
{
	__FILE__,
	&driver_astrof,
	"astrof2",
	"Astro Fighter (set 2)",
	"1980",
	"Data East",
	CREDITS,
	0,
	&astrof_machine_driver,
	0,

	rom_astrof2,
	0, 0,
	0,
	0,

	input_ports_astrof,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	astrof_hiload, astrof_hisave
};

struct GameDriver driver_astrof3 =
{
	__FILE__,
	&driver_astrof,
	"astrof3",
	"Astro Fighter (set 3)",
	"1980",
	"Data East",
	CREDITS,
	0,
	&astrof_machine_driver,
	0,

	rom_astrof3,
	0, 0,
	0,
	0,

	input_ports_astrof,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	astrof_hiload, astrof_hisave
};

struct GameDriver driver_tomahawk =
{
	__FILE__,
	0,
	"tomahawk",
	"Tomahawk 777 (Revision 1)",
	"1980",
	"Data East",
	CREDITS,
	0,
	&tomahawk_machine_driver,
	0,

	rom_tomahawk,
	0, 0,
	0,
	0,

	input_ports_tomahawk,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	tomahawk_hiload, tomahawk_hisave

};

struct GameDriver driver_tomahaw5 =
{
	__FILE__,
	&driver_tomahawk,  				// Only the color PROM is shared
	"tomahaw5",
	"Tomahawk 777 (Revision 5)",
	"1980",
	"Data East",
	CREDITS,
	0,
	&tomahawk_machine_driver,
	0,

	rom_tomahaw5,
	0, 0,
	0,
	0,

	input_ports_tomahawk,

	0, 0, 0,
	ORIENTATION_ROTATE_90,

	tomahawk_hiload, tomahawk_hisave
};
