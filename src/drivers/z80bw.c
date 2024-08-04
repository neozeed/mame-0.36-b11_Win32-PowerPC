/***************************************************************************

Midway?? Z80 board memory map (preliminary)

0000-1bff ROM
2000-23ff RAM
2400-3fff Video RAM  (also writes between 4000 and 4fff, but only to simplify
                      code, ie. doesn't do clipping when the spaceship scrolls in)

Games which are referenced by this driver:
------------------------------------------
Astro Invader (astinvad)
Kamikaze (kamikaze)
Space Intruder (spaceint)

I/O ports:
read:
08        IN0
09        IN1
0a        IN2

write:
04        sound
05        sound
07		  ???
0b		  ???

TODO:

- How many sets of controls are there on an upright machine?

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"

int  astinvad_interrupt(void);

void z80bw_init_palette(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
void astinvad_videoram_w(int offset,int data);
void spaceint_videoram_w (int offset,int data);
void astinvad_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void spaceint_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
void astinvad_sh_port_4_w(int offset,int data);
void astinvad_sh_port_5_w(int offset,int data);
void astinvad_sh_update(void);

struct Samplesinterface astinvad_samples_interface;


static struct MemoryReadAddress astinvad_readmem[] =
{
	{ 0x0000, 0x1bff, MRA_ROM },
	{ 0x1c00, 0x3fff, MRA_RAM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress astinvad_writemem[] =
{
	{ 0x0000, 0x1bff, MWA_ROM },
	{ 0x1c00, 0x23ff, MWA_RAM },
	{ 0x2400, 0x3fff, astinvad_videoram_w, &videoram, &videoram_size },
	{ -1 }  /* end of table */
};


static struct IOReadPort astinvad_readport[] =
{
	{ 0x08, 0x08, input_port_1_r },
	{ 0x09, 0x09, input_port_2_r },
	{ 0x0a, 0x0a, input_port_3_r },
	{ -1 }  /* end of table */
};

static struct IOWritePort astinvad_writeport[] =
{
	{ 0x04, 0x04, astinvad_sh_port_4_w },
	{ 0x05, 0x05, astinvad_sh_port_5_w },
	{ -1 }  /* end of table */
};


INPUT_PORTS_START( astinvad )
	PORT_START	/* FAKE - select cabinet type */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_2WAY )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x88, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_2WAY | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_2WAY | IPF_PLAYER2 )

	PORT_START      /* IN2 */
	PORT_DIPNAME( 0x01, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static struct MachineDriver astinvad_machine_driver = /* LT */
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			2000000,        /* 2 Mhz? */
			0,
			astinvad_readmem,astinvad_writemem,astinvad_readport,astinvad_writeport,
			interrupt,1    /* two interrupts per frame */
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,       /* frames per second, vblank duration */
	1,      /* single CPU, no need for interleaving */
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 0*8, 28*8-1 },
	0,      /* no gfxdecodeinfo - bitmapped display */
	8, 0,
	z80bw_init_palette,

	VIDEO_TYPE_RASTER | VIDEO_SUPPORTS_DIRTY | VIDEO_MODIFIES_PALETTE,
	0,
	0,
	0,
	astinvad_vh_screenrefresh,

	/* sound hardware */
	0, 0, 0, 0,
	{
		{
			SOUND_SAMPLES,
			&astinvad_samples_interface
		}
	}
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( astinvad )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "ai_cpu_1.rom", 0x0000, 0x0400, 0x20e3ec41 )
	ROM_LOAD( "ai_cpu_2.rom", 0x0400, 0x0400, 0xe8f1ab55 )
	ROM_LOAD( "ai_cpu_3.rom", 0x0800, 0x0400, 0xa0092553 )
	ROM_LOAD( "ai_cpu_4.rom", 0x0c00, 0x0400, 0xbe14185c )
	ROM_LOAD( "ai_cpu_5.rom", 0x1000, 0x0400, 0xfee681ec )
	ROM_LOAD( "ai_cpu_6.rom", 0x1400, 0x0400, 0xeb338863 )
	ROM_LOAD( "ai_cpu_7.rom", 0x1800, 0x0400, 0x16dcfea4 )

	ROM_REGIONX(0x0400, REGION_PROMS)
	ROM_LOAD( "ai_vid_c.rom", 0x0000, 0x0400, 0xb45287ff )
ROM_END

ROM_START( kamikaze )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "km01",         0x0000, 0x0800, 0x8aae7414 )
	ROM_LOAD( "km02",         0x0800, 0x0800, 0x6c7a2beb )
	ROM_LOAD( "km03",         0x1000, 0x0800, 0x3e8dedb6 )
	ROM_LOAD( "km04",         0x1800, 0x0800, 0x494e1f6d )

	ROM_REGIONX(0x0400, REGION_PROMS)
	ROM_LOAD( "ai_vid_c.rom", 0x0000, 0x0400, BADCRC(0xb45287ff) )
ROM_END


/*****************************************************************************/
/* Highscore save and load HSC 11/04/98										 */


static int astinvad_hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if (memcmp(&RAM[0x1cff],"\x08\x20\x03",3) == 0 )
    {
        void *f;

        if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
        {
            osd_fread(f,&RAM[0x1fc9],19);
            osd_fclose(f);
        }

        return 1;
    }

    else
    return 0;  /* we can't load the hi scores yet */
}

