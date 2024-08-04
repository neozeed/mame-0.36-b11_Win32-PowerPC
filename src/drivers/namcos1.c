/***************************************************************************

Namco System 1

Preliminary driver by:
Ernesto Corvi
ernesto@imagina.com


Namco System 1 Banking explained:

This system uses a custom IC to allow accessing an address space of
4 MB, whereas the CPU it uses can only address a maximum of
64 KB. The way it does it is writing to the addresses $Ex00, where
x is the destination bank, and the data written is a17 to a21.
It then can read an 8k bank at $x000.

Examples:
When a write to $E000 happens, then the proper chip get selected
and the cpu can now access it from $0000 to $1fff.
When a write to $E200 happens, then the proper chip get selected
and the cpu can now access it from $2000 to $3fff.
Etc.

Emulating this is not trivial.

issue:
  not work                   : tankfrce
  not playable               : bakutotu
                               berabohm(addittional key PCB?)
  shadow/highlight sprite    : mmaze, pacmania , soukobdx , dangseed , etc
  playfield offset           : dspirit (1dot shift when flipscreen)
  sprite (hide) position     : rompers opening
  display error (cause is cpu communication ?)
    : dangseed credit and score is not changed.
    : ws90 time count of select is wrong , somwtime no display 'score'
    : gaalga88 flip sprite in opening of 2player case 'TYPE A'


-----------------------------------------------------
NAMCO SYSTEM BOARD GAME HISTORY?(TABLE SIZE GAME FRAME)
-----------------------------------------------------

(NAMCO SYSTEM I)

87.4  [YOU-KAI-DOU-CHUU-KI]                      "YD"    NONE
87.6  DragonSpirit(OLD VERSION)                  "DS"    136
87.7  Blazer                                     "BZ"    144
87.?? DragonSpirit(NEW VERSION)                  "DS2"   136
87.9  Quester                                    "QS"    'on sub board(A)'
87.?? Quester(SPECIAL EDITION)                   "QS2"   'on sub board(A)'
87.11 PAC-MANIA                                  "PN"    151
??.?? PAC-MANIA(?)(it's rare in japan)           "PN2"   (151?)
87.12 Galaga'88                                  "G8"    153
88.3  '88 PRO-[YAKYUU] WORLD-STADIUM.            (WS?)   (154?)
88.5  [CHOU-ZETU-RINN-JINN] [BERABOW]-MAN.       "BM"    'on sub board(B)'
88.7  MELHEN MAZE(Alice in Wonderland)           "MM"    152
88.8  [BAKU-TOTU-KIJYUU-TEI]                     "BK"    155
88.10 pro-tennis WORLD COAT                      "WC"    143
88.11 splatter house                             "SH"    181
88.12 FAIS OFF(spell-mistake?)                   "FO"    'on sub board(C)'
89.2  ROMPERS                                    "RP"    182
89.3  BLAST-OFF                                  "BO"    183
89.1  PRO-[YAKYUU] WORLD-STADIUM'89.             "W9"    184
89.12 DANGERUS SEED                              "DR"    308
90.7  PRO-[YAKYUU] WORLD-STADIUM'90.             "W90"   310
90.10 [PISUTORU-DAIMYOU NO BOUKEN]               "PD"    309
90.11 [SOUKO-BAN] DX                             "SB"    311
91.12 TANK-FOURCE                                "TF"    185

-----------------------------------------------------

About system I sub board:
sub board used game always need sub board.
if sub bord nothing then not starting game.
because, key custum chip on sub board.
(diffalent type key custum chip on mother board.)

-----------------------------------------------------

'sub board(A)'= for sencer interface bord.
it having changed jp that used format taito/namco.

-----------------------------------------------------

'sub board(B)'= two toggle switch bord.
it sub board the inter face that 'two toggle switch'.
( == in japan called 'BERABOW-SWITCH'.)

<push switch side view>

  =~~~~~=       =~~~~~=
   |   |         |   |
   +---+         +---+
    | |           |||

NORMAL-SW     BERABOW-SW
(two-pins)    (tree-pins)
(GND?,sw1)     (GND?,sw1,sw2)

It abnormal switch was can feel player pushed power.
(It power in proportion to the push speed.)
It used the game controls, 1 stick & 2 it botton game.
Passaged over-time from sw1 triggerd to sw2 triggerd then
min panch/kick,and short time then max panch/kick feeled the game.

-----------------------------------------------------

'sub board(C)'= can 4 players sub board.
it subboard on 3 player & 4 player input lines.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6800/m6800.h"

/* from vidhrdw */
extern void namcos1_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);
extern void namcos1_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
extern int namcos1_vh_start( void );
extern void namcos1_vh_stop( void );

/* from machine */
extern void namcos1_bankswitch_w( int offset, int data );
extern void namcos1_subcpu_bank(int offset,int data);

extern int namcos1_0_banked_area0_r(int offset);
extern int namcos1_0_banked_area1_r(int offset);
extern int namcos1_0_banked_area2_r(int offset);
extern int namcos1_0_banked_area3_r(int offset);
extern int namcos1_0_banked_area4_r(int offset);
extern int namcos1_0_banked_area5_r(int offset);
extern int namcos1_0_banked_area6_r(int offset);
extern int namcos1_0_banked_area7_r(int offset);
extern int namcos1_1_banked_area0_r(int offset);
extern int namcos1_1_banked_area1_r(int offset);
extern int namcos1_1_banked_area2_r(int offset);
extern int namcos1_1_banked_area3_r(int offset);
extern int namcos1_1_banked_area4_r(int offset);
extern int namcos1_1_banked_area5_r(int offset);
extern int namcos1_1_banked_area6_r(int offset);
extern int namcos1_1_banked_area7_r(int offset);
extern void namcos1_0_banked_area0_w( int offset, int data );
extern void namcos1_0_banked_area1_w( int offset, int data );
extern void namcos1_0_banked_area2_w( int offset, int data );
extern void namcos1_0_banked_area3_w( int offset, int data );
extern void namcos1_0_banked_area4_w( int offset, int data );
extern void namcos1_0_banked_area5_w( int offset, int data );
extern void namcos1_0_banked_area6_w( int offset, int data );
extern void namcos1_1_banked_area0_w( int offset, int data );
extern void namcos1_1_banked_area1_w( int offset, int data );
extern void namcos1_1_banked_area2_w( int offset, int data );
extern void namcos1_1_banked_area3_w( int offset, int data );
extern void namcos1_1_banked_area4_w( int offset, int data );
extern void namcos1_1_banked_area5_w( int offset, int data );
extern void namcos1_1_banked_area6_w( int offset, int data );
extern void namcos1_cpu_control_w( int offset, int data );
extern void namcos1_sound_bankswitch_w( int offset, int data );

extern void namcos1_mcu_bankswitch_w(int offset,int data);
extern void namcos1_mcu_patch_w(int offset,int data);

extern void namcos1_machine_init( void );

/************************************************************************************/

static struct MemoryReadAddress main_readmem[] =
{
	{ 0x0000, 0x1fff, namcos1_0_banked_area0_r },
	{ 0x2000, 0x3fff, namcos1_0_banked_area1_r },
	{ 0x4000, 0x5fff, namcos1_0_banked_area2_r },
	{ 0x6000, 0x7fff, namcos1_0_banked_area3_r },
	{ 0x8000, 0x9fff, namcos1_0_banked_area4_r },
	{ 0xa000, 0xbfff, namcos1_0_banked_area5_r },
	{ 0xc000, 0xdfff, namcos1_0_banked_area6_r },
	{ 0xe000, 0xffff, namcos1_0_banked_area7_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress main_writemem[] =
{
	{ 0x0000, 0x1fff, namcos1_0_banked_area0_w },
	{ 0x2000, 0x3fff, namcos1_0_banked_area1_w },
	{ 0x4000, 0x5fff, namcos1_0_banked_area2_w },
	{ 0x6000, 0x7fff, namcos1_0_banked_area3_w },
	{ 0x8000, 0x9fff, namcos1_0_banked_area4_w },
	{ 0xa000, 0xbfff, namcos1_0_banked_area5_w },
	{ 0xc000, 0xdfff, namcos1_0_banked_area6_w },
	{ 0xe000, 0xefff, namcos1_bankswitch_w },
	{ 0xf000, 0xf000, namcos1_cpu_control_w },
	{ 0xf200, 0xf200, MWA_NOP }, /* watchdog? */
//	{ 0xf400, 0xf400, MWA_NOP }, /* unknown */
//	{ 0xf600, 0xf600, MWA_NOP }, /* unknown */
	{ 0xfc00, 0xfc01, namcos1_subcpu_bank },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress sub_readmem[] =
{
	{ 0x0000, 0x1fff, namcos1_1_banked_area0_r },
	{ 0x2000, 0x3fff, namcos1_1_banked_area1_r },
	{ 0x4000, 0x5fff, namcos1_1_banked_area2_r },
	{ 0x6000, 0x7fff, namcos1_1_banked_area3_r },
	{ 0x8000, 0x9fff, namcos1_1_banked_area4_r },
	{ 0xa000, 0xbfff, namcos1_1_banked_area5_r },
	{ 0xc000, 0xdfff, namcos1_1_banked_area6_r },
	{ 0xe000, 0xffff, namcos1_1_banked_area7_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sub_writemem[] =
{
	{ 0xe000, 0xefff, namcos1_bankswitch_w },
	{ 0x0000, 0x1fff, namcos1_1_banked_area0_w },
	{ 0x2000, 0x3fff, namcos1_1_banked_area1_w },
	{ 0x4000, 0x5fff, namcos1_1_banked_area2_w },
	{ 0x6000, 0x7fff, namcos1_1_banked_area3_w },
	{ 0x8000, 0x9fff, namcos1_1_banked_area4_w },
	{ 0xa000, 0xbfff, namcos1_1_banked_area5_w },
	{ 0xc000, 0xdfff, namcos1_1_banked_area6_w },
	{ 0xf000, 0xf000, MWA_NOP }, /* IO Chip */
	{ 0xf200, 0xf200, MWA_NOP }, /* watchdog? */
//	{ 0xf400, 0xf400, MWA_NOP }, /* unknown */
//	{ 0xf600, 0xf600, MWA_NOP }, /* unknown */
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x3fff, MRA_BANK1 },	/* Banked ROMs */
	{ 0x4000, 0x4001, YM2151_status_port_0_r },
	{ 0x5000, 0x50ff, namcos1_wavedata_r },  /* PSG ( Shared ) */
	{ 0x5100, 0x513f, namcos1_sound_r }, /* PSG ( Shared ) */
	{ 0x5140, 0x54ff, MRA_RAM },	/* Sound RAM 1 - ( Shared ) */
	{ 0x7000, 0x77ff, MRA_BANK2 },	/* Sound RAM 2 - ( Shared ) */
	{ 0x8000, 0x9fff, MRA_RAM },	/* Sound RAM 3 */
	{ 0xc000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x3fff, MWA_ROM },	/* Banked ROMs */
	{ 0x4000, 0x4000, YM2151_register_port_0_w },
	{ 0x4001, 0x4001, YM2151_data_port_0_w },
	{ 0x5000, 0x50ff, namcos1_wavedata_w,&namco_wavedata },  /* PSG ( Shared ) */
	{ 0x5100, 0x513f, namcos1_sound_w,&namco_soundregs }, /* PSG ( Shared ) */
	{ 0x5140, 0x54ff, MWA_RAM },	/* Sound RAM 1 - ( Shared ) */
	{ 0x7000, 0x77ff, MWA_BANK2 },	/* Sound RAM 2 - ( Shared ) */
	{ 0x8000, 0x9fff, MWA_RAM },	/* Sound RAM 3 */
	{ 0xc000, 0xc001, namcos1_sound_bankswitch_w }, /* bank selector */
	{ 0xd001, 0xd001, MWA_NOP },	/* watchdog? */
	{ 0xc000, 0xc000, MWA_NOP}, /* unknown write 10h */
	{ 0xe000, 0xe000, MWA_NOP}, /* IRQ clar ? */
	{ -1 }  /* end of table */
};

static int dsw_r( int offs ) {
	int ret = readinputport( 2 );

	if ( offs == 0 )
		ret >>= 4;

	return 0xf0 | ( ret & 0x0f );
}

static void dac0_w(int offset,int data)
{
	DAC_signed_data_w(0,data);
}
static void dac1_w(int offset,int data)
{
	DAC_signed_data_w(1,data);
}

static struct MemoryReadAddress mcu_readmem[] =
{
	{ 0x0000, 0x001f, hd63701_internal_registers_r },
	{ 0x0080, 0x00ff, MRA_RAM }, /* built in RAM */
	{ 0x1400, 0x1400, input_port_0_r },
	{ 0x1401, 0x1401, input_port_1_r },
	{ 0x1000, 0x1002, dsw_r },
	{ 0x4000, 0xbfff, MRA_BANK4 }, /* banked ROM */
	{ 0xc000, 0xc7ff, MRA_BANK3 },
	{ 0xc800, 0xcfff, MRA_RAM }, /* EEPROM */
	{ 0xf000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress mcu_writemem[] =
{
	{ 0x0000, 0x001f, hd63701_internal_registers_w },
	{ 0x0080, 0x00ff, MWA_RAM }, /* built in RAM */
	{ 0x4000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xc000, namcos1_mcu_patch_w },
	{ 0xc000, 0xc7ff, MWA_BANK3 },
	{ 0xc800, 0xcfff, MWA_RAM }, /* EEPROM */
	{ 0xd000, 0xd000, dac0_w }, /* DAC0 */
	{ 0xd400, 0xd400, dac1_w }, /* DAC1 */
	{ 0xd800, 0xd800, namcos1_mcu_bankswitch_w }, /* BANK selector */
	{ 0xf000, 0xf000, MWA_NOP }, /* IRQ clear ? */
	{ -1 }  /* end of table */
};

static struct IOWritePort mcu_writeport[] =
{
//	{ HD63701_PORT2, HD63701_PORT2, mwh_error },
// internal register 4 : port ?
// internal register 5 : port ?
	{ -1 }	/* end of table */
};

static struct IOReadPort mcu_readport[] =
{
	{ HD63701_PORT1, HD63701_PORT1, input_port_3_r },
	{ -1 }	/* end of table */
};

static int namcos1_eeprom_load(void)
{
	void *f;

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
	{
		osd_fread(f,&Machine->memory_region[6][0xc800],0x800);
		osd_fclose(f);
	}
	else
	{
		usrintf_showmessage("F2 toggles test mode");
		memset(&Machine->memory_region[6][0xc800],0xff,0x800);
	}

	return 1;
}

static void namcos1_eeprom_save(void)
{
	void *f;

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&Machine->memory_region[6][0xc800],0x800);
		osd_fclose(f);
	}
}

INPUT_PORTS_START( namcos1 )
	PORT_START /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START	/* DSW1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x00, "Auto Data Sampling" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_SERVICE, "Service Button", KEYCODE_F1, IP_JOY_NONE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
    8,8,    /* 8*8 characters */
    16384,   /* 32*256 characters */
    1,      /* bits per pixel */
    { 0 },     /* bitplanes offset */
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8     /* every char takes 8 consecutive bytes */
};

static struct GfxLayout tilelayout =
{
    8,8,    /* 8*8 characters */
	16384,   /* 16384 characters max */
    8,      /* 8 bits per pixel */
    { 0, 1, 2, 3, 4, 5, 6, 7 },     /* bitplanes offset */
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    { 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
    64*8     /* every char takes 64 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
	32,32,  /* 32*32 sprites */
	8192/4,	/* 2048 sprites at 32x32 */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 },        /* the bitplanes are packed */
	{  0*4,  1*4,  2*4,  3*4,  4*4,  5*4,  6*4,  7*4,
	   8*4,  9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4,
	 256*4,257*4,258*4,259*4,260*4,261*4,262*4,263*4,
	 264*4,265*4,266*4,267*4,268*4,269*4,270*4,271*4},
	{ 0*4*16, 1*4*16,  2*4*16,  3*4*16,  4*4*16,  5*4*16,  6*4*16,  7*4*16,
	  8*4*16, 9*4*16, 10*4*16, 11*4*16, 12*4*16, 13*4*16, 14*4*16, 15*4*16,
	 32*4*16,33*4*16, 34*4*16, 35*4*16, 36*4*16, 37*4*16, 38*4*16, 39*4*16,
	 40*4*16,41*4*16, 42*4*16, 43*4*16, 44*4*16, 45*4*16, 46*4*16, 47*4*16 },
	32*4*8*4   /* every sprite takes 128 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x00000, 	&charlayout,        0,   1 }, /* character's mask */
	{ 1, 0x20000, 	&tilelayout,   128*16,   6 }, /* characters */
	{ 1, 0x120000,	&spritelayout,      0, 128 }, /* sprites 32/16/8/4 dots */
	{ -1 } /* end of array */
};

static void namcos1_sound_interrupt( int irq )
{
	cpu_set_irq_line( 2, M6809_FIRQ_LINE , irq ? ASSERT_LINE : CLEAR_LINE);
}

static struct YM2151interface ym2151_interface =
{
	1,			/* 1 chip */
	3579580,	/* 3.58 MHZ */
	{ YM3012_VOL(50,MIXER_PAN_LEFT,50,MIXER_PAN_RIGHT) },
	{ namcos1_sound_interrupt },
	{ 0 }
};

static struct namco_interface namco_interface =
{
	23920/2,/* sample rate (approximate value) */
	8,		/* number of voices */
	50,		/* playback volume */
	-1,		/* memory region */
	1		/* stereo */
};

static struct DACinterface dac_interface =
{
	2,			/* 2 channel ? */
	{ 50,50 }	/* mixing level */
};

static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
		    CPU_M6809,
		    49152000/32,        /* Not sure if divided by 32 or 24 */
		    0,
		    main_readmem,main_writemem,0,0,
		    interrupt,1,
		},
		{
		    CPU_M6809,
		    49152000/32,        /* Not sure if divided by 32 or 24 */
		    2,
		    sub_readmem,sub_writemem,0,0,
		    interrupt,1,
		},
		{
		    CPU_M6809,
			49152000/32,        /* Not sure if divided by 32 or 24 */
		    3,
		    sound_readmem,sound_writemem,0,0,
		    interrupt,1
		},
		{
		    CPU_HD63701,	/* or compatible 6808 with extra instructions */
			49152000/8/4,
		    6,
		    mcu_readmem,mcu_writemem,mcu_readport,mcu_writeport,
		    interrupt,1
		}
	},
	60, DEFAULT_REAL_60HZ_VBLANK_DURATION,  /* frames per second, vblank duration */
	0,/* CPU slice timer is made by machine_init */
	namcos1_machine_init,

	/* video hardware */
	36*8, 28*8, { 0*8, 36*8-1, 0*8, 28*8-1 },
	gfxdecodeinfo,
	128*16+6*256+1, 128*16+6*256+1,
	namcos1_vh_convert_color_prom,

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE | VIDEO_UPDATE_BEFORE_VBLANK,
	0,
	namcos1_vh_start,
	namcos1_vh_stop,
	namcos1_vh_screenrefresh,