static void astinvad_hisave(void)
{
    void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


    if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
    {
		osd_fwrite(f,&RAM[0x1fc9],19);
        osd_fclose(f);
    }
}

/* LT 20-3-1998 */
struct GameDriver driver_astinvad =
{
	__FILE__,
	0,
	"astinvad",
	"Astro Invader",
	"1980",
	"Stern",
	"Lee Taylor",
	0,
	&astinvad_machine_driver,
	0,

	rom_astinvad,
	0, 0,
	0,
	0,

	input_ports_astinvad,

	0, 0, 0,
	ORIENTATION_ROTATE_270,

	astinvad_hiload,astinvad_hisave
};

/* LT 20 - 3 19978 */
struct GameDriver driver_kamikaze =
{
	__FILE__,
	&driver_astinvad,
	"kamikaze",
	"Kamikaze",
	"1979",
	"Leijac Corporation",
	"Lee Taylor",
	0,
	&astinvad_machine_driver,
	0,

	rom_kamikaze,
	0, 0,
	0,
	0,

	input_ports_astinvad,

	0, 0, 0,
	ORIENTATION_ROTATE_270,

	0,0
};

/*------------------------------------------------------------------------------
 Shoei Space Intruder
 Added By Lee Taylor (lee@defender.demon.co.uk)
 December 1998
------------------------------------------------------------------------------*/


static int spaceint_interrupt(void)
{
	if (readinputport(3) & 1)	/* Coin */
		return nmi_interrupt();
	else return interrupt();
}


static struct MemoryReadAddress spaceint_readmem[] =
{
	{ 0x0000, 0x17ff, MRA_ROM },
	{ 0x2000, 0x23ff, MRA_RAM },
	{ 0x4000, 0x5fff, MRA_RAM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress spaceint_writemem[] =
{
	{ 0x0000, 0x17ff, MWA_ROM },
	{ 0x2000, 0x23ff, MWA_RAM },
	{ 0x4000, 0x40ff, MWA_RAM },
	{ 0x4100, 0x5fff, spaceint_videoram_w, &videoram, &videoram_size },
	{ -1 }  /* end of table */
};


static struct IOReadPort spaceint_readport[] =
{
	{ 0x00, 0x00, input_port_1_r },
	{ 0x01, 0x01, input_port_2_r },
	{ -1 }  /* end of table */
};

static struct IOWritePort spaceint_writeport[] =
{
	{ -1 }  /* end of table */
};


INPUT_PORTS_START( spaceint )
	PORT_START	/* FAKE - select cabinet type */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2  )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_2WAY)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_2WAY  )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_2WAY | IPF_PLAYER2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_2WAY | IPF_PLAYER2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_PLAYER2)

	PORT_START      /* IN1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
  //PORT_DIPSETTING(    0x06, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START	/* FAKE */
	/* The coin slots are not memory mapped. Coin insertion causes a NMI. */
	/* This fake input port is used by the interrupt */
	/* handler to be notified of coin insertions. We use IMPULSE to */
	/* trigger exactly one interrupt, without having to check when the */
	/* user releases the key. */
	PORT_BIT_IMPULSE( 0x01, IP_ACTIVE_HIGH, IPT_COIN1, 1 )
INPUT_PORTS_END


static struct MachineDriver spaceint_machine_driver = /* 20-12-1998 LT */
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			2000000,        /* 2 Mhz? */
			0,
			spaceint_readmem,spaceint_writemem,spaceint_readport,spaceint_writeport,
			spaceint_interrupt,1
		}
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,       /* frames per second, vblank duration */
	1,      /* single CPU, no need for interleaving */
	0,

	/* video hardware */
	//32*8, 32*8, { 0*8, 32*8-1, 0*8, 32*8-1 },
	32*8, 32*8, { 0*8, 32*8-1, 0*8, 32*8-1 },
	0,      /* no gfxdecodeinfo - bitmapped display */
	8, 0,
	z80bw_init_palette,

	VIDEO_TYPE_RASTER | VIDEO_SUPPORTS_DIRTY | VIDEO_MODIFIES_PALETTE,
	0,
	0,
	0,
	spaceint_vh_screenrefresh,

	/* sound hardware */
	0, 0, 0, 0,
	{
		{
			SOUND_SAMPLES,
			&astinvad_samples_interface
		}
	}
};


ROM_START( spaceint )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "1",	 0x0000, 0x0400, 0x184314d2 )
	ROM_LOAD( "2",	 0x0400, 0x0400, 0x55459aa1 )
	ROM_LOAD( "3",	 0x0800, 0x0400, 0x9d6819be )
	ROM_LOAD( "4",	 0x0c00, 0x0400, 0x432052d4 )
	ROM_LOAD( "5",	 0x1000, 0x0400, 0xc6cfa650 )
	ROM_LOAD( "6",	 0x1400, 0x0400, 0xc7ccf40f )

	ROM_REGIONX(0x0100, REGION_PROMS)
	ROM_LOAD( "clr", 0x0000, 0x0100, 0x13c1803f )
ROM_END


/* LT 21-12-1998 */
struct GameDriver driver_spaceint =
{
	__FILE__,
	0,
	"spaceint",
	"Space Intruder",
	"1980",
	"Shoei",
	"Lee Taylor",
	0,
	&spaceint_machine_driver,
	0,

	rom_spaceint,
	0, 0,
	0,
	0,

	input_ports_spaceint,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_WRONG_COLORS | GAME_NO_SOUND,

	0, 0
};