	/* sound hardware */
	SOUND_SUPPORTS_STEREO,0,0,0,
	{
		{
			SOUND_YM2151,
			&ym2151_interface
		},
		{
			SOUND_NAMCO,
			&namco_interface
		},
		{
			SOUND_DAC,
			&dac_interface
		}
	}
};


/***************************************************************************

  Game driver(s)

***************************************************************************/
/* load half size ROM to full size space */
#define ROM_LOAD_HS(name,start,length,crc)  \
	ROM_LOAD(name,start,length,crc) \
	ROM_RELOAD(start+length,length)

/* Shadowland */
ROM_START( shadowld )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "yd.ch8",			0x00000, 0x20000, 0x0c8e69d0 )	/* character mask */
	ROM_LOAD( "yd.ch0",			0x20000, 0x20000, 0x717441dd )	/* characters */
	ROM_LOAD( "yd.ch1",			0x40000, 0x20000, 0xc1be6e35 )	/* characters */
	ROM_LOAD( "yd.ch2",			0x60000, 0x20000, 0x2df8d8cc )	/* characters */
	ROM_LOAD( "yd.ch3",			0x80000, 0x20000, 0xd4e15c9e )	/* characters */
	ROM_LOAD( "yd.ch4",			0xa0000, 0x20000, 0xc0041e0d )	/* characters */
	ROM_LOAD( "yd.ch5",			0xc0000, 0x20000, 0x7b368461 )	/* characters */
	ROM_LOAD( "yd.ch6",			0xe0000, 0x20000, 0x3ac6a90e )	/* characters */
	ROM_LOAD( "yd.ch7",			0x100000, 0x20000, 0x8d2cffa5 )	/* characters */

	ROM_LOAD( "yd.ob0",			0x120000, 0x20000, 0xefb8efe3 )	/* sprites */
	ROM_LOAD( "yd.ob1",			0x140000, 0x20000, 0xbf4ee682 )	/* sprites */
	ROM_LOAD( "yd.ob2",			0x160000, 0x20000, 0xcb721682 )	/* sprites */
	ROM_LOAD( "yd.ob3",			0x180000, 0x20000, 0x8a6c3d1c )	/* sprites */
	ROM_LOAD( "yd.ob4",			0x1a0000, 0x20000, 0xef97bffb )	/* sprites */
	ROM_LOAD( "yd3.ob5",		0x1c0000, 0x10000, 0x1e4aa460 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "yd1.sd0",		0x0c000, 0x10000, 0xa9cb51fb )
	ROM_LOAD( "yd1.sd1",		0x1c000, 0x10000, 0x65d1dc0d )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "yd3.p7",		0x000000, 0x10000, 0xf1c271a0 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd3.p6",		0x020000, 0x10000, 0x93d6811c )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p5",		0x040000, 0x10000, 0x29a78bd6 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p3",		0x080000, 0x10000, 0xa4f27c24 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p2",		0x0a0000, 0x10000, 0x62e5bbec )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p1",		0x0c0000, 0x10000, 0xa8ea6bd3 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p0",		0x0e0000, 0x10000, 0x07e49883 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0xd0000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "yd1.v0",		0x10000, 0x10000, 0xcde1ee23 )
	ROM_LOAD_HS( "yd1.v1",		0x30000, 0x10000, 0xc61f462b )
	ROM_LOAD_HS( "yd1.v2",		0x50000, 0x10000, 0x821ad462 )
	ROM_LOAD_HS( "yd1.v3",		0x70000, 0x10000, 0x1e003489 )
	ROM_LOAD_HS( "yd1.v4",		0x90000, 0x10000, 0xa106e6f6 )
	ROM_LOAD_HS( "yd1.v5",		0xb0000, 0x10000, 0xde72f38f )
ROM_END

/* Youkai Douchuuki (Shadowland Japan) */
ROM_START( youkaidk )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "yd.ch8",			0x00000, 0x20000, 0x0c8e69d0 )	/* character mask */
	ROM_LOAD( "yd.ch0",			0x20000, 0x20000, 0x717441dd )	/* characters */
	ROM_LOAD( "yd.ch1",			0x40000, 0x20000, 0xc1be6e35 )	/* characters */
	ROM_LOAD( "yd.ch2",			0x60000, 0x20000, 0x2df8d8cc )	/* characters */
	ROM_LOAD( "yd.ch3",			0x80000, 0x20000, 0xd4e15c9e )	/* characters */
	ROM_LOAD( "yd.ch4",			0xa0000, 0x20000, 0xc0041e0d )	/* characters */
	ROM_LOAD( "yd.ch5",			0xc0000, 0x20000, 0x7b368461 )	/* characters */
	ROM_LOAD( "yd.ch6",			0xe0000, 0x20000, 0x3ac6a90e )	/* characters */
	ROM_LOAD( "yd.ch7",			0x100000, 0x20000, 0x8d2cffa5 )	/* characters */

	ROM_LOAD( "yd.ob0",			0x120000, 0x20000, 0xefb8efe3 )	/* sprites */
	ROM_LOAD( "yd.ob1",			0x140000, 0x20000, 0xbf4ee682 )	/* sprites */
	ROM_LOAD( "yd.ob2",			0x160000, 0x20000, 0xcb721682 )	/* sprites */
	ROM_LOAD( "yd.ob3",			0x180000, 0x20000, 0x8a6c3d1c )	/* sprites */
	ROM_LOAD( "yd.ob4",			0x1a0000, 0x20000, 0xef97bffb )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "yd1.sd0",		0x0c000, 0x10000, 0xa9cb51fb )
	ROM_LOAD( "yd1.sd1",		0x1c000, 0x10000, 0x65d1dc0d )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "yd2prg7b.bin",0x000000, 0x10000, 0xa05bf3ae )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1-prg6.bin",0x020000, 0x10000, 0x785a2772 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p5",		0x040000, 0x10000, 0x29a78bd6 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p3",		0x080000, 0x10000, 0xa4f27c24 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p2",		0x0a0000, 0x10000, 0x62e5bbec )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p1",		0x0c0000, 0x10000, 0xa8ea6bd3 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "yd1.p0",		0x0e0000, 0x10000, 0x07e49883 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0xd0000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "yd1.v0",		0x10000, 0x10000, 0xcde1ee23 )
	ROM_LOAD_HS( "yd1.v1",		0x30000, 0x10000, 0xc61f462b )
	ROM_LOAD_HS( "yd1.v2",		0x50000, 0x10000, 0x821ad462 )
	ROM_LOAD_HS( "yd1.v3",		0x70000, 0x10000, 0x1e003489 )
	ROM_LOAD_HS( "yd1.v4",		0x90000, 0x10000, 0xa106e6f6 )
	ROM_LOAD_HS( "yd1.v5",		0xb0000, 0x10000, 0xde72f38f )
ROM_END

/* Dragon Spirit */
ROM_START( dspirit )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "dschr-8.bin",	0x00000, 0x20000, 0x946eb242 )	/* character mask */
	ROM_LOAD( "dschr-0.bin",	0x20000, 0x20000, 0x7bf28ac3 )	/* characters */
	ROM_LOAD( "dschr-1.bin",	0x40000, 0x20000, 0x03582fea )	/* characters */
	ROM_LOAD( "dschr-2.bin",	0x60000, 0x20000, 0x5e05f4f9 )	/* characters */
	ROM_LOAD( "dschr-3.bin",	0x80000, 0x20000, 0xdc540791 )	/* characters */
	ROM_LOAD( "dschr-4.bin",	0xa0000, 0x20000, 0xffd1f35c )	/* characters */
	ROM_LOAD( "dschr-5.bin",	0xc0000, 0x20000, 0x8472e0a3 )	/* characters */
	ROM_LOAD( "dschr-6.bin",	0xe0000, 0x20000, 0xa799665a )	/* characters */
	ROM_LOAD( "dschr-7.bin",	0x100000, 0x20000, 0xa51724af )	/* characters */

	ROM_LOAD( "dsobj-0.bin",	0x120000, 0x20000, 0x03ec3076 )	/* sprites */
	ROM_LOAD( "dsobj-1.bin",	0x140000, 0x20000, 0xe67a8fa4 )	/* sprites */
	ROM_LOAD( "dsobj-2.bin",	0x160000, 0x20000, 0x061cd763 )	/* sprites */
	ROM_LOAD( "dsobj-3.bin",	0x180000, 0x20000, 0x63225a09 )	/* sprites */
	ROM_LOAD( "dsobj-4.bin",	0x1a0000, 0x10000, 0xa6246fcb )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "dssnd-0.bin",	0x0c000, 0x10000, 0x27100065 )
	ROM_LOAD( "dssnd-1.bin",	0x1c000, 0x10000, 0xb398645f )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "dsprg-7.bin",	0x000000, 0x10000, 0xf4c0d75e )	/* 8 * 8k banks */
	ROM_LOAD_HS( "dsprg-6.bin",	0x020000, 0x10000, 0xa82737b4 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "dsprg-5.bin",	0x040000, 0x10000, 0x9a3a1028 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "dsprg-4.bin",	0x060000, 0x10000, 0xf3307870 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "dsprg-3.bin",	0x080000, 0x10000, 0xc6e5954b )	/* 8 * 8k banks */
	ROM_LOAD_HS( "dsprg-2.bin",	0x0a0000, 0x10000, 0x3c9b0100 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "dsprg-1.bin",	0x0c0000, 0x10000, 0xf7e3298a )	/* 8 * 8k banks */
	ROM_LOAD_HS( "dsprg-0.bin",	0x0e0000, 0x10000, 0xb22a2856 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0xb0000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "dsvoi-0.bin",	0x10000, 0x10000, 0x313b3508 )
	ROM_LOAD( "dsvoi-1.bin",	0x30000, 0x20000, 0x54790d4e )
	ROM_LOAD( "dsvoi-2.bin",	0x50000, 0x20000, 0x05298534 )
	ROM_LOAD( "dsvoi-3.bin",	0x70000, 0x20000, 0x13e84c7e )
	ROM_LOAD( "dsvoi-4.bin",	0x90000, 0x20000, 0x34fbb8cd )
ROM_END

/* Blazer */
ROM_START( blazer )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bz1_chr8.bin",	0x000000, 0x20000, 0xdb28bfca )	/* character mask */
	ROM_LOAD( "bz1_chr0.bin",	0x020000, 0x20000, 0xd346ba61 )	/* characters */
	ROM_LOAD( "bz1_chr1.bin",	0x040000, 0x20000, 0xe45eb2ea )	/* characters */
	ROM_LOAD( "bz1_chr2.bin",	0x060000, 0x20000, 0x599079ee )	/* characters */
	ROM_LOAD( "bz1_chr3.bin",	0x080000, 0x20000, 0xd5182e36 )	/* characters */
	ROM_LOAD( "bz1_chr4.bin",	0x0a0000, 0x20000, 0xe788259e )	/* characters */
	ROM_LOAD( "bz1_chr5.bin",	0x0c0000, 0x20000, 0x107e6814 )	/* characters */
	ROM_LOAD( "bz1_chr6.bin",	0x0e0000, 0x20000, 0x0312e2ba )	/* characters */
	ROM_LOAD( "bz1_chr7.bin",	0x100000, 0x20000, 0xd9d9a90f )	/* characters */

	ROM_LOAD( "bz1_obj0.bin",	0x120000, 0x20000, 0x22aee927 )	/* sprites */
	ROM_LOAD( "bz1_obj1.bin",	0x140000, 0x20000, 0x7cb10112 )	/* sprites */
	ROM_LOAD( "bz1_obj2.bin",	0x160000, 0x20000, 0x34b23bb7 )	/* sprites */
	ROM_LOAD( "bz1_obj3.bin",	0x180000, 0x20000, 0x9bc1db71 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x3c000)     /* 192k for the sound cpu */
	ROM_LOAD( "bz1_snd0.bin",	0x0c000, 0x10000, 0x6c3a580b )
	ROM_LOAD( "bz1_voi4.bin",	0x1c000, 0x20000, 0xd206b1bd )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "bz1_prg7.bin",0x000000, 0x10000, 0x2d4cbb95 )	/* 8 * 8k banks */
	ROM_LOAD( "bz1_prg6.bin",	0x020000, 0x20000, 0x81c48fc0 )	/* 8 * 8k banks */
	ROM_LOAD( "bz1_prg5.bin",	0x040000, 0x20000, 0x900da191 )	/* 8 * 8k banks */
	ROM_LOAD( "bz1_prg4.bin",	0x060000, 0x20000, 0x65ef6f05 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "bz1_prg3.bin",0x080000, 0x10000, 0x81b32a1a )	/* 8 * 8k banks */
	ROM_LOAD_HS( "bz1_prg2.bin",0x0a0000, 0x10000, 0x5d700aed )	/* 8 * 8k banks */
	ROM_LOAD_HS( "bz1_prg1.bin",0x0c0000, 0x10000, 0xc54bbbf4 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "bz1_prg0.bin",0x0e0000, 0x10000, 0xa7dd195b )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0x90000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "bz1_voi0.bin",0x10000, 0x10000, 0x3d09d32e )
	ROM_LOAD( "bz1_voi1.bin",	0x30000, 0x20000, 0x2043b141 )
	ROM_LOAD( "bz1_voi2.bin",	0x50000, 0x20000, 0x64143442 )
	ROM_LOAD( "bz1_voi3.bin",	0x70000, 0x20000, 0x26cfc510 )
ROM_END

/* Pacmania */
ROM_START( pacmania )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pm_chr8.bin",	0x000000, 0x10000, 0xf3afd65d )	/* character mask */
	ROM_LOAD( "pm_chr0.bin",	0x020000, 0x20000, 0x7c57644c )	/* characters */
	ROM_LOAD( "pm_chr1.bin",	0x040000, 0x20000, 0x7eaa67ed )	/* characters */
	ROM_LOAD( "pm_chr2.bin",	0x060000, 0x20000, 0x27e739ac )	/* characters */
	ROM_LOAD( "pm_chr3.bin",	0x080000, 0x20000, 0x1dfda293 )	/* characters */

	ROM_LOAD( "pm_obj0.bin",	0x120000, 0x20000, 0xfda57e8b )	/* sprites */
	ROM_LOAD( "pm_obj1.bin",	0x140000, 0x20000, 0x4c08affe )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "pm_snd0.bin",	0x0c000, 0x10000, 0xc10370fa )
	ROM_LOAD( "pm_snd1.bin",	0x1c000, 0x10000, 0xf761ed5a )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, load it at the bottom 64k */
	ROM_LOAD_HS( "pm_prg7.bin",	0x000000, 0x10000, 0x462fa4fd )	/* 8 * 8k banks */
	ROM_LOAD( "pm_prg6.bin",	0x020000, 0x20000, 0xfe94900c )	/* 16 * 8k banks */

	ROM_REGION( 0x14000 ) /* 64k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0x30000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "pm_voice.bin",0x10000, 0x10000, 0x1ad5788f )
ROM_END

/* Pacmania (Japan) deff o1,s0,s1,p7,v0 */
ROM_START( pacmanij )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pm_chr8.bin",	0x000000, 0x10000, 0xf3afd65d )	/* character mask */
	ROM_LOAD( "pm_chr0.bin",	0x020000, 0x20000, 0x7c57644c )	/* characters */
	ROM_LOAD( "pm_chr1.bin",	0x040000, 0x20000, 0x7eaa67ed )	/* characters */
	ROM_LOAD( "pm_chr2.bin",	0x060000, 0x20000, 0x27e739ac )	/* characters */
	ROM_LOAD( "pm_chr3.bin",	0x080000, 0x20000, 0x1dfda293 )	/* characters */

	ROM_LOAD( "pm_obj0.bin",	0x120000, 0x20000, 0xfda57e8b )	/* sprites */
	ROM_LOAD( "pm-01.b9",	    0x140000, 0x20000, 0x27bdf440 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "pm-s0.a10",		0x0c000, 0x10000, 0xd5ef5eee )
	ROM_LOAD( "pm-s1.b10",		0x1c000, 0x10000, 0x411bc134 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, load it at the bottom 64k */
	ROM_LOAD_HS( "pm-p7.t10",	0x000000, 0x10000, 0x2aa99e2b )	/* 8 * 8k banks */
	ROM_LOAD( "pm_prg6.bin",	0x020000, 0x20000, 0xfe94900c )	/* 16 * 8k banks */

	ROM_REGION( 0x14000 ) /* 64k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0x30000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "pm-v0.b5",	0x10000, 0x10000, 0xe2689f79 )
ROM_END

/* Galaga 88 */
ROM_START( galaga88 )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

    ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "g88_chr8.rom",	0x00000, 0x20000, 0x3862ed0a )	/* character mask */
	ROM_LOAD( "g88_chr0.rom",	0x20000, 0x20000, 0x68559c78 )	/* characters */
	ROM_LOAD( "g88_chr1.rom",	0x40000, 0x20000, 0x3dc0f93f )	/* characters */
	ROM_LOAD( "g88_chr2.rom",	0x60000, 0x20000, 0xdbf26f1f )	/* characters */
	ROM_LOAD( "g88_chr3.rom",	0x80000, 0x20000, 0xf5d6cac5 )	/* characters */

	ROM_LOAD( "g88_obj0.rom",	0x120000, 0x20000, 0xd7112e3f )	/* sprites */
	ROM_LOAD( "g88_obj1.rom",	0x140000, 0x20000, 0x680db8e7 )	/* sprites */
	ROM_LOAD( "g88_obj2.rom",	0x160000, 0x20000, 0x13c97512 )	/* sprites */
	ROM_LOAD( "g88_obj3.rom",	0x180000, 0x20000, 0x3ed3941b )	/* sprites */
	ROM_LOAD( "g88_obj4.rom",	0x1a0000, 0x20000, 0x370ff4ad )	/* sprites */
	ROM_LOAD( "g88_obj5.rom",	0x1c0000, 0x20000, 0xb0645169 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "g88_snd0.rom",	0x0c000, 0x10000, 0x164a3fdc )
	ROM_LOAD( "g88_snd1.rom",	0x1c000, 0x10000, 0x16a4b784 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "g88_prg7.rom",0x000000, 0x10000, 0xdf75b7fc )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88_prg6.rom",0x020000, 0x10000, 0x7e3471d3 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88_prg5.rom",0x040000, 0x10000, 0x4fbd3f6c )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88_prg1.rom",0x0c0000, 0x10000, 0xe68cb351 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88_prg0.rom",0x0e0000, 0x10000, 0x0f0778ca )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0xd0000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "g88_vce0.rom",0x10000, 0x10000, 0x86921dd4 )
	ROM_LOAD_HS( "g88_vce1.rom",0x30000, 0x10000, 0x9c300e16 )
	ROM_LOAD_HS( "g88_vce2.rom",0x50000, 0x10000, 0x5316b4b0 )
	ROM_LOAD_HS( "g88_vce3.rom",0x70000, 0x10000, 0xdc077af4 )
	ROM_LOAD_HS( "g88_vce4.rom",0x90000, 0x10000, 0xac0279a7 )
	ROM_LOAD_HS( "g88_vce5.rom",0xb0000, 0x10000, 0x014ddba1 )
ROM_END

/* Galaga 88 japan */
ROM_START( galag88j )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

    ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "g88_chr8.rom",	0x00000, 0x20000, 0x3862ed0a )	/* character mask */
	ROM_LOAD( "g88_chr0.rom",	0x20000, 0x20000, 0x68559c78 )	/* characters */
	ROM_LOAD( "g88_chr1.rom",	0x40000, 0x20000, 0x3dc0f93f )	/* characters */
	ROM_LOAD( "g88_chr2.rom",	0x60000, 0x20000, 0xdbf26f1f )	/* characters */
	ROM_LOAD( "g88_chr3.rom",	0x80000, 0x20000, 0xf5d6cac5 )	/* characters */

	ROM_LOAD( "g88_obj0.rom",	0x120000, 0x20000, 0xd7112e3f )	/* sprites */
	ROM_LOAD( "g88_obj1.rom",	0x140000, 0x20000, 0x680db8e7 )	/* sprites */
	ROM_LOAD( "g88_obj2.rom",	0x160000, 0x20000, 0x13c97512 )	/* sprites */
	ROM_LOAD( "g88_obj3.rom",	0x180000, 0x20000, 0x3ed3941b )	/* sprites */
	ROM_LOAD( "g88_obj4.rom",	0x1a0000, 0x20000, 0x370ff4ad )	/* sprites */
	ROM_LOAD( "g88_obj5.rom",	0x1c0000, 0x20000, 0xb0645169 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "g88_snd0.rom",	0x0c000, 0x10000, 0x164a3fdc )
	ROM_LOAD( "g88_snd1.rom",	0x1c000, 0x10000, 0x16a4b784 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "g88jprg7.rom",0x000000, 0x10000, 0x7c10965d )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88jprg6.rom",0x020000, 0x10000, 0xe7203707 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88_prg5.rom",0x040000, 0x10000, 0x4fbd3f6c )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88_prg1.rom",0x0c0000, 0x10000, 0xe68cb351 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "g88_prg0.rom",0x0e0000, 0x10000, 0x0f0778ca )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0xd0000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "g88_vce0.rom",0x10000, 0x10000, 0x86921dd4 )
	ROM_LOAD_HS( "g88_vce1.rom",0x30000, 0x10000, 0x9c300e16 )
	ROM_LOAD_HS( "g88_vce2.rom",0x50000, 0x10000, 0x5316b4b0 )
	ROM_LOAD_HS( "g88_vce3.rom",0x70000, 0x10000, 0xdc077af4 )
	ROM_LOAD_HS( "g88_vce4.rom",0x90000, 0x10000, 0xac0279a7 )
	ROM_LOAD_HS( "g88_vce5.rom",0xb0000, 0x10000, 0x014ddba1 )
ROM_END

/* Beraboh Man */
ROM_START( berabohm )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bm-chr8.bin",	0x00000, 0x20000, 0x92860e95 )	/* character mask */
	ROM_LOAD( "bm-chr0.bin",	0x20000, 0x20000, 0xeda1d92e )	/* characters */
	ROM_LOAD( "bm-chr1.bin",	0x40000, 0x20000, 0x8ae1891e )	/* characters */
	ROM_LOAD( "bm-chr2.bin",	0x60000, 0x20000, 0x774cdf4e )	/* characters */
	ROM_LOAD( "bm-chr3.bin",	0x80000, 0x20000, 0x6d81e6c9 )	/* characters */
	ROM_LOAD( "bm-chr4.bin",	0xa0000, 0x20000, 0xf4597683 )	/* characters */
	ROM_LOAD( "bm-chr5.bin",	0xc0000, 0x20000, 0x0e0abde0 )	/* characters */
	ROM_LOAD( "bm-chr6.bin",	0xe0000, 0x20000, 0x4a61f08c )	/* characters */

	ROM_LOAD( "bm-obj0.bin",	0x120000, 0x20000, 0x15724b94 )	/* sprites */
	ROM_LOAD( "bm-obj1.bin",	0x140000, 0x20000, 0x5d21f962 )	/* sprites */
	ROM_LOAD( "bm-obj2.bin",	0x160000, 0x20000, 0x5d48e924 )	/* sprites */
	ROM_LOAD( "bm-obj3.bin",	0x180000, 0x20000, 0xcbe56b7f )	/* sprites */
	ROM_LOAD( "bm-obj4.bin",	0x1a0000, 0x20000, 0x76dcc24c )	/* sprites */
	ROM_LOAD( "bm-obj5.bin",	0x1c0000, 0x20000, 0xfe70201d )	/* sprites */
	ROM_LOAD( "bm-obj7.bin",	0x200000, 0x20000, 0x377c81ed )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "bm1-snd0.bin",	0x0c000, 0x10000, 0xd5d53cb1 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD( "bm1prg7b.bin",	0x010000, 0x10000, 0xe0c36ddd )	/* 8 * 8k banks */
	ROM_CONTINUE(				0x000000, 0x10000 )
	ROM_LOAD_HS( "bm1-prg6.bin",0x020000, 0x10000, 0xa51b69a5 )	/* 8 * 8k banks */
	ROM_LOAD( "bm1-prg4.bin",	0x060000, 0x20000, 0xf6cfcb8c )	/* 8 * 8k banks */
	ROM_LOAD( "bm1-prg1.bin",	0x0c0000, 0x20000, 0xb15f6407 )	/* 8 * 8k banks */
	ROM_LOAD( "bm1-prg0.bin",	0x0e0000, 0x20000, 0xb57ff8c1 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x70000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "bm1-voi0.bin",0x10000, 0x10000, 0x4e40d0ca )
	ROM_LOAD(    "bm1-voi1.bin",0x30000, 0x20000, 0xbe9ce0a8 )
	ROM_LOAD_HS( "bm1-voi2.bin",0x50000, 0x10000, 0x41225d04 )
ROM_END

/* Marchen Maze */
ROM_START( mmaze )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "mm.ch8",			0x00000, 0x20000, 0xa3784dfe )	/* character mask */
	ROM_LOAD( "mm.ch0",			0x20000, 0x20000, 0x43ff2dfc )	/* characters */
	ROM_LOAD( "mm.ch1",			0x40000, 0x20000, 0xb9b4b72d )	/* characters */
	ROM_LOAD( "mm.ch2",			0x60000, 0x20000, 0xbee28425 )	/* characters */
	ROM_LOAD( "mm.ch3",			0x80000, 0x20000, 0xd9f41e5c )	/* characters */
	ROM_LOAD( "mm.ch4",			0xa0000, 0x20000, 0x3484f4ae )	/* characters */
	ROM_LOAD( "mm.ch5",			0xc0000, 0x20000, 0xc863deba )	/* characters */

	ROM_LOAD( "mm.ob0",			0x120000, 0x20000, 0xd4b7e698 )	/* sprites */
	ROM_LOAD( "mm.ob1",			0x140000, 0x20000, 0x1ce49e04 )	/* sprites */
	ROM_LOAD( "mm.ob2",			0x160000, 0x20000, 0x3d3d5de3 )	/* sprites */
	ROM_LOAD( "mm.ob3",			0x180000, 0x20000, 0xdac57358 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "mm.sd0",			0x0c000, 0x10000, 0x25d25e07 )
	ROM_LOAD( "mm1.sd1",		0x1c000, 0x10000, 0x4a3cc89e )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "mm1.p7",		0x000000, 0x10000, 0x085e58cc )	/* 8 * 8k banks */
	ROM_LOAD_HS( "mm1.p6",		0x020000, 0x10000, 0xeaf530d8 )	/* 8 * 8k banks */
	ROM_LOAD( "mm.p2",   		0x0a0000, 0x20000, 0x91bde09f )	/* 8 * 8k banks */
	ROM_LOAD( "mm.p1",   		0x0c0000, 0x20000, 0x6ba14e41 )	/* 8 * 8k banks */
	ROM_LOAD( "mm.p0",   		0x0e0000, 0x20000, 0xe169a911 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0x80000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD( "mm.v0",			0x10000, 0x20000, 0xee974cff )
	ROM_LOAD( "mm.v1",			0x30000, 0x20000, 0xd09b5830 )
ROM_END

/* Bakutotsu Kijuutei */
ROM_START( bakutotu )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bk-chr8.bin",	0x00000, 0x20000, 0x6c8d4029 )	/* character mask */
	ROM_LOAD( "bk-chr0.bin",	0x20000, 0x20000, 0x4e011058 )	/* characters */
	ROM_LOAD( "bk-chr1.bin",	0x40000, 0x20000, 0x496fcb9b )	/* characters */
	ROM_LOAD( "bk-chr2.bin",	0x60000, 0x20000, 0xdc812e28 )	/* characters */
	ROM_LOAD( "bk-chr3.bin",	0x80000, 0x20000, 0x2b6120f4 )	/* characters */

	ROM_LOAD( "bk-obj0.bin",	0x120000, 0x20000, 0x88c627c1 )	/* sprites */
	ROM_LOAD( "bk-obj3.bin",	0x180000, 0x20000, 0xf7d1909a )	/* sprites */
	ROM_LOAD( "bk-obj4.bin",	0x1a0000, 0x20000, 0x27ed1441 )	/* sprites */
	ROM_LOAD( "bk-obj5.bin",	0x1c0000, 0x20000, 0x790560c0 )	/* sprites */
	ROM_LOAD( "bk-obj6.bin",	0x1e0000, 0x20000, 0x2cd4d2ea )	/* sprites */
	ROM_LOAD( "bk-obj7.bin",	0x200000, 0x20000, 0x809aa0e6 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "bk1-snd0.bin",	0x0c000, 0x10000, 0xc35d7df6 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD( "bk1-prg7.bin",	0x010000, 0x10000, 0xfac1c1bf )	/* 8 * 8k banks */
	ROM_CONTINUE(				0x000000, 0x10000 )
	ROM_LOAD(    "bk1-prg6.bin",0x020000, 0x20000, 0x57a3ce42 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "bk1-prg5.bin",0x040000, 0x10000, 0xdceed7cb )	/* 8 * 8k banks */
	ROM_LOAD_HS( "bk1-prg4.bin",0x060000, 0x10000, 0x96446d48 )	/* 8 * 8k banks */
	ROM_LOAD(    "bk1-prg3.bin",0x080000, 0x20000, 0xe608234f )	/* 8 * 8k banks */
	ROM_LOAD_HS( "bk1-prg2.bin",0x090000, 0x10000, 0x7a686daa)	/* 8 * 8k banks */
	ROM_LOAD_HS( "bk1-prg1.bin",0x0b0000, 0x10000, 0xd389d6d4 )	/* 8 * 8k banks */
	ROM_LOAD(    "bk1-prg0.bin",0x0e0000, 0x20000, 0x4529c362 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x30000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "bk1-voi0.bin",0x10000, 0x10000, 0x008e290e )
ROM_END

/* World Court */
ROM_START( wldcourt )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "wc1-chr8.bin",	0x00000, 0x20000, 0x23e1c399 )	/* character mask */
	ROM_LOAD( "wc1-chr0.bin",	0x20000, 0x20000, 0x9fb07b9b )	/* characters */
	ROM_LOAD( "wc1-chr1.bin",	0x40000, 0x20000, 0x01bfbf60 )	/* characters */
	ROM_LOAD( "wc1-chr2.bin",	0x60000, 0x20000, 0x7e8acf45 )	/* characters */
	ROM_LOAD( "wc1-chr3.bin",	0x80000, 0x20000, 0x924e9c81 )	/* characters */

	ROM_LOAD( "wc1-obj0.bin",	0x120000, 0x20000, 0x70d562f8 )	/* sprites */
	ROM_LOAD( "wc1-obj1.bin",	0x140000, 0x20000, 0xba8b034a )	/* sprites */
	ROM_LOAD( "wc1-obj2.bin",	0x160000, 0x20000, 0xc2bd5f0f )	/* sprites */
	ROM_LOAD( "wc1-obj3.bin",	0x180000, 0x10000, 0x1aa2dbc8 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "wc1-snd0.bin",	0x0c000, 0x10000, 0x17a6505d )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "wc1-prg7.bin",0x000000, 0x10000, 0x8a7c6cac )	/* 8 * 8k banks */
	ROM_LOAD_HS( "wc1-prg6.bin",0x020000, 0x10000, 0xe9216b9e )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0x50000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "wc1-voi0.bin",0x10000, 0x10000, 0xb57919f7 )
	ROM_LOAD( "wc1-voi1.bin",	0x30000, 0x20000, 0x97974b4b )
ROM_END

/* Splatter House */
ROM_START( splatter )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "chr8",			0x00000, 0x20000, 0x321f483b )	/* character mask */
	ROM_LOAD( "chr0",			0x20000, 0x20000, 0x4dd2ef05 )	/* characters */
	ROM_LOAD( "chr1",			0x40000, 0x20000, 0x7a764999 )	/* characters */
	ROM_LOAD( "chr2",			0x60000, 0x20000, 0x6e6526ee )	/* characters */
	ROM_LOAD( "chr3",			0x80000, 0x20000, 0x8d05abdb )	/* characters */
	ROM_LOAD( "chr4",			0xa0000, 0x20000, 0x1e1f8488 )	/* characters */
	ROM_LOAD( "chr5",			0xc0000, 0x20000, 0x684cf554 )	/* characters */

	ROM_LOAD( "obj0",			0x120000, 0x20000, 0x1cedbbae )	/* sprites */
	ROM_LOAD( "obj1",			0x140000, 0x20000, 0xe56e91ee )	/* sprites */
	ROM_LOAD( "obj2",			0x160000, 0x20000, 0x3dfb0230 )	/* sprites */
	ROM_LOAD( "obj3",			0x180000, 0x20000, 0xe4e5a581 )	/* sprites */
	ROM_LOAD( "obj4",			0x1a0000, 0x20000, 0xb2422182 )	/* sprites */
	ROM_LOAD( "obj5",			0x1c0000, 0x20000, 0x24d0266f )	/* sprites */
	ROM_LOAD( "obj6",			0x1e0000, 0x20000, 0x80830b0e )	/* sprites */
	ROM_LOAD( "obj7",			0x200000, 0x20000, 0x08b1953a )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "sound0",			0x0c000, 0x10000, 0x90abd4ad )
	ROM_LOAD( "sound1",			0x1c000, 0x10000, 0x8ece9e0a )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "prg7",		0x000000, 0x10000, 0x24c8cbd7 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "prg6",		0x020000, 0x10000, 0x97a3e664 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "prg5",		0x040000, 0x10000, 0x0187de9a )	/* 8 * 8k banks */
	ROM_LOAD_HS( "prg4",		0x060000, 0x10000, 0x350dee5b )	/* 8 * 8k banks */
	ROM_LOAD_HS( "prg3",		0x080000, 0x10000, 0x955ce93f )	/* 8 * 8k banks */
	ROM_LOAD_HS( "prg2",		0x0a0000, 0x10000, 0x434dbe7d )	/* 8 * 8k banks */
	ROM_LOAD_HS( "prg1",		0x0c0000, 0x10000, 0x7a3efe09 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "prg0",		0x0e0000, 0x10000, 0x4e07e6d9 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0x90000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD( "voice0",   		0x20000, 0x20000, 0x2199cb66 )
	ROM_LOAD( "voice1",   		0x30000, 0x20000, 0x9b6472af )
	ROM_LOAD( "voice2",			0x50000, 0x20000, 0x25ea75b6 )
	ROM_LOAD( "voice3",			0x70000, 0x20000, 0x5eebcdb4 )
ROM_END

/* Rompers */
ROM_START( rompers )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "rp1-chr8.bin",	0x00000, 0x10000, 0x69cfe46a )	/* character mask */
	ROM_LOAD( "rp1-chr0.bin",	0x20000, 0x20000, 0x41b10ef3 )	/* characters */
	ROM_LOAD( "rp1-chr1.bin",	0x40000, 0x20000, 0xc18cd24e )	/* characters */
	ROM_LOAD( "rp1-chr2.bin",	0x60000, 0x20000, 0x6c9a3c79 )	/* characters */
	ROM_LOAD( "rp1-chr3.bin",	0x80000, 0x20000, 0x473aa788 )	/* characters */

	ROM_LOAD( "rp1-obj0.bin",	0x120000, 0x20000, 0x1dcbf8bb )	/* sprites */
	ROM_LOAD( "rp1-obj1.bin",	0x140000, 0x20000, 0xcb98e273 )	/* sprites */
	ROM_LOAD( "rp1-obj2.bin",	0x160000, 0x20000, 0x6ebd191e )	/* sprites */
	ROM_LOAD( "rp1-obj3.bin",	0x180000, 0x20000, 0x7c9828a1 )	/* sprites */
	ROM_LOAD( "rp1-obj4.bin",	0x1a0000, 0x20000, 0x0348220b )	/* sprites */
	ROM_LOAD( "rp1-obj5.bin",	0x1c0000, 0x10000, 0x9e2ba243 )	/* sprites */
	ROM_LOAD( "rp1-obj6.bin",	0x1e0000, 0x10000, 0x6bf2aca6 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "rp1-snd0.bin",	0x0c000, 0x10000, 0xc7c8d649 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "rp1-prg7.bin",0x000000, 0x10000, 0x49d057e2 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "rp1-prg6.bin",0x020000, 0x10000, 0x80821065 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "rp1-prg5.bin",0x040000, 0x10000, 0x98bd4133)	/* 8 * 8k banks */
	ROM_LOAD_HS( "rp1-prg4.bin",0x060000, 0x10000, 0x0918f06d )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x30000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD( "rp1-voi0.bin",	0x10000, 0x20000, 0x11caef7e )
ROM_END

/* Blast off */
ROM_START( blastoff )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bo1-chr8.bin",	0x00000, 0x20000, 0xe8b5f2d4 )	/* character mask */
	ROM_LOAD( "bo1-chr0.bin",	0x20000, 0x20000, 0xbdc0afb5 )	/* characters */
	ROM_LOAD( "bo1-chr1.bin",	0x40000, 0x20000, 0x963d2639 )	/* characters */
	ROM_LOAD( "bo1-chr2.bin",	0x60000, 0x20000, 0xacdb6894 )	/* characters */
	ROM_LOAD( "bo1-chr3.bin",	0x80000, 0x20000, 0x214ec47f )	/* characters */
	ROM_LOAD( "bo1-chr4.bin",	0xa0000, 0x20000, 0x08397583 )	/* characters */
	ROM_LOAD( "bo1-chr5.bin",	0xc0000, 0x20000, 0x20402429 )	/* characters */
	ROM_LOAD( "bo1-chr7.bin",	0x100000, 0x20000, 0x4c5c4603 )	/* characters */

	ROM_LOAD( "bo1-obj0.bin",	0x120000, 0x20000, 0xb3308ae7 )	/* sprites */
	ROM_LOAD( "bo1-obj1.bin",	0x140000, 0x20000, 0xc9c93c47 )	/* sprites */
	ROM_LOAD( "bo1-obj2.bin",	0x160000, 0x20000, 0xeef77527 )	/* sprites */
	ROM_LOAD( "bo1-obj3.bin",	0x180000, 0x20000, 0xe3d9ed58 )	/* sprites */
	ROM_LOAD( "bo1-obj4.bin",	0x1a0000, 0x20000, 0xc2c1b9cb )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "bo1-snd0.bin",	0x0c000, 0x10000, 0x2ecab76e )
	ROM_LOAD( "bo1-snd1.bin",	0x1c000, 0x10000, 0x048a6af1 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD( "bo1prg7b.bin",	0x010000, 0x10000, 0xb630383c )	/* 8 * 8k banks */
	ROM_CONTINUE(				0x000000, 0x10000 )
	ROM_LOAD( "bo1-prg6.bin",	0x020000, 0x20000, 0xd60da63e )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x70000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD( "bo1-voi0.bin",	0x10000, 0x20000, 0x47065e18 )
	ROM_LOAD( "bo1-voi1.bin",	0x30000, 0x20000, 0x0308b18e )
	ROM_LOAD( "bo1-voi2.bin",	0x50000, 0x20000, 0x88cab230 )
ROM_END

/* Dangerous Sseed */
ROM_START( dangseed )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "dr1-chr8.bin",	0x00000, 0x20000, 0x0fbaa10e )	/* character mask */
	ROM_LOAD( "dr1-chr0.bin",	0x20000, 0x20000, 0x419bacc7 )	/* characters */
	ROM_LOAD( "dr1-chr1.bin",	0x40000, 0x20000, 0x55ce77e1 )	/* characters */
	ROM_LOAD( "dr1-chr2.bin",	0x60000, 0x20000, 0x6f913419 )	/* characters */
	ROM_LOAD( "dr1-chr3.bin",	0x80000, 0x20000, 0xfe1f1a25 )	/* characters */
	ROM_LOAD( "dr1-chr4.bin",	0xa0000, 0x20000, 0xc34471bc )	/* characters */
	ROM_LOAD( "dr1-chr5.bin",	0xc0000, 0x20000, 0x715c0720 )	/* characters */
	ROM_LOAD( "dr1-chr6.bin",	0xe0000, 0x20000, 0x5c1b71fa )	/* characters */

	ROM_LOAD( "dr1-obj0.bin",	0x120000, 0x20000, 0xabb95644 )	/* sprites */
	ROM_LOAD( "dr1-obj1.bin",	0x140000, 0x20000, 0x24d6db51 )	/* sprites */
	ROM_LOAD( "dr1-obj2.bin",	0x160000, 0x20000, 0x7e3a78c0 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "dr1-snd0.bin",	0x0c000, 0x20000, 0xbcbbb21d )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD( "dr1-prg7.bin",	0x010000, 0x10000, 0xd7d2f653 )	/* 8 * 8k banks */
	ROM_CONTINUE(				0x000000, 0x10000 )
	ROM_LOAD_HS( "dr1-prg6.bin",0x020000, 0x10000, 0xcc68262b )	/* 8 * 8k banks */
	ROM_LOAD( "dr1-prg5.bin",	0x040000, 0x20000, 0x7986bbdd )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x30000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD( "dr1-voi0.bin",	0x10000, 0x20000, 0xde4fdc0e )
ROM_END

/* World Stadium 90 */
ROM_START( ws90 )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "wschr-8.bin",	0x00000, 0x20000, 0xd1897b9b )	/* character mask */
	ROM_LOAD( "wschr-0.bin",	0x20000, 0x20000, 0x3e3e96b4 )	/* characters */
	ROM_LOAD( "wschr-1.bin",	0x40000, 0x20000, 0x897dfbc1 )	/* characters */
	ROM_LOAD( "wschr-2.bin",	0x60000, 0x20000, 0xe142527c )	/* characters */
	ROM_LOAD( "wschr-3.bin",	0x80000, 0x20000, 0x907d4dc8 )	/* characters */
	ROM_LOAD( "wschr-4.bin",	0xa0000, 0x20000, 0xafb11e17 )	/* characters */
	ROM_LOAD( "wschr-6.bin",	0xe0000, 0x20000, 0xa16a17c2 )	/* characters */

	ROM_LOAD( "wsobj-0.bin",	0x120000, 0x20000, 0x12dc83a6 )	/* sprites */
	ROM_LOAD( "wsobj-1.bin",	0x140000, 0x20000, 0x68290a46 )	/* sprites */
	ROM_LOAD( "wsobj-2.bin",	0x160000, 0x20000, 0xcd5ba55d )	/* sprites */
	ROM_LOAD( "wsobj-3.bin",	0x180000, 0x10000, 0x7d0b8961 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "wssnd-0.bin",	0x0c000, 0x10000, 0x52b84d5a )
	ROM_LOAD( "wssnd-1.bin",	0x1c000, 0x10000, 0x31bf74c1 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "wsprg-7.bin",	0x000000, 0x10000, 0x37ae1b25 )	/* 8 * 8k banks */
	ROM_LOAD_HS( "wsprg-2.bin",	0x0a0000, 0x10000, 0xb9e98e2f )	/* 8 * 8k banks */
	ROM_LOAD_HS( "wsprg-1.bin",	0x0c0000, 0x10000, 0x7ad8768f )	/* 8 * 8k banks */
	ROM_LOAD_HS( "wsprg-0.bin",	0x0e0000, 0x10000, 0xb0234298 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */
	/* 0x00000 - 0x08000 = RAM6 ( 4 * 8k ) */
	/* 0x08000 - 0x0c000 = RAM1 ( 2 * 8k ) */
	/* 0x0c000 - 0x14000 = RAM3 ( 3 * 8k ) */

	ROM_REGION( 0x50000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "wsvoi-0.bin",	0x10000, 0x10000, 0xf6949199 )
	ROM_LOAD( "wsvoi-1.bin",	0x30000, 0x20000, 0x210e2af9 )
ROM_END

/* Pistol Daimyo no Bouken */
ROM_START( pistoldm )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pd-chr8.bin",	0x00000, 0x20000, 0xa5f516db )	/* character mask */
	ROM_LOAD( "pd-chr0.bin",	0x20000, 0x20000, 0xadbbaf5c )	/* characters */
	ROM_LOAD( "pd-chr1.bin",	0x40000, 0x20000, 0xb4e4f554 )	/* characters */
	ROM_LOAD( "pd-chr2.bin",	0x60000, 0x20000, 0x84592540 )	/* characters */
	ROM_LOAD( "pd-chr3.bin",	0x80000, 0x20000, 0x450bdaa9 )	/* characters */

	ROM_LOAD( "pd-obj0.bin",	0x120000, 0x20000, 0x7269821d )	/* sprites */
	ROM_LOAD( "pd-obj1.bin",	0x140000, 0x20000, 0x4f9738e5 )	/* sprites */
	ROM_LOAD( "pd-obj2.bin",	0x160000, 0x20000, 0x33208776 )	/* sprites */
	ROM_LOAD( "pd-obj3.bin",	0x180000, 0x20000, 0x0dbd54ef )	/* sprites */
	ROM_LOAD( "pd-obj4.bin",	0x1a0000, 0x20000, 0x58e838e2 )	/* sprites */
	ROM_LOAD( "pd-obj5.bin",	0x1c0000, 0x20000, 0x414f9a9d )	/* sprites */
	ROM_LOAD( "pd-obj6.bin",	0x1e0000, 0x20000, 0x91b4e6e0 )	/* sprites */
	ROM_LOAD( "pd-obj7.bin",	0x200000, 0x20000, 0x00d4a8f0 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "pd1-snd0.bin",	0x0c000, 0x20000, 0x026da54e )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD( "pd1prg7b.bin",	0x010000, 0x10000, 0x7189b797 )	/* 8 * 8k banks */
	ROM_CONTINUE(				0x000000, 0x10000 )
	ROM_LOAD( "pd1-prg0.bin",	0x0e0000, 0x20000, 0x9db9b89c )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x50000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD( "pd-voi0.bin",	0x10000, 0x20000, 0xad1b8128 )
	ROM_LOAD( "pd-voi1.bin",	0x30000, 0x20000, 0x2871c494 )
ROM_END

/* Soukoban DX */
ROM_START( soukobdx )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "sb1-chr8.bin",	0x00000, 0x10000, 0x5692b297 )	/* character mask */
	ROM_LOAD( "sb1-chr0.bin",	0x20000, 0x20000, 0x267f1331 )	/* characters */
	ROM_LOAD( "sb1-chr1.bin",	0x40000, 0x20000, 0xe5ff61ad )	/* characters */
	ROM_LOAD( "sb1-chr2.bin",	0x60000, 0x20000, 0x099b746b )	/* characters */
	ROM_LOAD( "sb1-chr3.bin",	0x80000, 0x20000, 0x1551bb7c )	/* characters */

	ROM_LOAD( "sb1-obj0.bin",	0x120000, 0x10000, 0xed810da4 )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "sb1-snd0.bin",	0x0c000, 0x10000, 0xbf46a106 )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD_HS( "sb1-prg7.bin",0x000000, 0x10000, 0xc3bd418a )	/* 8 * 8k banks */
	ROM_LOAD( "sb1-prg1.bin",	0x0c0000, 0x20000, 0x5d1fdd94 )	/* 8 * 8k banks */
	ROM_LOAD( "sb1-prg0.bin",	0x0e0000, 0x20000, 0x8af8cb73 )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x40000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",	0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD_HS( "sb1-voi0.bin",0x10000, 0x10000, 0x63d9cedf )
ROM_END

/* Tank Force */
ROM_START( tankfrce )
	ROM_REGION(0x10000)     /* 64k for the main cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION_DISPOSE(0x220000)     /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "tf1-chr8.bin",	0x00000, 0x20000, 0x7d53b31e )	/* character mask */
	ROM_LOAD( "tf1-chr0.bin",	0x20000, 0x20000, 0x51fedc8c )	/* characters */
	ROM_LOAD( "tf1-chr1.bin",	0x40000, 0x20000, 0x76e1bc56 )	/* characters */
	ROM_LOAD( "tf1-chr2.bin",	0x60000, 0x20000, 0xfcb645d9 )	/* characters */
	ROM_LOAD( "tf1-chr3.bin",	0x80000, 0x20000, 0xa8dbf080 )	/* characters */
	ROM_LOAD( "tf1-chr4.bin",	0xa0000, 0x20000, 0x51fedc8c )	/* characters */
	ROM_LOAD( "tf1-chr5.bin",	0xc0000, 0x20000, 0xe6c6609a )	/* characters */

	ROM_LOAD( "tf1-obj0.bin",	0x120000, 0x20000, 0x4bedd51a )	/* sprites */
	ROM_LOAD( "tf1-obj1.bin",	0x140000, 0x20000, 0xdf674d6d )	/* sprites */

	ROM_REGION(0x10000)     /* 64k for the sub cpu */
	/* Nothing loaded here. Bankswitching makes sure this gets the necessary code */

	ROM_REGION(0x2c000)     /* 192k for the sound cpu */
	ROM_LOAD( "tf1-snd0.bin",	0x0c000, 0x20000, 0x4d9cf7aa )

	ROM_REGION( 0x100000 ) /* 1m for ROMs */
	/* PRGx ROMs go here - they can be 128k max. */
	/* if you got a 64k one, reload it */
	ROM_LOAD( "tf1-prg7.bin",	0x010000, 0x10000, 0x9dfa0dd5 )	/* 8 * 8k banks */
	ROM_CONTINUE(				0x000000, 0x10000 )
	ROM_LOAD( "tf1-prg1.bin",	0x0c0000, 0x20000, 0x4a8bb251 )	/* 8 * 8k banks */
	ROM_LOAD( "tf1-prg0.bin",	0x0e0000, 0x20000, 0x2ae4b9eb )	/* 8 * 8k banks */

	ROM_REGION( 0x14000 ) /* 80k for RAM */

	ROM_REGION( 0x50000 ) /* the MCU & voice */
	ROM_LOAD( "ns1-mcu.bin",    0x0f000, 0x01000, 0xffb5c0bd )	/* mcu 'kernel' code */
	ROM_LOAD( "tf1-voi0.bin",	0x10000, 0x20000, 0xf542676a )
	ROM_LOAD( "tf1-voi1.bin",	0x30000, 0x20000, 0x615d09cd )
ROM_END


#define GAME_DRIVER(NAME,YEAR,REALNAME,MANU,INIT_NAME,ORIENTATION) \
extern void INIT_NAME##_driver_init(void); \
struct GameDriver driver_##NAME = \
{ \
	__FILE__,         \
	0,                \
	#NAME,            \
	REALNAME,         \
	#YEAR,            \
	MANU,             \
	"Ernesto Corvi\nJROK\nTatsuyuki Satoh", \
	0,                \
	&machine_driver,  \
	INIT_NAME##_driver_init, \
	rom_##NAME,       \
	0, 0,             \
	0,                \
	0,                \
	input_ports_namcos1,      \
	0, 0, 0,          \
	ORIENTATION,      \
	namcos1_eeprom_load, \
	namcos1_eeprom_save  \
};

#define CLONE_DRIVER(NAME,CLONE,YEAR,REALNAME,MANU,INIT_NAME,ORIENTATION) \
extern void INIT_NAME##_driver_init(void); \
struct GameDriver driver_##NAME = \
{ \
	__FILE__,         \
	&driver_##CLONE,  \
	#NAME,            \
	REALNAME,         \
	#YEAR,             \
	MANU,             \
	"Ernesto Corvi\nJROK\nTatsuyuki Satoh", \
	0,                \
	&machine_driver,  \
	INIT_NAME##_driver_init, \
	rom_##NAME,       \
	0, 0,             \
	0,                \
	0,                \
	input_ports_namcos1,      \
	0, 0, 0,          \
	ORIENTATION,      \
	namcos1_eeprom_load, \
	namcos1_eeprom_save  \
};

GAME_DRIVER (shadowld,         1987,"Shadow Land",                    "Namco",shadowld,ORIENTATION_DEFAULT | GAME_REQUIRES_16BIT)
CLONE_DRIVER(youkaidk,shadowld,1987,"Yokai Douchuuki (Japan)",        "Namco",shadowld,ORIENTATION_DEFAULT | GAME_REQUIRES_16BIT)
GAME_DRIVER (dspirit,          1987,"Dragon Spirit",                  "Namco",dspirit, ORIENTATION_ROTATE_270)
//GAME_DRIVER (dspirita,       1987,"Dragon Spirit (set 2)",          "Namco",dspirit, ORIENTATION_ROTATE_270)
GAME_DRIVER (blazer,           1987,"Blazer (Japan)",                 "Namco",blazer,  ORIENTATION_ROTATE_270)
//GAME_DRIVER (quester,        1987,"Quester",                        "Namco",quester, ORIENTATION_DEFAULT | GAME_NOT_WORKING)
GAME_DRIVER (pacmania,         1987,"Pac-Mania",                      "Namco",pacmania,ORIENTATION_ROTATE_270 | GAME_REQUIRES_16BIT)
CLONE_DRIVER(pacmanij,pacmania,1987,"Pac-Mania (Japan)",              "Namco",pacmania,ORIENTATION_ROTATE_270 | GAME_REQUIRES_16BIT)
/* galaga88 use shadow sprite , and could fit in 256 colors stage 21 */
GAME_DRIVER (galaga88,         1987,"Galaga '88",                     "Namco",galaga88,ORIENTATION_ROTATE_270 | GAME_REQUIRES_16BIT)
CLONE_DRIVER(galag88j,galaga88,1987,"Galaga '88 (Japan)",             "Namco",galaga88,ORIENTATION_ROTATE_270 | GAME_REQUIRES_16BIT)
//GAME_DRIVER (wstadium,       1988,"World Stadium",                  "Namco",wstadium,ORIENTATION_DEFAULT | GAME_NOT_WORKING)
GAME_DRIVER (berabohm,         1988,"Beraboh Man",                    "Namco",berabohm,ORIENTATION_DEFAULT | GAME_NOT_WORKING)
//GAME_DRIVER (alice,          1988,"Alice In Wonderland",            "Namco",alice,   ORIENTATION_DEFAULT)
GAME_DRIVER (mmaze,            1988,"Marchen Maze (Japan)",           "Namco",alice,   ORIENTATION_DEFAULT | GAME_REQUIRES_16BIT)
GAME_DRIVER (bakutotu,         1988,"Bakutotsu Kijuutei",             "Namco",bakutotu,ORIENTATION_DEFAULT | GAME_NOT_WORKING)
GAME_DRIVER (wldcourt,         1988,"World Court (Japan)",            "Namco",wldcourt,ORIENTATION_DEFAULT)
/* in theory Splatterhouse could fit in 256 colors */
GAME_DRIVER (splatter,         1988,"Splatter House (Japan)",         "Namco",splatter,ORIENTATION_DEFAULT | GAME_REQUIRES_16BIT)
//GAME_DRIVER (faceoff,        1988,"Face Off",                       "Namco",faceoff, ORIENTATION_DEFAULT | GAME_NOT_WORKING)
GAME_DRIVER (rompers,          1989,"Rompers (Japan)",                "Namco",rompers, ORIENTATION_ROTATE_270 | GAME_REQUIRES_16BIT)
GAME_DRIVER (blastoff,         1989,"Blast Off (Japan)",              "Namco",blastoff,ORIENTATION_ROTATE_270)
//GAME_DRIVER (ws89,           1989,"World Stadium 89",               "Namco",ws89,    ORIENTATION_DEFAULT | GAME_NOT_WORKING)
/* dangseed overflows palette in a few places, it might be improveable */
GAME_DRIVER (dangseed,         1989,"Dangerous Seed (Japan)",         "Namco",dangseed,ORIENTATION_ROTATE_270 | GAME_REQUIRES_16BIT)
GAME_DRIVER (ws90,             1990,"World Stadium 90 (Japan)",       "Namco",ws90,    ORIENTATION_DEFAULT)
GAME_DRIVER (pistoldm,         1990,"Pistol Daimyo no Bouken (Japan)","Namco",pistoldm,ORIENTATION_DEFAULT)
GAME_DRIVER (soukobdx,         1990,"Souko Ban Deluxe (Japan)",       "Namco",soukobdx,ORIENTATION_DEFAULT)
GAME_DRIVER (tankfrce,         1991,"Tank Force (Japan)",             "Namco",tankfrce,ORIENTATION_DEFAULT | GAME_NOT_WORKING)
