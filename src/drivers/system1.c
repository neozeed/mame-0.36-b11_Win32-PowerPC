/******************************************************************************

Sega System 1 / System 2

Up'n Down, Mister Viking, Flicky, SWAT, Water Match and Bull Fight are known
to run on IDENTICAL hardware (they were sold by Bally-Midway as ROM swaps).

TODO: background is misplaced in wbmlju

******************************************************************************/

#include "driver.h"
#include "vidhrdw/system1.h"
#include "cpu/z80/z80.h"



/* in machine/segacrpt.c */
void regulus_decode(void);
void mrviking_decode(void);
void swat_decode(void);
void flicky_decode(void);
void bullfgtj_decode(void);
void pitfall2_decode(void);
void nprinces_decode(void);
void seganinj_decode(void);
void imsorry_decode(void);
void teddybb_decode(void);
void hvymetal_decode(void);
void myheroj_decode(void);
void fdwarrio_decode(void);
void wboy3_decode(void);
void wboy4_decode(void);
void gardia_decode(void);



static void system1_init_machine(void)
{
	/* skip the long IC CHECK in Teddyboy Blues and Choplifter */
	/* this is not a ROM patch, the game checks a RAM location */
	/* before doing the test */
	memory_region(Machine->drv->cpu[0].memory_region)[0xeffe] = 0x4f;
	memory_region(Machine->drv->cpu[0].memory_region)[0xefff] = 0x4b;

	system1_define_sprite_pixelmode(system1_SPRITE_PIXEL_MODE1);
	system1_define_background_memory(system1_BACKGROUND_MEMORY_SINGLE);
}

static void chplft_init_machine(void)
{
	/* skip the long IC CHECK in Teddyboy Blues and Choplifter */
	/* this is not a ROM patch, the game checks a RAM location */
	/* before doing the test */
	memory_region(Machine->drv->cpu[0].memory_region)[0xeffe] = 0x4f;
	memory_region(Machine->drv->cpu[0].memory_region)[0xefff] = 0x4b;

	system1_define_sprite_pixelmode(system1_SPRITE_PIXEL_MODE2);
	system1_define_background_memory(system1_BACKGROUND_MEMORY_SINGLE);
}

static void wbml_init_machine(void)
{
	/* skip the long IC CHECK in Teddyboy Blues and Choplifter */
	/* this is not a ROM patch, the game checks a RAM location */
	/* before doing the test */
	memory_region(Machine->drv->cpu[0].memory_region)[0xeffe] = 0x4f;
	memory_region(Machine->drv->cpu[0].memory_region)[0xefff] = 0x4b;

	system1_define_sprite_pixelmode(system1_SPRITE_PIXEL_MODE2);
	system1_define_background_memory(system1_BACKGROUND_MEMORY_BANKED);
}


static int bankswitch;

int wbml_bankswitch_r(int offset)
{
	return bankswitch;
}

void hvymetal_bankswitch_w(int offset,int data)
{
	int bankaddress;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* patch out the obnoxiously long startup RAM tests */
	ROM[0x4a55] = 0xc3;
	RAM[0x4a56] = 0xb6;
	RAM[0x4a57] = 0x4a;

	bankaddress = 0x10000 + (((data & 0x04)>>2) * 0x4000) + (((data & 0x40)>>5) * 0x4000);
	cpu_setbank(1,&RAM[bankaddress]);
	/* TODO: the memory system doesn't yet support bank switching on an encrypted */
	/* ROM, so we have to copy the data manually */
	memcpy(&ROM[0x8000],&RAM[bankaddress],0x4000);

	bankswitch = data;
}

void brain_bankswitch_w(int offset,int data)
{
	int bankaddress;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	bankaddress = 0x10000 + (((data & 0x04)>>2) * 0x4000) + (((data & 0x40)>>5) * 0x4000);
	cpu_setbank(1,&RAM[bankaddress]);
	/* TODO: the memory system doesn't yet support bank switching on an encrypted */
	/* ROM, so we have to copy the data manually */
	memcpy(&ROM[0x8000],&RAM[bankaddress],0x4000);

	bankswitch = data;
}

void chplft_bankswitch_w(int offset,int data)
{
	int bankaddress;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	bankaddress = 0x10000 + (((data & 0x0c)>>2) * 0x4000);
	cpu_setbank(1,&RAM[bankaddress]);

	bankswitch = data;
}

void wbml_bankswitch_w(int offset,int data)
{
	int bankaddress;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	bankaddress = 0x10000 + (((data & 0x0c)>>2) * 0x4000);
	cpu_setbank(1,&RAM[bankaddress]);
	/* TODO: the memory system doesn't yet support bank switching on an encrypted */
	/* ROM, so we have to copy the data manually */
	if (ROM != RAM)
		memcpy(&ROM[0x8000],&RAM[bankaddress+0x20000],0x4000);

	bankswitch = data;
}

void system1_soundport_w(int offset, int data)
{
	soundlatch_w(0,data);
	cpu_cause_interrupt(1,Z80_NMI_INT);
	/* spin for a while to let the Z80 read the command (fixes hanging sound in Regulus) */
	cpu_spinuntil_time(TIME_IN_USEC(50));
}



static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0xbfff, MRA_ROM },
	{ 0xc000, 0xefff, MRA_RAM },
	{ 0xf020, 0xf03f, MRA_RAM },
	{ 0xf800, 0xfbff, MRA_RAM },
	{ -1 } /* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xcfff, MWA_RAMROM },
	{ 0xd000, 0xd1ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xd800, 0xdfff, system1_paletteram_w, &paletteram },
	{ 0xe000, 0xe7ff, system1_backgroundram_w, &system1_backgroundram, &system1_backgroundram_size },
	{ 0xe800, 0xeeff, MWA_RAM, &system1_videoram, &system1_videoram_size },
	{ 0xefbd, 0xefbd, MWA_RAM, &system1_scroll_y },
	{ 0xeffc, 0xeffd, MWA_RAM, &system1_scroll_x },
	{ 0xf000, 0xf3ff, system1_background_collisionram_w, &system1_background_collisionram },
	{ 0xf800, 0xfbff, system1_sprites_collisionram_w, &system1_sprites_collisionram },
	{ -1 } /* end of table */
};

static struct MemoryReadAddress wbml_readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0xbfff, MRA_BANK1 },
	{ 0xc000, 0xdfff, MRA_RAM },
	{ 0xe000, 0xefff, wbml_paged_videoram_r },
	{ 0xf020, 0xf03f, MRA_RAM },
	{ 0xf800, 0xfbff, MRA_RAM },
	{ -1 } /* end of table */
};

static struct MemoryWriteAddress wbml_writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xcfff, MWA_RAMROM },
	{ 0xd000, 0xd1ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xd800, 0xddff, system1_paletteram_w, &paletteram },
	{ 0xe000, 0xefff, wbml_paged_videoram_w },
	{ 0xf000, 0xf3ff, system1_background_collisionram_w, &system1_background_collisionram },
	{ 0xf800, 0xfbff, system1_sprites_collisionram_w, &system1_sprites_collisionram },
	{ -1 } /* end of table */
};

static struct MemoryReadAddress chplft_readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0xbfff, MRA_BANK1 },
	{ 0xc000, 0xefff, MRA_RAM },
	{ 0xf020, 0xf03f, MRA_RAM },
	{ 0xf800, 0xfbff, MRA_RAM },
	{ -1 } /* end of table */
};

static struct MemoryWriteAddress chplft_writemem[] =
{
	{ 0x0000, 0xbfff, MWA_ROM },
	{ 0xc000, 0xcfff, MWA_RAMROM },
	{ 0xd000, 0xd1ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xd800, 0xdfff, system1_paletteram_w, &paletteram },
	{ 0xe7c0, 0xe7ff, choplifter_scroll_x_w, &system1_scrollx_ram },
	{ 0xe000, 0xe7ff, system1_videoram_w, &system1_videoram, &system1_videoram_size },
	{ 0xe800, 0xeeff, system1_backgroundram_w, &system1_backgroundram, &system1_backgroundram_size },
	{ 0xf000, 0xf3ff, system1_background_collisionram_w, &system1_background_collisionram },
	{ 0xf800, 0xfbff, system1_sprites_collisionram_w, &system1_sprites_collisionram },
	{ -1 } /* end of table */
};

static struct IOReadPort readport[] =
{
	{ 0x00, 0x00, input_port_0_r },	/* joy1 */
	{ 0x04, 0x04, input_port_1_r },	/* joy2 */
	{ 0x08, 0x08, input_port_2_r },	/* coin,start */
	{ 0x0c, 0x0c, input_port_3_r },	/* DIP2 */
	{ 0x0e, 0x0e, input_port_3_r },	/* DIP2 blckgalb reads it from here */
	{ 0x0d, 0x0d, input_port_4_r },	/* DIP1 some games read it from here... */
	{ 0x10, 0x10, input_port_4_r },	/* DIP1 ... and some others from here */
									/* but there are games which check BOTH! */
	{ 0x15, 0x15, system1_videomode_r },
	{ 0x19, 0x19, system1_videomode_r },	/* mirror address */
	{ -1 }	/* end of table */
};

static struct IOWritePort writeport[] =
{
	{ 0x14, 0x14, system1_soundport_w },	/* sound commands */
	{ 0x15, 0x15, system1_videomode_w },	/* video control and (in some games) bank switching */
	{ 0x18, 0x18, system1_soundport_w },	/* mirror address */
	{ 0x19, 0x19, system1_videomode_w },	/* mirror address */
	{ -1 }	/* end of table */
};

static struct IOReadPort wbml_readport[] =
{
	{ 0x00, 0x00, input_port_0_r },	/* joy1 */
	{ 0x04, 0x04, input_port_1_r },	/* joy2 */
	{ 0x08, 0x08, input_port_2_r },	/* coin,start */
	{ 0x0c, 0x0c, input_port_3_r },	/* DIP2 */
	{ 0x0d, 0x0d, input_port_4_r },	/* DIP1 some games read it from here... */
	{ 0x10, 0x10, input_port_4_r },	/* DIP1 ... and some others from here */
									/* but there are games which check BOTH! */
	{ 0x15, 0x15, wbml_bankswitch_r },
	{ 0x16, 0x16, wbml_bg_bankselect_r },
	{ 0x19, 0x19, wbml_bankswitch_r },	/* mirror address */
	{ -1 }	/* end of table */
};

static struct IOWritePort wbml_writeport[] =
{
	{ 0x14, 0x14, system1_soundport_w },	/* sound commands */
	{ 0x15, 0x15, wbml_bankswitch_w },
	{ 0x16, 0x16, wbml_bg_bankselect_w },
	{ -1 }	/* end of table */
};

static struct IOWritePort hvymetal_writeport[] =
{
	{ 0x18, 0x18, system1_soundport_w },	/* sound commands */
	{ 0x19, 0x19, hvymetal_bankswitch_w },
	{ -1 }	/* end of table */
};

static struct IOWritePort brain_writeport[] =
{
	{ 0x18, 0x18, system1_soundport_w },	/* sound commands */
	{ 0x19, 0x19, brain_bankswitch_w },
	{ -1 }	/* end of table */
};

static struct IOWritePort chplft_writeport[] =
{
	{ 0x14, 0x14, system1_soundport_w },	/* sound commands */
	{ 0x15, 0x15, chplft_bankswitch_w },
	{ -1 }	/* end of table */
};


static unsigned char *work_ram;

static int work_ram_r(int offset)
{
	return work_ram[offset];
}

static void work_ram_w(int offset,int data)
{
	work_ram[offset] = data;
}

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0x87ff, work_ram_r },
	{ 0x8800, 0x8fff, work_ram_r },
	{ 0xe000, 0xe000, soundlatch_r },
	{ 0xffff, 0xffff, soundlatch_r },	/* 4D warriors reads also from here - bug? */
	{ -1 } /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x7fff, MWA_ROM },
	{ 0x8000, 0x87ff, work_ram_w, &work_ram },
	{ 0x8800, 0x8fff, work_ram_w },
	{ 0xa000, 0xa003, SN76496_0_w },	/* Choplifter writes to the four addresses */
	{ 0xc000, 0xc003, SN76496_1_w },	/* in sequence */
	{ -1 } /* end of table */
};


#define IN0_PORT \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) \
	PORT_BITX(0x04, IP_ACTIVE_LOW, IPT_SERVICE, DEF_STR( Service_Mode ), KEYCODE_F2, IP_JOY_NONE ) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define DSW1_PORT \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" ) \
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" ) \
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) ) \
/*	PORT_DIPSETTING(    0x00, "1/1" ) */ \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" ) \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" ) \
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" ) \
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" ) \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
/*	PORT_DIPSETTING(    0x00, "1/1" ) */


INPUT_PORTS_START( starjack )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR (Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "20000 50000" )
	PORT_DIPSETTING(    0x20, "30000 70000" )
	PORT_DIPSETTING(    0x10, "40000 90000" )
	PORT_DIPSETTING(    0x00, "50000 110000" )
	PORT_DIPSETTING(    0x38, "20000" )
	PORT_DIPSETTING(    0x28, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, "Easy" )
	PORT_DIPSETTING(    0x80, "Medium" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
INPUT_PORTS_END

INPUT_PORTS_START( starjacs )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x08, 0x08, "Ship" )
	PORT_DIPSETTING(    0x08, "Single" )
	PORT_DIPSETTING(    0x00, "Multi" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "30000 70000" )
	PORT_DIPSETTING(    0x20, "40000 90000" )
	PORT_DIPSETTING(    0x10, "50000 110000" )
	PORT_DIPSETTING(    0x00, "60000 130000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, "Easy" )
	PORT_DIPSETTING(    0x80, "Medium" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
INPUT_PORTS_END

INPUT_PORTS_START( regulus )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x00, "Allow Continue" )
	PORT_DIPSETTING(    0x80, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
INPUT_PORTS_END

INPUT_PORTS_START( upndown )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x38, "10000" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x28, "30000" )
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, "Easy" )
	PORT_DIPSETTING(    0x80, "Medium" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
INPUT_PORTS_END

INPUT_PORTS_START( mrviking )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "10000 30000 60000" )
	PORT_DIPSETTING(    0x20, "20000 40000 70000" )
	PORT_DIPSETTING(    0x10, "30000 60000 90000" )
	PORT_DIPSETTING(    0x00, "40000 70000 100000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
INPUT_PORTS_END

INPUT_PORTS_START( swat )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_4WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_4WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_4WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_4WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x38, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x28, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x18, "70000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x08, "90000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( flicky )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "30000 80000 160000" )
	PORT_DIPSETTING(    0x20, "30000 100000 200000" )
	PORT_DIPSETTING(    0x10, "40000 120000 240000" )
	PORT_DIPSETTING(    0x00, "40000 140000 280000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Difficulty?" )
	PORT_DIPSETTING(    0xc0, "Easy?" )
	PORT_DIPSETTING(    0x80, "Medium?" )
	PORT_DIPSETTING(    0x40, "Hard?" )
	PORT_DIPSETTING(    0x00, "Hardest?" )
INPUT_PORTS_END

INPUT_PORTS_START( bullfgtj )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x10, "70000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( pitfall2 )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "20000 50000" )
	PORT_DIPSETTING(    0x00, "30000 70000" )
	PORT_DIPNAME( 0x20, 0x00, "Allow Continue" )
	PORT_DIPSETTING(    0x20, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x40, 0x40, "Time" )
	PORT_DIPSETTING(    0x00, "2 Minutes" )
	PORT_DIPSETTING(    0x40, "3 Minutes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( pitfallu )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x18, 0x18, "Starting Stage" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x20, 0x00, "Allow Continue" )
	PORT_DIPSETTING(    0x20, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x40, 0x40, "Time" )
	PORT_DIPSETTING(    0x00, "2 Minutes" )
	PORT_DIPSETTING(    0x40, "3 Minutes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( seganinj )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "20k 70k 120k 170k" )
	PORT_DIPSETTING(    0x00, "60k 100k 160k 200k" )
	PORT_DIPNAME( 0x20, 0x00, "Allow Continue" )
	PORT_DIPSETTING(    0x20, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( imsorry )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0C, 0x0C, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0C, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x10, "50000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( teddybb )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "100k 400k" )
	PORT_DIPSETTING(    0x20, "200k 600k" )
	PORT_DIPSETTING(    0x10, "400k 800k" )
	PORT_DIPSETTING(    0x00, "600k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( hvymetal )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "50000 100000" )
	PORT_DIPSETTING(    0x20, "60000 120000" )
	PORT_DIPSETTING(    0x10, "70000 150000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x00, "Allow Continue" )
	PORT_DIPSETTING(    0x80, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
INPUT_PORTS_END

INPUT_PORTS_START( myhero )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x10, "70000" )
	PORT_DIPSETTING(    0x00, "90000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( chplft )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 70000" )
	PORT_DIPSETTING(    0x20, "50000 100000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Easy" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START      /* DSW0 */
	DSW1_PORT
INPUT_PORTS_END

INPUT_PORTS_START( fdwarrio )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x38, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x28, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x18, "70000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x08, "90000" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( brain )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( wboy )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "30k 100k 170k 240k" )
	PORT_DIPSETTING(    0x00, "30k 120k 210k 300k" )
	PORT_DIPNAME( 0x20, 0x20, "Allow Continue" )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x20, "Yes" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* same as wboy, additional Energy Consumption switch */
INPUT_PORTS_START( wbdeluxe )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Has to be 0 otherwise the game resets */
												/* if you die after level 1. */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START      /* DSW1 */
	DSW1_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "30k 100k 170k 240k" )
	PORT_DIPSETTING(    0x00, "30k 120k 210k 300k" )
	PORT_DIPNAME( 0x20, 0x20, "Allow Continue" )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x20, "Yes" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x00, "Energy Consumption" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )
INPUT_PORTS_END

INPUT_PORTS_START( wboyu )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_2WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "4 Coins/2 Credits" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x02, "2 Coins/2 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Allow Continue" )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x10, "Yes" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Mode" )
	PORT_DIPSETTING(    0xc0, "Normal Game" )
	PORT_DIPSETTING(    0x80, "Free Play" )
	PORT_DIPSETTING(    0x40, "Test Mode" )
	PORT_DIPSETTING(    0x00, "Endless Game" )
INPUT_PORTS_END

INPUT_PORTS_START( blockgal )
	PORT_START	/* IN1 */
	PORT_ANALOG( 0xff, 0x00, IPT_DIAL | IPF_REVERSE, 60, 15, 0, 0, 0)

	PORT_START	/* IN2 */
	PORT_ANALOG( 0xff, 0x00, IPT_DIAL | IPF_REVERSE | IPF_COCKTAIL, 60, 15, 0, 0, 0)

	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )

	PORT_START	/* DSW0 */
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START      /* DSW1 */
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( tokisens )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START      /* DSW1 */
	DSW1_PORT
INPUT_PORTS_END

INPUT_PORTS_START( wbml )
	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )

	PORT_START	/* IN0 */
	IN0_PORT

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
/* 0x00 gives 4 lives */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "30000 100000 200000" )
	PORT_DIPSETTING(    0x00, "50000 150000 250000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_BITX(    0x40, 0x40, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Test Mode", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START      /* DSW0 */
	DSW1_PORT
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,	/* 8 by 8 */
	2048,	/* 2048 characters */
	3,		/* 3 bits per pixel */
	{ 0, 2048*8*8, 2*2048*8*8 },			/* plane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static struct GfxLayout chplft_charlayout =
{
	8,8,	/* 8 by 8 */
	4096,	/* 4096 characters */
	3,	/* 3 bits per pixel */
	{ 0, 4096*8*8, 2*4096*8*8 },		/* plane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};



static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	/* sprites use colors 0-511, but are not defined here */
	{ 1, 0x0000, &charlayout, 512, 128 },
	{ -1 } /* end of array */
};

static struct GfxDecodeInfo chplft_gfxdecodeinfo[] =
{
	/* sprites use colors 0-511, but are not defined here */
	{ 1, 0x0000, &chplft_charlayout, 512, 128 },
	{ -1 } /* end of array */
};



static struct SN76496interface sn76496_interface =
{
	2,		/* 2 chips */
	{ 2000000, 4000000 },	/* 8 MHz / 4 ?*/
	{ 50, 50 }
};



static struct MachineDriver system1_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			4000000,	/* My Hero has 2 OSCs 8 & 20 MHz (Cabbe Info) */
			0,		/* memory region */
			readmem,writemem,readport,writeport,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,
			3,		/* memory region */
			sound_readmem,sound_writemem,0,0,
			interrupt,4			/* NMIs are caused by the main CPU */
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,					/* single CPU, no need for interleaving */
	system1_init_machine,

	/* video hardware */
	256, 256,				/* screen_width, screen_height */
	{ 0*8, 32*8-1, 0*8, 28*8-1 },			/* struct rectangle visible_area */
	gfxdecodeinfo,				/* GfxDecodeInfo */
	2048,					/* total colors */
	2048,					/* color table length */
	system1_vh_convert_color_prom,		/* convert color prom routine */

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,					/* vh_init routine */
	system1_vh_start,			/* vh_start routine */
	system1_vh_stop,			/* vh_stop routine */
	system1_vh_screenrefresh,		/* vh_update routine */

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_SN76496,
			&sn76496_interface
		}
	}
};

/* driver with reduced visible area for scrolling games */
static struct MachineDriver system1_small_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			4000000,	/* My Hero has 2 OSCs 8 & 20 MHz (Cabbe Info) */
			0,		/* memory region */
			readmem,writemem,readport,writeport,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,
			3,		/* memory region */
			sound_readmem,sound_writemem,0,0,
			interrupt,4			/* NMIs are caused by the main CPU */
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,					/* single CPU, no need for interleaving */
	system1_init_machine,

	/* video hardware */
	256, 256,				/* screen_width, screen_height */
	{ 0*8+8, 32*8-1-8, 0*8, 28*8-1 },			/* struct rectangle visible_area */
	gfxdecodeinfo,				/* GfxDecodeInfo */
	2048,					/* total colors */
	2048,					/* color table length */
	system1_vh_convert_color_prom,		/* convert color prom routine */

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,					/* vh_init routine */
	system1_vh_start,			/* vh_start routine */
	system1_vh_stop,			/* vh_stop routine */
	system1_vh_screenrefresh,		/* vh_update routine */

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_SN76496,
			&sn76496_interface
		}
	}
};

static struct MachineDriver pitfall2_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			3650000,			/* 3.65 MHz ? changing it to 4 makes the title disappear */
			0,				/* memory region */
			readmem,writemem,readport,writeport,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			3000000,			/* 3 Mhz ? */
			3,				/* memory region */
			sound_readmem,sound_writemem,0,0,
			interrupt,4			/* NMIs are caused by the main CPU */
		},

	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,					/* single CPU, no need for interleaving */
	system1_init_machine,

	/* video hardware */
	256, 256,				/* screen_width, screen_height */
	{ 0*8, 32*8-1, 0*8, 28*8-1 },		/* struct rectangle visible_area */
	gfxdecodeinfo,				/* GfxDecodeInfo */
	2048,					/* total colors */
	2048,					/* color table length */
	system1_vh_convert_color_prom,	        /* convert color prom routine */

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,					/* vh_init routine */
	system1_vh_start,			/* vh_start routine */
	system1_vh_stop,			/* vh_stop routine */
	system1_vh_screenrefresh,		/* vh_update routine */

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_SN76496,
			&sn76496_interface
		}
	}
};

static struct MachineDriver hvymetal_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			4000000,			/* 4 MHz ? */
			0,				/* memory region */
			chplft_readmem,writemem,wbml_readport,hvymetal_writeport,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,			/* 4 Mhz ? */
			3,				/* memory region */
			sound_readmem,sound_writemem,0,0,
			interrupt,4			/* NMIs are caused by the main CPU */
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	        /* frames per second, vblank duration */
	1,					/* single CPU, no need for interleaving */
	chplft_init_machine,

	/* video hardware */
	256, 256,					/* screen_width, screen_height */
	{ 0*8, 32*8-1, 0*8, 28*8-1 },			/* struct rectangle visible_area */
	chplft_gfxdecodeinfo,			        /* GfxDecodeInfo */
	2048,						/* total colors */
	2048,						/* color table length */
	system1_vh_convert_color_prom,	/* convert color prom routine */

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,								/* vh_init routine */
	system1_vh_start,				/* vh_start routine */
	system1_vh_stop,				/* vh_stop routine */
	system1_vh_screenrefresh,		        /* vh_update routine */

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_SN76496,
			&sn76496_interface
		}
	}
};

static struct MachineDriver chplft_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			4000000,			/* 4 MHz ? */
			0,				/* memory region */
			chplft_readmem,chplft_writemem,wbml_readport,chplft_writeport,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,			/* 4 Mhz ? */
			3,				/* memory region */
			sound_readmem,sound_writemem,0,0,
			interrupt,4			/* NMIs are caused by the main CPU */
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	        /* frames per second, vblank duration */
	1,					/* single CPU, no need for interleaving */
	chplft_init_machine,

	/* video hardware */
	256, 256,					/* screen_width, screen_height */
	{ 0*8, 32*8-1, 0*8, 28*8-1 },			/* struct rectangle visible_area */
	chplft_gfxdecodeinfo,			        /* GfxDecodeInfo */
	2048,						/* total colors */
	2048,						/* color table length */
	system1_vh_convert_color_prom,	/* convert color prom routine */

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,								/* vh_init routine */
	system1_vh_start,				/* vh_start routine */
	system1_vh_stop,				/* vh_stop routine */
	choplifter_vh_screenrefresh,		        /* vh_update routine */

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_SN76496,
			&sn76496_interface
		}
	}
};

static struct MachineDriver brain_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			4000000,	/* My Hero has 2 OSCs 8 & 20 MHz (Cabbe Info) */
			0,		/* memory region */
			readmem,writemem,readport,brain_writeport,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,
			3,		/* memory region */
			sound_readmem,sound_writemem,0,0,
			interrupt,4			/* NMIs are caused by the main CPU */
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,					/* single CPU, no need for interleaving */
	system1_init_machine,

	/* video hardware */
	256, 256,				/* screen_width, screen_height */
	{ 0*8, 32*8-1, 0*8, 28*8-1 },			/* struct rectangle visible_area */
	gfxdecodeinfo,				/* GfxDecodeInfo */
	2048,					/* total colors */
	2048,					/* color table length */
	system1_vh_convert_color_prom,		/* convert color prom routine */

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,					/* vh_init routine */
	system1_vh_start,			/* vh_start routine */
	system1_vh_stop,			/* vh_stop routine */
	system1_vh_screenrefresh,		/* vh_update routine */

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_SN76496,
			&sn76496_interface
		}
	}
};

static struct MachineDriver wbml_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			4000000,			/* 4 MHz ? */
			0,				/* memory region */
			wbml_readmem,wbml_writemem,wbml_readport,wbml_writeport,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			4000000,			/* 4 Mhz ? */
			3,				/* memory region */
			sound_readmem,sound_writemem,0,0,
			interrupt,4			/* NMIs are caused by the main CPU */
		},
	},
	60, DEFAULT_60HZ_VBLANK_DURATION,	/* frames per second, vblank duration */
	1,					/* single CPU, no need for interleaving */
	wbml_init_machine,

	/* video hardware */
	256, 256,				/* screen_width, screen_height */
	{ 0*8, 32*8-1, 0*8, 28*8-1 },		/* struct rectangle visible_area */
	chplft_gfxdecodeinfo,			/* GfxDecodeInfo */
	1536, 1536,
	system1_vh_convert_color_prom,	        /* convert color prom routine */

	VIDEO_TYPE_RASTER | VIDEO_MODIFIES_PALETTE,
	0,					/* vh_init routine */
	system1_vh_start,			/* vh_start routine */
	system1_vh_stop,			/* vh_stop routine */
	wbml_vh_screenrefresh,		        /* vh_update routine */

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_SN76496,
			&sn76496_interface
		}
	}
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

/* Since the standard System 1 PROM has part # 5317, Star Jacker, whose first */
/* ROM is #5318, is probably the first or second System 1 game produced */
ROM_START( starjack )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "5320b",        0x0000, 0x2000, 0x7ab72ecd )
	ROM_LOAD( "5321a",        0x2000, 0x2000, 0x38b99050 )
	ROM_LOAD( "5322a",        0x4000, 0x2000, 0x103a595b )
	ROM_LOAD( "5323",         0x6000, 0x2000, 0x46af0d58 )
	ROM_LOAD( "5324",         0x8000, 0x2000, 0x1e89efe2 )
	ROM_LOAD( "5325",         0xa000, 0x2000, 0xd6e379a1 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "5331",         0x0000, 0x2000, 0x251d898f )
	ROM_LOAD( "5330",         0x2000, 0x2000, 0xeb048745 )
	ROM_LOAD( "5329",         0x4000, 0x2000, 0x3e8bcaed )
	ROM_LOAD( "5328",         0x6000, 0x2000, 0x9ed7849f )
	ROM_LOAD( "5327",         0x8000, 0x2000, 0x79e92cb1 )
	ROM_LOAD( "5326",         0xa000, 0x2000, 0xba7e2b47 )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "5318",         0x0000, 0x4000, 0x6f2e1fd3 )
	ROM_LOAD( "5319",         0x4000, 0x4000, 0xebee4999 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "5332",         0x0000, 0x2000, 0x7a72ab3d )
ROM_END

ROM_START( starjacs )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "sja1ic29",     0x0000, 0x2000, 0x59a22a1f )
	ROM_LOAD( "sja1ic30",     0x2000, 0x2000, 0x7f4597dc )
	ROM_LOAD( "sja1ic31",     0x4000, 0x2000, 0x6074c046 )
	ROM_LOAD( "sja1ic32",     0x6000, 0x2000, 0x1c48a3fa )
	ROM_LOAD( "sja1ic33",     0x8000, 0x2000, 0x7598bd51 )
	ROM_LOAD( "sja1ic34",     0xa000, 0x2000, 0xf66fa604 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "5331",         0x0000, 0x2000, 0x251d898f )
	ROM_LOAD( "sja1ic65",     0x2000, 0x2000, 0x0ab1893c )
	ROM_LOAD( "5329",         0x4000, 0x2000, 0x3e8bcaed )
	ROM_LOAD( "sja1ic64",     0x6000, 0x2000, 0x7f628ae6 )
	ROM_LOAD( "5327",         0x8000, 0x2000, 0x79e92cb1 )
	ROM_LOAD( "sja1ic63",     0xa000, 0x2000, 0x5bcb253e )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	/* SJA1IC86 and SJA1IC93 in the original set were bad, so I'm using the ones */
	/* from the Sega version. However I suspect the real ones should be slightly */
	/* different. */
	ROM_LOAD( "5318",         0x0000, 0x4000, 0x6f2e1fd3 )
	ROM_LOAD( "5319",         0x4000, 0x4000, 0xebee4999 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "5332",         0x0000, 0x2000, 0x7a72ab3d )
ROM_END

ROM_START( regulus )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr5640a.29",  0x0000, 0x2000, 0xdafb1528 )	/* encrypted */
	ROM_LOAD( "epr5641a.30",  0x2000, 0x2000, 0x0fcc850e )	/* encrypted */
	ROM_LOAD( "epr5642a.31",  0x4000, 0x2000, 0x4feffa17 )	/* encrypted */
	ROM_LOAD( "epr5643a.32",  0x6000, 0x2000, 0xb8ac7eb4 )	/* encrypted */
	ROM_LOAD( "epr5644.33",   0x8000, 0x2000, 0xffd05b7d )
	ROM_LOAD( "epr5645a.34",  0xa000, 0x2000, 0x6b4bf77c )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr5651.82",   0x0000, 0x2000, 0xf07f3e82 )
	ROM_LOAD( "epr5650.65",   0x2000, 0x2000, 0x84c1baa2 )
	ROM_LOAD( "epr5649.81",   0x4000, 0x2000, 0x6774c895 )
	ROM_LOAD( "epr5648.64",   0x6000, 0x2000, 0x0c69e92a )
	ROM_LOAD( "epr5647.80",   0x8000, 0x2000, 0x9330f7b5 )
	ROM_LOAD( "epr5646.63",   0xa000, 0x2000, 0x4dfacbbc )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr5638.92",   0x0000, 0x4000, 0x617363dd )
	ROM_LOAD( "epr5639.93",   0x4000, 0x4000, 0xa4ec5131 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr5652.3",    0x0000, 0x2000, 0x74edcb98 )
ROM_END

ROM_START( regulusu )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr-5950.129", 0x0000, 0x2000, 0x3b047b67 )
	ROM_LOAD( "epr-5951.130", 0x2000, 0x2000, 0xd66453ab )
	ROM_LOAD( "epr-5952.131", 0x4000, 0x2000, 0xf3d0158a )
	ROM_LOAD( "epr-5953.132", 0x6000, 0x2000, 0xa9ad4f44 )
	ROM_LOAD( "epr5644.33",   0x8000, 0x2000, 0xffd05b7d )
	ROM_LOAD( "epr-5955.134", 0xa000, 0x2000, 0x65ddb2a3 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr5651.82",   0x0000, 0x2000, 0xf07f3e82 )
	ROM_LOAD( "epr5650.65",   0x2000, 0x2000, 0x84c1baa2 )
	ROM_LOAD( "epr5649.81",   0x4000, 0x2000, 0x6774c895 )
	ROM_LOAD( "epr5648.64",   0x6000, 0x2000, 0x0c69e92a )
	ROM_LOAD( "epr5647.80",   0x8000, 0x2000, 0x9330f7b5 )
	ROM_LOAD( "epr5646.63",   0xa000, 0x2000, 0x4dfacbbc )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr5638.92",   0x0000, 0x4000, 0x617363dd )
	ROM_LOAD( "epr5639.93",   0x4000, 0x4000, 0xa4ec5131 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr5652.3",    0x0000, 0x2000, 0x74edcb98 )
ROM_END

ROM_START( upndown )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "upnd5679.bin", 0x0000, 0x2000, 0xc4f2f9c2 )
	ROM_LOAD( "upnd5680.bin", 0x2000, 0x2000, 0x837f021c )
	ROM_LOAD( "upnd5681.bin", 0x4000, 0x2000, 0xe1c7ff7e )
	ROM_LOAD( "upnd5682.bin", 0x6000, 0x2000, 0x4a5edc1e )
	ROM_LOAD( "upnd5683.bin", 0x8000, 0x2000, 0x208dfbdf )
	ROM_LOAD( "upnd5684.bin", 0xA000, 0x2000, 0x32fa95da )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "upnd5527.bin", 0x0000, 0x2000, 0xb2d616f1 )
	ROM_LOAD( "upnd5526.bin", 0x2000, 0x2000, 0x8a8b33c2 )
	ROM_LOAD( "upnd5525.bin", 0x4000, 0x2000, 0xe749c5ef )
	ROM_LOAD( "upnd5524.bin", 0x6000, 0x2000, 0x8b886952 )
	ROM_LOAD( "upnd5523.bin", 0x8000, 0x2000, 0xdede35d9 )
	ROM_LOAD( "upnd5522.bin", 0xA000, 0x2000, 0x5e6d9dff )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "upnd5514.bin", 0x0000, 0x4000, 0xfcc0a88b )
	ROM_LOAD( "upnd5515.bin", 0x4000, 0x4000, 0x60908838 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "upnd5528.bin", 0x0000, 0x2000, 0x00cd44ab )
ROM_END

ROM_START( mrviking )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "vepr5873",     0x0000, 0x2000, 0x14d21624 )	/* encrypted */
	ROM_LOAD( "vepr5874",     0x2000, 0x2000, 0x6df7de87 )	/* encrypted */
	ROM_LOAD( "vepr5975",     0x4000, 0x2000, 0xac226100 )	/* encrypted */
	ROM_LOAD( "vepr5876",     0x6000, 0x2000, 0xe77db1dc )	/* encrypted */
	ROM_LOAD( "vepr5755",     0x8000, 0x2000, 0xedd62ae1 )
	ROM_LOAD( "vepr5756",     0xa000, 0x2000, 0x11974040 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "vepr5762",     0x0000, 0x2000, 0x4a91d08a )
	ROM_LOAD( "vepr5761",     0x2000, 0x2000, 0xf7d61b65 )
	ROM_LOAD( "vepr5760",     0x4000, 0x2000, 0x95045820 )
	ROM_LOAD( "vepr5759",     0x6000, 0x2000, 0x5f9bae4e )
	ROM_LOAD( "vepr5758",     0x8000, 0x2000, 0x808ee706 )
	ROM_LOAD( "vepr5757",     0xa000, 0x2000, 0x480f7074 )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "viepr574",     0x0000, 0x4000, 0xe24682cd )
	ROM_LOAD( "vepr5750",     0x4000, 0x4000, 0x6564d1ad )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "vepr5763",     0x0000, 0x2000, 0xd712280d )
ROM_END

ROM_START( swat )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr5807b.29",  0x0000, 0x2000, 0x93db9c9f )	/* encrypted */
	ROM_LOAD( "epr5808.30",   0x2000, 0x2000, 0x67116665 )	/* encrypted */
	ROM_LOAD( "epr5809.31",   0x4000, 0x2000, 0xfd792fc9 )	/* encrypted */
	ROM_LOAD( "epr5810.32",   0x6000, 0x2000, 0xdc2b279d )	/* encrypted */
	ROM_LOAD( "epr5811.33",   0x8000, 0x2000, 0x093e3ab1 )
	ROM_LOAD( "epr5812.34",   0xa000, 0x2000, 0x5bfd692f )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr5818.82",   0x0000, 0x2000, 0xb22033d9 )
	ROM_LOAD( "epr5817.65",   0x2000, 0x2000, 0xfd942797 )
	ROM_LOAD( "epr5816.81",   0x4000, 0x2000, 0x4384376d )
	ROM_LOAD( "epr5815.64",   0x6000, 0x2000, 0x16ad046c )
	ROM_LOAD( "epr5814.80",   0x8000, 0x2000, 0xbe721c99 )
	ROM_LOAD( "epr5813.63",   0xa000, 0x2000, 0x0d42c27e )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr5805.92",   0x0000, 0x4000, 0x5a732865 )
	ROM_LOAD( "epr5806.93",   0x4000, 0x4000, 0x26ac258c )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr5819.3",    0x0000, 0x2000, 0xf6afd0fd )
ROM_END

ROM_START( flicky )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr5978",      0x0000, 0x4000, 0x296f1492 )	/* encrypted */
	ROM_LOAD( "epr5979",      0x4000, 0x4000, 0x64b03ef9 )	/* encrypted */

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6001",      0x0000, 0x4000, 0xf1a75200 )
	ROM_LOAD( "epr6000",      0x4000, 0x4000, 0x299aefb7 )
	ROM_LOAD( "epr5999",      0x8000, 0x4000, 0x1ca53157 )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr5855",      0x0000, 0x4000, 0xb5f894a1 )
	ROM_LOAD( "epr5856",      0x4000, 0x4000, 0x266af78f )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr5869",      0x0000, 0x2000, 0x6d220d4e )
ROM_END

ROM_START( flicky2 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "f_9",          0x0000, 0x2000, 0xa65ac88e )	/* encrypted */
	ROM_LOAD( "f_10",         0x2000, 0x2000, 0x18b412f4 )	/* encrypted */
	ROM_LOAD( "f_11",         0x4000, 0x2000, 0xa5558d7e )	/* encrypted */
	ROM_LOAD( "f_12",         0x6000, 0x2000, 0x1b35fef1 )	/* encrypted */

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6001",      0x0000, 0x4000, 0xf1a75200 )
	ROM_LOAD( "epr6000",      0x4000, 0x4000, 0x299aefb7 )
	ROM_LOAD( "epr5999",      0x8000, 0x4000, 0x1ca53157 )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr5855",      0x0000, 0x4000, 0xb5f894a1 )
	ROM_LOAD( "epr5856",      0x4000, 0x4000, 0x266af78f )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr5869",      0x0000, 0x2000, 0x6d220d4e )
ROM_END

ROM_START( bullfgtj )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "6071",         0x0000, 0x4000, 0x96b57df9 )	/* encrypted */
	ROM_LOAD( "6072",         0x4000, 0x4000, 0xf7baadd0 )	/* encrypted */
	ROM_LOAD( "6073",         0x8000, 0x4000, 0x721af166 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6089",         0x0000, 0x2000, 0xa183e5ff )
	ROM_LOAD( "6088",         0x2000, 0x2000, 0xb919b4a6 )
	ROM_LOAD( "6087",         0x4000, 0x2000, 0x2677742c )
	ROM_LOAD( "6086",         0x6000, 0x2000, 0x76b5a084 )
	ROM_LOAD( "6085",         0x8000, 0x2000, 0x9c3ddc62 )
	ROM_LOAD( "6084",         0xA000, 0x2000, 0x90e1fa5f )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "6069",         0x0000, 0x4000, 0xfe691e41 )
	ROM_LOAD( "6070",         0x4000, 0x4000, 0x34f080df )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "6077",         0x0000, 0x2000, 0x02a37602 )
ROM_END

ROM_START( pitfall2 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr6456a.116", 0x0000, 0x4000, 0xbcc8406b )	/* encrypted */
	ROM_LOAD( "epr6457a.109", 0x4000, 0x4000, 0xa016fd2a )	/* encrypted */
	ROM_LOAD( "epr6458a.96",  0x8000, 0x4000, 0x5c30b3e8 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6474a.62",  0x0000, 0x2000, 0x9f1711b9 )
	ROM_LOAD( "epr6473a.61",  0x2000, 0x2000, 0x8e53b8dd )
	ROM_LOAD( "epr6472a.64",  0x4000, 0x2000, 0xe0f34a11 )
	ROM_LOAD( "epr6471a.63",  0x6000, 0x2000, 0xd5bc805c )
	ROM_LOAD( "epr6470a.66",  0x8000, 0x2000, 0x1439729f )
	ROM_LOAD( "epr6469a.65",  0xa000, 0x2000, 0xe4ac6921 )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr6454a.117", 0x0000, 0x4000, 0xa5d96780 )
	ROM_LOAD( "epr6455.05",   0x4000, 0x4000, 0x32ee64a1 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr6462.120",  0x0000, 0x2000, 0x86bb9185 )
ROM_END

ROM_START( pitfallu )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr6623",      0x0000, 0x4000, 0xbcb47ed6 )
	ROM_LOAD( "epr6624a",     0x4000, 0x4000, 0x6e8b09c1 )
	ROM_LOAD( "epr6625",      0x8000, 0x4000, 0xdc5484ba )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6474a.62",  0x0000, 0x2000, 0x9f1711b9 )
	ROM_LOAD( "epr6473a.61",  0x2000, 0x2000, 0x8e53b8dd )
	ROM_LOAD( "epr6472a.64",  0x4000, 0x2000, 0xe0f34a11 )
	ROM_LOAD( "epr6471a.63",  0x6000, 0x2000, 0xd5bc805c )
	ROM_LOAD( "epr6470a.66",  0x8000, 0x2000, 0x1439729f )
	ROM_LOAD( "epr6469a.65",  0xa000, 0x2000, 0xe4ac6921 )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr6454a.117", 0x0000, 0x4000, 0xa5d96780 )
	ROM_LOAD( "epr6455.05",   0x4000, 0x4000, 0x32ee64a1 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr6462.120",  0x0000, 0x2000, 0x86bb9185 )
ROM_END

ROM_START( seganinj )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic116.bin",    0x0000, 0x4000, 0xa5d0c9d0 )	/* encrypted */
	ROM_LOAD( "ic109.bin",    0x4000, 0x4000, 0xb9e6775c )	/* encrypted */
	ROM_LOAD( "7151.96",      0x8000, 0x4000, 0xf2eeb0d8 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6558.82",      0x0000, 0x2000, 0x2af9eaeb )
	ROM_LOAD( "6592.61",      0x2000, 0x2000, 0x7804db86 )
	ROM_LOAD( "6556.81",      0x4000, 0x2000, 0x79fd26f7 )
	ROM_LOAD( "6590.63",      0x6000, 0x2000, 0xbf858cad )
	ROM_LOAD( "6554.80",      0x8000, 0x2000, 0x5ac9d205 )
	ROM_LOAD( "6588.65",      0xA000, 0x2000, 0xdc931dbb )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "6546.117",     0x0000, 0x4000, 0xa4785692 )
	ROM_LOAD( "6548.04",      0x4000, 0x4000, 0xbdf278c1 )
	ROM_LOAD( "6547.110",     0x8000, 0x4000, 0x34451b08 )
	ROM_LOAD( "6549.05",      0xc000, 0x4000, 0xd2057668 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "6559.120",     0x0000, 0x2000, 0x5a1570ee )
ROM_END

ROM_START( seganinu )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "7149.116",     0x0000, 0x4000, 0xcd9fade7 )
	ROM_LOAD( "7150.109",     0x4000, 0x4000, 0xc36351e2 )
	ROM_LOAD( "7151.96",      0x8000, 0x4000, 0xf2eeb0d8 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6558.82",      0x0000, 0x2000, 0x2af9eaeb )
	ROM_LOAD( "6592.61",      0x2000, 0x2000, 0x7804db86 )
	ROM_LOAD( "6556.81",      0x4000, 0x2000, 0x79fd26f7 )
	ROM_LOAD( "6590.63",      0x6000, 0x2000, 0xbf858cad )
	ROM_LOAD( "6554.80",      0x8000, 0x2000, 0x5ac9d205 )
	ROM_LOAD( "6588.65",      0xA000, 0x2000, 0xdc931dbb )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "6546.117",     0x0000, 0x4000, 0xa4785692 )
	ROM_LOAD( "6548.04",      0x4000, 0x4000, 0xbdf278c1 )
	ROM_LOAD( "6547.110",     0x8000, 0x4000, 0x34451b08 )
	ROM_LOAD( "6549.05",      0xc000, 0x4000, 0xd2057668 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "6559.120",     0x0000, 0x2000, 0x5a1570ee )
ROM_END

ROM_START( nprinces )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr6550.116",  0x0000, 0x4000, 0x5f6d59f1 )	/* encrypted */
	ROM_LOAD( "epr6551.109",  0x4000, 0x4000, 0x1af133b2 )	/* encrypted */
	ROM_LOAD( "7151.96",      0x8000, 0x4000, 0xf2eeb0d8 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6558.82",      0x0000, 0x2000, 0x2af9eaeb )
	ROM_LOAD( "6557.65",      0x2000, 0x2000, 0x6eb131d0 )
	ROM_LOAD( "6556.81",      0x4000, 0x2000, 0x79fd26f7 )
	ROM_LOAD( "6555.64",      0x6000, 0x2000, 0x7f669aac )
	ROM_LOAD( "6554.80",      0x8000, 0x2000, 0x5ac9d205 )
	ROM_LOAD( "6553.63",      0xA000, 0x2000, 0xeb82a8fe )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "6546.117",     0x0000, 0x4000, 0xa4785692 )
	ROM_LOAD( "6548.04",      0x4000, 0x4000, 0xbdf278c1 )
	ROM_LOAD( "6547.110",     0x8000, 0x4000, 0x34451b08 )
	ROM_LOAD( "6549.05",      0xc000, 0x4000, 0xd2057668 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "6559.120",     0x0000, 0x2000, 0x5a1570ee )
ROM_END

ROM_START( nprincsu )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "6573.129",     0x0000, 0x2000, 0xd2919c7d )
	ROM_LOAD( "6574.130",     0x2000, 0x2000, 0x5a132833 )
	ROM_LOAD( "6575.131",     0x4000, 0x2000, 0xa94b0bd4 )
	ROM_LOAD( "6576.132",     0x6000, 0x2000, 0x27d3bbdb )
	ROM_LOAD( "6577.133",     0x8000, 0x2000, 0x73616e03 )
	ROM_LOAD( "6578.134",     0xa000, 0x2000, 0xab68499f )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6558.82",      0x0000, 0x2000, 0x2af9eaeb )
	ROM_LOAD( "6557.65",      0x2000, 0x2000, 0x6eb131d0 )
	ROM_LOAD( "6556.81",      0x4000, 0x2000, 0x79fd26f7 )
	ROM_LOAD( "6555.64",      0x6000, 0x2000, 0x7f669aac )
	ROM_LOAD( "6554.80",      0x8000, 0x2000, 0x5ac9d205 )
	ROM_LOAD( "6553.63",      0xA000, 0x2000, 0xeb82a8fe )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "6546.117",     0x0000, 0x4000, 0xa4785692 )
	ROM_LOAD( "6548.04",      0x4000, 0x4000, 0xbdf278c1 )
	ROM_LOAD( "6547.110",     0x8000, 0x4000, 0x34451b08 )
	ROM_LOAD( "6549.05",      0xc000, 0x4000, 0xd2057668 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "6559.120",     0x0000, 0x2000, 0x5a1570ee )
ROM_END

ROM_START( nprincsb )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "nprinces.001", 0x0000, 0x4000, 0xe0de073c )	/* encrypted */
	ROM_LOAD( "nprinces.002", 0x4000, 0x4000, 0x27219c7f )	/* encrypted */
	ROM_LOAD( "7151.96",      0x8000, 0x4000, 0xf2eeb0d8 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6558.82",      0x0000, 0x2000, 0x2af9eaeb )
	ROM_LOAD( "6557.65",      0x2000, 0x2000, 0x6eb131d0 )
	ROM_LOAD( "6556.81",      0x4000, 0x2000, 0x79fd26f7 )
	ROM_LOAD( "6555.64",      0x6000, 0x2000, 0x7f669aac )
	ROM_LOAD( "6554.80",      0x8000, 0x2000, 0x5ac9d205 )
	ROM_LOAD( "6553.63",      0xA000, 0x2000, 0xeb82a8fe )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "6546.117",     0x0000, 0x4000, 0xa4785692 )
	ROM_LOAD( "6548.04",      0x4000, 0x4000, 0xbdf278c1 )
	ROM_LOAD( "6547.110",     0x8000, 0x4000, 0x34451b08 )
	ROM_LOAD( "6549.05",      0xc000, 0x4000, 0xd2057668 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "6559.120",     0x0000, 0x2000, 0x5a1570ee )
ROM_END

ROM_START( imsorry )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr6676.116",  0x0000, 0x4000, 0xeb087d7f )	/* encrypted */
	ROM_LOAD( "epr6677.109",  0x4000, 0x4000, 0xbd244bee )	/* encrypted */
	ROM_LOAD( "epr6678.96",   0x8000, 0x4000, 0x2e16b9fd )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6684.u62",  0x0000, 0x2000, 0x2c8df377 )
	ROM_LOAD( "epr6683.u61",  0x2000, 0x2000, 0x89431c48 )
	ROM_LOAD( "epr6682.u64",  0x4000, 0x2000, 0x256a9246 )
	ROM_LOAD( "epr6681.u63",  0x6000, 0x2000, 0x6974d189 )
	ROM_LOAD( "epr6680.u66",  0x8000, 0x2000, 0x10a629d6 )
	ROM_LOAD( "epr6674.u65",  0xA000, 0x2000, 0x143d883c )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr66xx.117",  0x0000, 0x4000, 0x1ba167ee )
	ROM_LOAD( "epr66xx.u04",  0x4000, 0x4000, 0xedda7ad6 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr6656.113",  0x0000, 0x2000, 0x25e3d685 )
ROM_END

ROM_START( imsorryj )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "6647.116",     0x0000, 0x4000, 0xcc5d915d )	/* encrypted */
	ROM_LOAD( "6648.109",     0x4000, 0x4000, 0x37574d60 )	/* encrypted */
	ROM_LOAD( "6649.96",      0x8000, 0x4000, 0x5f59bdee )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6655.62",      0x0000, 0x2000, 0xbe1f762f )
	ROM_LOAD( "6654.61",      0x2000, 0x2000, 0xed5f7fc8 )
	ROM_LOAD( "6653.64",      0x4000, 0x2000, 0x8b4845a7 )
	ROM_LOAD( "6652.63",      0x6000, 0x2000, 0x001d68cb )
	ROM_LOAD( "6651.66",      0x8000, 0x2000, 0x4ee9b5e6 )
	ROM_LOAD( "6650.65",      0xA000, 0x2000, 0x3fca4414 )

	ROM_REGION(0x8000)	/* 32k for sprites data */
	ROM_LOAD( "epr66xx.117",  0x0000, 0x4000, 0x1ba167ee )
	ROM_LOAD( "epr66xx.u04",  0x4000, 0x4000, 0xedda7ad6 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr6656.113",  0x0000, 0x2000, 0x25e3d685 )
ROM_END

ROM_START( teddybb )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "6768.116",     0x0000, 0x4000, 0x5939817e )	/* encrypted */
	ROM_LOAD( "6769.109",     0x4000, 0x4000, 0x14a98ddd )	/* encrypted */
	ROM_LOAD( "6770.96",      0x8000, 0x4000, 0x67b0c7c2 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "6776.62",      0x0000, 0x2000, 0xa0e5aca7 )
	ROM_LOAD( "6775.61",      0x2000, 0x2000, 0xcdb77e51 )
	ROM_LOAD( "6774.64",      0x4000, 0x2000, 0x0cab75c3 )
	ROM_LOAD( "6773.63",      0x6000, 0x2000, 0x0ef8d2cd )
	ROM_LOAD( "6772.66",      0x8000, 0x2000, 0xc33062b5 )
	ROM_LOAD( "6771.65",      0xA000, 0x2000, 0xc457e8c5 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "6735.117",     0x0000, 0x4000, 0x1be35a97 )
	ROM_LOAD( "6737.004",     0x4000, 0x4000, 0x6b53aa7a )
	ROM_LOAD( "6736.110",     0x8000, 0x4000, 0x565c25d0 )
	ROM_LOAD( "6738.005",     0xc000, 0x4000, 0xe116285f )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "6748.120",     0x0000, 0x2000, 0xc2a1b89d )
ROM_END

/* This is the first System 1 game to have extended ROM space */
ROM_START( hvymetal )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "epra6790.1",   0x00000, 0x8000, 0x59195bb9 )	/* encrypted */
	ROM_LOAD( "epra6789.2",   0x10000, 0x8000, 0x83e1d18a )
	ROM_LOAD( "epra6788.3",   0x18000, 0x8000, 0x6ecefd57 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6795.62",   0x00000, 0x4000, 0x58a3d038 )
	ROM_LOAD( "epr6796.61",   0x04000, 0x4000, 0xd8b08a55 )
	ROM_LOAD( "epr6793.64",   0x08000, 0x4000, 0x487407c2 )
	ROM_LOAD( "epr6794.63",   0x0c000, 0x4000, 0x89eb3793 )
	ROM_LOAD( "epr6791.66",   0x10000, 0x4000, 0xa7dcd042 )
	ROM_LOAD( "epr6792.65",   0x14000, 0x4000, 0xd0be5e33 )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr6778.117",  0x00000, 0x8000, 0x0af61aee )
	ROM_LOAD( "epr6777.110",  0x08000, 0x8000, 0x91d7a197 )
	ROM_LOAD( "epr6780.4",    0x10000, 0x8000, 0x55b31df5 )
	ROM_LOAD( "epr6779.5",    0x18000, 0x8000, 0xe03a2b28 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr6787.120",  0x0000, 0x8000, 0xb64ac7f0 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr7036.3",     0x0000, 0x0100, 0x146f16fb ) /* palette red component */
	ROM_LOAD( "pr7035.2",     0x0100, 0x0100, 0x50b201ed ) /* palette green component */
	ROM_LOAD( "pr7034.1",     0x0200, 0x0100, 0xdfb5f139 ) /* palette blue component */
ROM_END

ROM_START( myhero )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr6963b.116", 0x0000, 0x4000, 0x4daf89d4 )
	ROM_LOAD( "epr6964a.109", 0x4000, 0x4000, 0xc26188e5 )
	ROM_LOAD( "epr6965.96",   0x8000, 0x4000, 0x3cbbaf64 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6966.u62",  0x0000, 0x2000, 0x157f0401 )
	ROM_LOAD( "epr6961.u61",  0x2000, 0x2000, 0xbe53ce47 )
	ROM_LOAD( "epr6960.u64",  0x4000, 0x2000, 0xbd381baa )
	ROM_LOAD( "epr6959.u63",  0x6000, 0x2000, 0xbc04e79a )
	ROM_LOAD( "epr6958.u66",  0x8000, 0x2000, 0x714f2c26 )
	ROM_LOAD( "epr6958.u65",  0xA000, 0x2000, 0x80920112 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr6921.117",  0x0000, 0x4000, 0xf19e05a1 )
	ROM_LOAD( "epr6923.u04",  0x4000, 0x4000, 0x7988adc3 )
	ROM_LOAD( "epr6922.110",  0x8000, 0x4000, 0x37f77a78 )
	ROM_LOAD( "epr6924.u05",  0xc000, 0x4000, 0x42bdc8f6 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr69xx.120",  0x0000, 0x2000, 0x0039e1e9 )
ROM_END

ROM_START( myheroj )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "mhj_boot.01",  0x0000, 0x4000, 0xff54dcec )
	ROM_LOAD( "mhj_boot.02",  0x4000, 0x4000, 0x5c41eea8 )
	ROM_LOAD( "epr6965.96",   0x8000, 0x4000, 0x3cbbaf64 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "mhj_boot.11",  0x0000, 0x4000, 0xfe2039f4 )
	ROM_LOAD( "mhj_boot.10",  0x4000, 0x4000, 0x0ff682e8 )
	ROM_LOAD( "mhj_boot.09",  0x8000, 0x4000, 0x558b6926 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr6921.117",  0x0000, 0x4000, 0xf19e05a1 )
	ROM_LOAD( "epr6923.u04",  0x4000, 0x4000, 0x7988adc3 )
	ROM_LOAD( "epr6922.110",  0x8000, 0x4000, 0x37f77a78 )
	ROM_LOAD( "epr6924.u05",  0xc000, 0x4000, 0x42bdc8f6 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "mhj_boot.08",  0x0000, 0x2000, 0xaf467223 )
ROM_END

ROM_START( myherok )
	ROM_REGION(0x10000)	/* 64k for code */
	/* all the three program ROMs have bits 0-1 swapped */
	/* when decoded, they are identical to the Japanese version */
	ROM_LOAD( "ry-11.rom",    0x0000, 0x4000, 0x6f4c8ee5 )	/* encrypted */
	ROM_LOAD( "ry-09.rom",    0x4000, 0x4000, 0x369302a1 )	/* encrypted */
	ROM_LOAD( "ry-07.rom",    0x8000, 0x4000, 0xb8e9922e )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	/* all three gfx ROMs have address lines A4 and A5 swapped, also #1 and #3 */
	/* have data lines D0 and D6 swapped, while #2 has data lines D1 and D5 swapped. */
	ROM_LOAD( "ry-04.rom",    0x0000, 0x4000, 0xdfb75143 )
	ROM_LOAD( "ry-03.rom",    0x4000, 0x4000, 0xcf68b4a2 )
	ROM_LOAD( "ry-02.rom",    0x8000, 0x4000, 0xd100eaef )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr6921.117",  0x0000, 0x4000, 0xf19e05a1 )
	ROM_LOAD( "epr6923.u04",  0x4000, 0x4000, 0x7988adc3 )
	ROM_LOAD( "epr6922.110",  0x8000, 0x4000, 0x37f77a78 )
	ROM_LOAD( "epr6924.u05",  0xc000, 0x4000, 0x42bdc8f6 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "mhj_boot.08",  0x0000, 0x2000, 0xaf467223 )
ROM_END

void myherok_unmangle(void)
{
	int A;
	unsigned char *RAM;

	/* additionally to the usual protection, all the program ROMs have data lines */
	/* D0 and D1 swapped. */
	RAM = memory_region(Machine->drv->cpu[0].memory_region);
	for (A = 0;A < 0xc000;A++)
		RAM[A] = (RAM[A] & 0xfc) | ((RAM[A] & 1) << 1) | ((RAM[A] & 2) >> 1);

	/* the tile gfx ROMs are mangled as well: */
	RAM = Machine->memory_region[1];

	/* the first ROM has data lines D0 and D6 swapped. */
	for (A = 0x0000;A < 0x4000;A++)
		RAM[A] = (RAM[A] & 0xbe) | ((RAM[A] & 0x01) << 6) | ((RAM[A] & 0x40) >> 6);

	/* the second ROM has data lines D1 and D5 swapped. */
	for (A = 0x4000;A < 0x8000;A++)
		RAM[A] = (RAM[A] & 0xdd) | ((RAM[A] & 0x02) << 4) | ((RAM[A] & 0x20) >> 4);

	/* the third ROM has data lines D0 and D6 swapped. */
	for (A = 0x8000;A < 0xc000;A++)
		RAM[A] = (RAM[A] & 0xbe) | ((RAM[A] & 0x01) << 6) | ((RAM[A] & 0x40) >> 6);

	/* also, all three ROMs have address lines A4 and A5 swapped. */
	for (A = 0;A < 0xc000;A++)
	{
		int A1;
		unsigned char temp;

		A1 = (A & 0xffcf) | ((A & 0x0010) << 1) | ((A & 0x0020) >> 1);
		if (A < A1)
		{
			temp = RAM[A];
			RAM[A] = RAM[A1];
			RAM[A1] = temp;
		}
	}
}


ROM_START( shtngmst )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "epr7100",      0x00000, 0x8000, 0x45e64431 )
	ROM_LOAD( "epr7101",      0x10000, 0x8000, 0xebf5ff72 )
	ROM_LOAD( "epr7102",      0x18000, 0x8000, 0xc890a4ad )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr7040",      0x00000, 0x8000, 0xf30769fa )
	ROM_LOAD( "epr7041",      0x08000, 0x8000, 0xf3e273f9 )
	ROM_LOAD( "epr7042",      0x10000, 0x8000, 0x6841c917 )

	ROM_REGION(0x38000)	/* 224 for sprites data - PROBABLY WRONG! */
	ROM_LOAD( "epr7105",      0x00000, 0x8000, 0x13111729 )
	ROM_LOAD( "epr7104",      0x08000, 0x8000, 0x84a679c5 )
	ROM_LOAD( "epr7107",      0x10000, 0x8000, 0x8f50ea24 )
	ROM_LOAD( "epr7106",      0x18000, 0x8000, 0xae7ab7a2 )
	ROM_LOAD( "epr7109",      0x20000, 0x8000, 0x097f7481 )
	ROM_LOAD( "epr7108",      0x28000, 0x8000, 0x816180ac )
	ROM_LOAD( "epr7110",      0x30000, 0x8000, 0x5d1a5048 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr7043",      0x0000, 0x8000, 0x99a368ab )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "epr7113",      0x0000, 0x0100, 0x5c0e1360 ) /* palette red component */
	ROM_LOAD( "epr7112",      0x0100, 0x0100, 0x46fbd351 ) /* palette green component */
	ROM_LOAD( "epr7111",      0x0200, 0x0100, 0x8123b6b9 ) /* palette blue component */
ROM_END

ROM_START( chplft )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "7124.90",      0x00000, 0x8000, 0x678d5c41 )
	ROM_LOAD( "7125.91",      0x10000, 0x8000, 0xf5283498 )
	ROM_LOAD( "7126.92",      0x18000, 0x8000, 0xdbd192ab )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "7127.4",       0x00000, 0x8000, 0x1e708f6d )
	ROM_LOAD( "7128.5",       0x08000, 0x8000, 0xb922e787 )
	ROM_LOAD( "7129.6",       0x10000, 0x8000, 0xbd3b6e6e )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "7121.87",      0x00000, 0x8000, 0xf2b88f73 )
	ROM_LOAD( "7120.86",      0x08000, 0x8000, 0x517d7fd3 )
	ROM_LOAD( "7123.89",      0x10000, 0x8000, 0x8f16a303 )
	ROM_LOAD( "7122.88",      0x18000, 0x8000, 0x7c93f160 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "7130.126",     0x0000, 0x8000, 0x346af118 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr7119.20",    0x0000, 0x0100, 0xb2a8260f ) /* palette red component */
	ROM_LOAD( "pr7118.14",    0x0100, 0x0100, 0x693e20c7 ) /* palette green component */
	ROM_LOAD( "pr7117.8",     0x0200, 0x0100, 0x4124307e ) /* palette blue component */
ROM_END

ROM_START( chplftb )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "7152.90",      0x00000, 0x8000, 0xfe49d83e )
	ROM_LOAD( "7153.91",      0x10000, 0x8000, 0x48697666 )
	ROM_LOAD( "7154.92",      0x18000, 0x8000, 0x56d6222a )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "7127.4",       0x00000, 0x8000, 0x1e708f6d )
	ROM_LOAD( "7128.5",       0x08000, 0x8000, 0xb922e787 )
	ROM_LOAD( "7129.6",       0x10000, 0x8000, 0xbd3b6e6e )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "7121.87",      0x00000, 0x8000, 0xf2b88f73 )
	ROM_LOAD( "7120.86",      0x08000, 0x8000, 0x517d7fd3 )
	ROM_LOAD( "7123.89",      0x10000, 0x8000, 0x8f16a303 )
	ROM_LOAD( "7122.88",      0x18000, 0x8000, 0x7c93f160 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "7130.126",     0x0000, 0x8000, 0x346af118 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr7119.20",    0x0000, 0x0100, 0xb2a8260f ) /* palette red component */
	ROM_LOAD( "pr7118.14",    0x0100, 0x0100, 0x693e20c7 ) /* palette green component */
	ROM_LOAD( "pr7117.8",     0x0200, 0x0100, 0x4124307e ) /* palette blue component */
ROM_END

ROM_START( chplftbl )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "7124bl.90",    0x00000, 0x8000, 0x71a37932 )
	ROM_LOAD( "7125.91",      0x10000, 0x8000, 0xf5283498 )
	ROM_LOAD( "7126.92",      0x18000, 0x8000, 0xdbd192ab )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "7127.4",       0x00000, 0x8000, 0x1e708f6d )
	ROM_LOAD( "7128.5",       0x08000, 0x8000, 0xb922e787 )
	ROM_LOAD( "7129.6",       0x10000, 0x8000, 0xbd3b6e6e )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "7121.87",      0x00000, 0x8000, 0xf2b88f73 )
	ROM_LOAD( "7120.86",      0x08000, 0x8000, 0x517d7fd3 )
	ROM_LOAD( "7123.89",      0x10000, 0x8000, 0x8f16a303 )
	ROM_LOAD( "7122.88",      0x18000, 0x8000, 0x7c93f160 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "7130.126",     0x0000, 0x8000, 0x346af118 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr7119.20",    0x0000, 0x0100, 0xb2a8260f ) /* palette red component */
	ROM_LOAD( "pr7118.14",    0x0100, 0x0100, 0x693e20c7 ) /* palette green component */
	ROM_LOAD( "pr7117.8",     0x0200, 0x0100, 0x4124307e ) /* palette blue component */
ROM_END

ROM_START( fdwarrio )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "4d.116",       0x0000, 0x4000, 0x546d1bc7 )	/* encrypted */
	ROM_LOAD( "4d.109",       0x4000, 0x4000, 0xf1074ec3 )	/* encrypted */
	ROM_LOAD( "4d.96",        0x8000, 0x4000, 0x387c1e8f )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "4d.62",        0x0000, 0x2000, 0xf31b2e09 )
	ROM_LOAD( "4d.61",        0x2000, 0x2000, 0x5430e925 )
	ROM_LOAD( "4d.64",        0x4000, 0x2000, 0x9f442351 )
	ROM_LOAD( "4d.63",        0x6000, 0x2000, 0x633232bd )
	ROM_LOAD( "4d.66",        0x8000, 0x2000, 0x52bfa2ed )
	ROM_LOAD( "4d.65",        0xA000, 0x2000, 0xe9ba4658 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "4d.117",       0x0000, 0x4000, 0x436e4141 )
	ROM_LOAD( "4d.04",        0x4000, 0x4000, 0x8b7cecef )
	ROM_LOAD( "4d.110",       0x8000, 0x4000, 0x6ec5990a )
	ROM_LOAD( "4d.05",        0xc000, 0x4000, 0xf31a1e6a )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "4d.120",       0x0000, 0x2000, 0x5241c009 )
ROM_END

ROM_START( brain )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "brain.1",      0x00000, 0x8000, 0x2d2aec31 )
	ROM_LOAD( "brain.2",      0x10000, 0x8000, 0x810a8ab5 )
	ROM_LOAD( "brain.3",      0x18000, 0x8000, 0x9a225634 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "brain.62",     0x0000, 0x4000, 0x7dce2302 )
	ROM_LOAD( "brain.64",     0x4000, 0x4000, 0x7ce03fd3 )
	ROM_LOAD( "brain.66",     0x8000, 0x4000, 0xea54323f )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "brain.117",    0x00000, 0x8000, 0x92ff71a4 )
	ROM_LOAD( "brain.110",    0x08000, 0x8000, 0xa1b847ec )
	ROM_LOAD( "brain.4",      0x10000, 0x8000, 0xfd2ea53b )
	/* 18000-1ffff empty */

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "brain.120",    0x0000, 0x8000, 0xc7e50278 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "prom.3",       0x0000, 0x0100, 0x00000000 ) /* palette red component */
	ROM_LOAD( "prom.2",       0x0100, 0x0100, 0x00000000 ) /* palette green component */
	ROM_LOAD( "prom.1",       0x0200, 0x0100, 0x00000000 ) /* palette blue component */
ROM_END

ROM_START( wboy )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "b-1.bin",      0x0000, 0x4000, 0x51d27534 )	/* encrypted */
	ROM_LOAD( "b-2.bin",      0x4000, 0x4000, 0xe29d1cd1 )	/* encrypted */
	ROM_LOAD( "epr7491.96",   0x8000, 0x4000, 0x1f7d0efe )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6497.62",   0x0000, 0x2000, 0x08d609ca )
	ROM_LOAD( "epr7496.61",   0x2000, 0x2000, 0x6f61fdf1 )
	ROM_LOAD( "epr7495.64",   0x4000, 0x2000, 0x6a0d2c2d )
	ROM_LOAD( "epr7494.63",   0x6000, 0x2000, 0xa8e281c7 )
	ROM_LOAD( "epr7493.66",   0x8000, 0x2000, 0x89305df4 )
	ROM_LOAD( "epr7492.65",   0xA000, 0x2000, 0x60f806b1 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr7485.117",  0x0000, 0x4000, 0xc2891722 )
	ROM_LOAD( "epr7487.04",   0x4000, 0x4000, 0x2d3a421b )
	ROM_LOAD( "epr7486.110",  0x8000, 0x4000, 0x8d622c50 )
	ROM_LOAD( "epr7488.05",   0xc000, 0x4000, 0x007c2f1b )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "ic120_98.bin", 0x0000, 0x2000, 0x78ae1e7b )
ROM_END

ROM_START( wboy2 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "wb_1",         0x0000, 0x4000, 0xbd6fef49 )	/* encrypted */
	ROM_LOAD( "wb_2",         0x4000, 0x4000, 0x4081b624 )	/* encrypted */
	ROM_LOAD( "wb_3",         0x8000, 0x4000, 0xc48a0e36 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6497.62",   0x0000, 0x2000, 0x08d609ca )
	ROM_LOAD( "epr7496.61",   0x2000, 0x2000, 0x6f61fdf1 )
	ROM_LOAD( "epr7495.64",   0x4000, 0x2000, 0x6a0d2c2d )
	ROM_LOAD( "epr7494.63",   0x6000, 0x2000, 0xa8e281c7 )
	ROM_LOAD( "epr7493.66",   0x8000, 0x2000, 0x89305df4 )
	ROM_LOAD( "epr7492.65",   0xA000, 0x2000, 0x60f806b1 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr7485.117",  0x0000, 0x4000, 0xc2891722 )
	ROM_LOAD( "epr7487.04",   0x4000, 0x4000, 0x2d3a421b )
	ROM_LOAD( "epr7486.110",  0x8000, 0x4000, 0x8d622c50 )
	ROM_LOAD( "epr7488.05",   0xc000, 0x4000, 0x007c2f1b )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "ic120_98.bin", 0x0000, 0x2000, 0x78ae1e7b )
ROM_END

ROM_START( wboy3 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "epr7489.116",  0x0000, 0x4000, 0x130f4b70 )	/* encrypted */
	ROM_LOAD( "epr7490.109",  0x4000, 0x4000, 0x9e656733 )	/* encrypted */
	ROM_LOAD( "epr7491.96",   0x8000, 0x4000, 0x1f7d0efe )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6497.62",   0x0000, 0x2000, 0x08d609ca )
	ROM_LOAD( "epr7496.61",   0x2000, 0x2000, 0x6f61fdf1 )
	ROM_LOAD( "epr7495.64",   0x4000, 0x2000, 0x6a0d2c2d )
	ROM_LOAD( "epr7494.63",   0x6000, 0x2000, 0xa8e281c7 )
	ROM_LOAD( "epr7493.66",   0x8000, 0x2000, 0x89305df4 )
	ROM_LOAD( "epr7492.65",   0xA000, 0x2000, 0x60f806b1 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr7485.117",  0x0000, 0x4000, 0xc2891722 )
	ROM_LOAD( "epr7487.04",   0x4000, 0x4000, 0x2d3a421b )
	ROM_LOAD( "epr7486.110",  0x8000, 0x4000, 0x8d622c50 )
	ROM_LOAD( "epr7488.05",   0xc000, 0x4000, 0x007c2f1b )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epra7498.120", 0x0000, 0x2000, 0xc198205c )
ROM_END

ROM_START( wboy4 )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic129",        0x0000, 0x2000, 0x1bbb7354 )	/* encrypted */
	ROM_LOAD( "ic130",        0x2000, 0x2000, 0x21007413 )	/* encrypted */
	ROM_LOAD( "ic131",        0x4000, 0x2000, 0x44b30433 )	/* encrypted */
	ROM_LOAD( "ic132",        0x6000, 0x2000, 0xbb525a0b )	/* encrypted */
	ROM_LOAD( "ic133",        0x8000, 0x2000, 0x8379aa23 )
	ROM_LOAD( "ic134",        0xa000, 0x2000, 0xc767a5d7 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6497.62",   0x0000, 0x2000, 0x08d609ca )
	ROM_LOAD( "epr7496.61",   0x2000, 0x2000, 0x6f61fdf1 )
	ROM_LOAD( "epr7495.64",   0x4000, 0x2000, 0x6a0d2c2d )
	ROM_LOAD( "epr7494.63",   0x6000, 0x2000, 0xa8e281c7 )
	ROM_LOAD( "epr7493.66",   0x8000, 0x2000, 0x89305df4 )
	ROM_LOAD( "epr7492.65",   0xA000, 0x2000, 0x60f806b1 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr7485.117",  0x0000, 0x4000, 0xc2891722 )
	ROM_LOAD( "epr7487.04",   0x4000, 0x4000, 0x2d3a421b )
	ROM_LOAD( "epr7486.110",  0x8000, 0x4000, 0x8d622c50 )
	ROM_LOAD( "epr7488.05",   0xc000, 0x4000, 0x007c2f1b )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr7502",      0x0000, 0x2000, 0xc92484b3 )
ROM_END

ROM_START( wboyu )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic116_89.bin", 0x0000, 0x4000, 0x73d8cef0 )
	ROM_LOAD( "ic109_90.bin", 0x4000, 0x4000, 0x29546828 )
	ROM_LOAD( "ic096_91.bin", 0x8000, 0x4000, 0xc7145c2a )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6497.62",   0x0000, 0x2000, 0x08d609ca )
	ROM_LOAD( "epr7496.61",   0x2000, 0x2000, 0x6f61fdf1 )
	ROM_LOAD( "epr7495.64",   0x4000, 0x2000, 0x6a0d2c2d )
	ROM_LOAD( "epr7494.63",   0x6000, 0x2000, 0xa8e281c7 )
	ROM_LOAD( "epr7493.66",   0x8000, 0x2000, 0x89305df4 )
	ROM_LOAD( "epr7492.65",   0xA000, 0x2000, 0x60f806b1 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "ic117_85.bin", 0x0000, 0x4000, 0x1ee96ae8 )
	ROM_LOAD( "ic004_87.bin", 0x4000, 0x4000, 0x119735bb )
	ROM_LOAD( "ic110_86.bin", 0x8000, 0x4000, 0x26d0fac4 )
	ROM_LOAD( "ic005_88.bin", 0xc000, 0x4000, 0x2602e519 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "ic120_98.bin", 0x0000, 0x2000, 0x78ae1e7b )
ROM_END

ROM_START( wboy4u )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic129_02.bin", 0x0000, 0x2000, 0x32c4b709 )
	ROM_LOAD( "ic130_03.bin", 0x2000, 0x2000, 0x56463ede )
	ROM_LOAD( "ic131_04.bin", 0x4000, 0x2000, 0x775ed392 )
	ROM_LOAD( "ic132_05.bin", 0x6000, 0x2000, 0x7b922708 )
	ROM_LOAD( "ic133",        0x8000, 0x2000, 0x8379aa23 )
	ROM_LOAD( "ic134",        0xa000, 0x2000, 0xc767a5d7 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6497.62",   0x0000, 0x2000, 0x08d609ca )
	ROM_LOAD( "epr7496.61",   0x2000, 0x2000, 0x6f61fdf1 )
	ROM_LOAD( "epr7495.64",   0x4000, 0x2000, 0x6a0d2c2d )
	ROM_LOAD( "epr7494.63",   0x6000, 0x2000, 0xa8e281c7 )
	ROM_LOAD( "epr7493.66",   0x8000, 0x2000, 0x89305df4 )
	ROM_LOAD( "epr7492.65",   0xA000, 0x2000, 0x60f806b1 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr7485.117",  0x0000, 0x4000, 0xc2891722 )
	ROM_LOAD( "epr7487.04",   0x4000, 0x4000, 0x2d3a421b )
	ROM_LOAD( "epr7486.110",  0x8000, 0x4000, 0x8d622c50 )
	ROM_LOAD( "epr7488.05",   0xc000, 0x4000, 0x007c2f1b )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epra7498.120", 0x0000, 0x2000, 0xc198205c )
ROM_END

ROM_START( wbdeluxe )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "wbd1.bin",     0x0000, 0x2000, 0xa1bedbd7 )
	ROM_LOAD( "ic130_03.bin", 0x2000, 0x2000, 0x56463ede )
	ROM_LOAD( "wbd3.bin",     0x4000, 0x2000, 0x6fcdbd4c )
	ROM_LOAD( "ic132_05.bin", 0x6000, 0x2000, 0x7b922708 )
	ROM_LOAD( "wbd5.bin",     0x8000, 0x2000, 0xf6b02902 )
	ROM_LOAD( "wbd6.bin",     0xa000, 0x2000, 0x43df21fe )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr6497.62",   0x0000, 0x2000, 0x08d609ca )
	ROM_LOAD( "epr7496.61",   0x2000, 0x2000, 0x6f61fdf1 )
	ROM_LOAD( "epr7495.64",   0x4000, 0x2000, 0x6a0d2c2d )
	ROM_LOAD( "epr7494.63",   0x6000, 0x2000, 0xa8e281c7 )
	ROM_LOAD( "epr7493.66",   0x8000, 0x2000, 0x89305df4 )
	ROM_LOAD( "epr7492.65",   0xA000, 0x2000, 0x60f806b1 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "epr7485.117",  0x0000, 0x4000, 0xc2891722 )
	ROM_LOAD( "epr7487.04",   0x4000, 0x4000, 0x2d3a421b )
	ROM_LOAD( "epr7486.110",  0x8000, 0x4000, 0x8d622c50 )
	ROM_LOAD( "epr7488.05",   0xc000, 0x4000, 0x007c2f1b )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epra7498.120", 0x0000, 0x2000, 0xc198205c )
ROM_END

ROM_START( gardia )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "epr10255.1",   0x00000, 0x8000, 0x89282a6b )
	ROM_LOAD( "epr10254.2",   0x10000, 0x8000, 0x2826b6d8 )
	ROM_LOAD( "epr10253.3",   0x18000, 0x8000, 0x7911260f )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr10249.61",  0x0000, 0x4000, 0x4e0ad0f2 )
	ROM_LOAD( "epr10248.64",  0x4000, 0x4000, 0x3515d124 )
	ROM_LOAD( "epr10247.66",  0x8000, 0x4000, 0x541e1555 )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr10234.117", 0x00000, 0x8000, 0x8a6aed33 )
	ROM_LOAD( "epr10233.110", 0x08000, 0x8000, 0xc52784d3 )
	ROM_LOAD( "epr10236.04",  0x10000, 0x8000, 0xb35ab227 )
	ROM_LOAD( "epr10235.5",   0x18000, 0x8000, 0x006a3151 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr10243.120", 0x0000, 0x4000, 0x87220660 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "bprom.3",      0x0000, 0x0100, 0x8eee0f72 ) /* palette red component */
	ROM_LOAD( "bprom.2",      0x0100, 0x0100, 0x3e7babd7 ) /* palette green component */
	ROM_LOAD( "bprom.1",      0x0200, 0x0100, 0x371c44a6 ) /* palette blue component */
ROM_END

ROM_START( gardiab )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "gardiabl.5",   0x00000, 0x8000, 0x207f9cbb )
	ROM_LOAD( "gardiabl.6",   0x10000, 0x8000, 0xb2ed05dc )
	ROM_LOAD( "gardiabl.7",   0x18000, 0x8000, 0x0a490588 )

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "gardiabl.8",   0x0000, 0x4000, 0x367c9a17 )
	ROM_LOAD( "gardiabl.9",   0x4000, 0x4000, 0x1540fd30 )
	ROM_LOAD( "gardiabl.10",  0x8000, 0x4000, 0xe5c9af10 )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr10234.117", 0x00000, 0x8000, 0x8a6aed33 )
	ROM_LOAD( "epr10233.110", 0x08000, 0x8000, 0xc52784d3 )
	ROM_LOAD( "epr10236.04",  0x10000, 0x8000, 0xb35ab227 )
	ROM_LOAD( "epr10235.5",   0x18000, 0x8000, 0x006a3151 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr10243.120", 0x0000, 0x4000, 0x87220660 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "bprom.3",      0x0000, 0x0100, 0x8eee0f72 ) /* palette red component */
	ROM_LOAD( "bprom.2",      0x0100, 0x0100, 0x3e7babd7 ) /* palette green component */
	ROM_LOAD( "bprom.1",      0x0200, 0x0100, 0x371c44a6 ) /* palette blue component */
ROM_END

ROM_START( blockgal )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "bg.116",       0x0000, 0x4000, 0xa99b231a )	/* encrypted */
	ROM_LOAD( "bg.109",       0x4000, 0x4000, 0xa6b573d5 )	/* encrypted */
	/* 0x8000-0xbfff empty (was same as My Hero) */

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bg.62",        0x0000, 0x2000, 0x7e3ea4eb )
	ROM_LOAD( "bg.61",        0x2000, 0x2000, 0x4dd3d39d )
	ROM_LOAD( "bg.64",        0x4000, 0x2000, 0x17368663 )
	ROM_LOAD( "bg.63",        0x6000, 0x2000, 0x0c8bc404 )
	ROM_LOAD( "bg.66",        0x8000, 0x2000, 0x2b7dc4fa )
	ROM_LOAD( "bg.65",        0xA000, 0x2000, 0xed121306 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "bg.117",       0x0000, 0x4000, 0xe99cc920 )
	ROM_LOAD( "bg.04",        0x4000, 0x4000, 0x213057f8 )
	ROM_LOAD( "bg.110",       0x8000, 0x4000, 0x064c812c )
	ROM_LOAD( "bg.05",        0xc000, 0x4000, 0x02e0b040 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "bg.120",       0x0000, 0x2000, 0xd848faff )
ROM_END

ROM_START( blckgalb )
	ROM_REGION(0x18000)	/* 64k for code */
	ROM_LOAD( "ic62",         0x10000, 0x8000, 0x65c47676 )	/* decrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )				/* decrypted data */

	ROM_REGION_DISPOSE(0xc000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "bg.62",        0x0000, 0x2000, 0x7e3ea4eb )
	ROM_LOAD( "bg.61",        0x2000, 0x2000, 0x4dd3d39d )
	ROM_LOAD( "bg.64",        0x4000, 0x2000, 0x17368663 )
	ROM_LOAD( "bg.63",        0x6000, 0x2000, 0x0c8bc404 )
	ROM_LOAD( "bg.66",        0x8000, 0x2000, 0x2b7dc4fa )
	ROM_LOAD( "bg.65",        0xA000, 0x2000, 0xed121306 )

	ROM_REGION(0x10000)	/* 64k for sprites data */
	ROM_LOAD( "bg.117",       0x0000, 0x4000, 0xe99cc920 )
	ROM_LOAD( "bg.04",        0x4000, 0x4000, 0x213057f8 )
	ROM_LOAD( "bg.110",       0x8000, 0x4000, 0x064c812c )
	ROM_LOAD( "bg.05",        0xc000, 0x4000, 0x02e0b040 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "bg.120",       0x0000, 0x2000, 0xd848faff )
ROM_END

ROM_START( tokisens )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "epr10961.90",  0x00000, 0x8000, 0x1466b61d )
	ROM_LOAD( "epr10962.91",  0x10000, 0x8000, 0xa8479f91 )
	ROM_LOAD( "epr10963.92",  0x18000, 0x8000, 0xb7193b39 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr10964.4",   0x00000, 0x8000, 0x9013b85c )
	ROM_LOAD( "epr10965.5",   0x08000, 0x8000, 0xe4755cc6 )
	ROM_LOAD( "epr10966.6",   0x10000, 0x8000, 0x5bbfbdcc )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr10958.87",  0x00000, 0x8000, 0xfc2bcbd7 )
	ROM_LOAD( "epr10957.86",  0x08000, 0x8000, 0x4ec56860 )
	ROM_LOAD( "epr10960.89",  0x10000, 0x8000, 0x880e0d44 )
	ROM_LOAD( "epr10959.88",  0x18000, 0x8000, 0x4deda48f )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr10967.126", 0x0000, 0x8000, 0x97966bf2 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "bprom.20",     0x0000, 0x0100, 0x8eee0f72 ) /* palette red component */
	ROM_LOAD( "bprom.14",     0x0100, 0x0100, 0x3e7babd7 ) /* palette green component */
	ROM_LOAD( "bprom.8",      0x0200, 0x0100, 0x371c44a6 ) /* palette blue component */
ROM_END

ROM_START( wbml )
	ROM_REGION(0x40000)	/* 256k for code */
	ROM_LOAD( "wbml.01",      0x20000, 0x8000, 0x66482638 )	/* Unencrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )              /* Now load the operands in RAM */
	ROM_LOAD( "wbml.02",      0x30000, 0x8000, 0x48746bb6 )	/* Unencrypted opcodes */
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "wbml.03",      0x38000, 0x8000, 0xd57ba8aa )	/* Unencrypted opcodes */
	ROM_CONTINUE(             0x18000, 0x8000 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "wbml.08",      0x00000, 0x8000, 0xbbea6afe )
	ROM_LOAD( "wbml.09",      0x08000, 0x8000, 0x77567d41 )
	ROM_LOAD( "wbml.10",      0x10000, 0x8000, 0xa52ffbdd )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, 0xaf0b3972 )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, 0x277d8f1d )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, 0xf05ffc76 )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, 0xcedc9c61 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, 0x7a4ee585 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, 0x27057298 )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, 0x41e4d86b )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, 0x08d71954 )
ROM_END

ROM_START( wbmlj )
	ROM_REGION(0x20000)	/* 256k for code */
	ROM_LOAD( "epr11031.90",  0x00000, 0x8000, 0x497ebfb4 )	/* encrypted */
	ROM_LOAD( "epr11032.91",  0x10000, 0x8000, 0x9d03bdb2 )	/* encrypted */
	ROM_LOAD( "epr11033.92",  0x18000, 0x8000, 0x7076905c )	/* encrypted */

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr11034.4",   0x00000, 0x8000, 0x37a2077d )
	ROM_LOAD( "epr11035.5",   0x08000, 0x8000, 0xcdf2a21b )
	ROM_LOAD( "epr11036.6",   0x10000, 0x8000, 0x644687fa )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, 0xaf0b3972 )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, 0x277d8f1d )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, 0xf05ffc76 )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, 0xcedc9c61 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, 0x7a4ee585 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, 0x27057298 )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, 0x41e4d86b )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, 0x08d71954 )
ROM_END

ROM_START( wbmlj2 )
	ROM_REGION(0x20000)	/* 256k for code */
	ROM_LOAD( "ep11031a.90",  0x00000, 0x8000, 0xbd3349e5 )	/* encrypted */
	ROM_LOAD( "epr11032.91",  0x10000, 0x8000, 0x9d03bdb2 )	/* encrypted */
	ROM_LOAD( "epr11033.92",  0x18000, 0x8000, 0x7076905c )	/* encrypted */

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr11034.4",   0x00000, 0x8000, 0x37a2077d )
	ROM_LOAD( "epr11035.5",   0x08000, 0x8000, 0xcdf2a21b )
	ROM_LOAD( "epr11036.6",   0x10000, 0x8000, 0x644687fa )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, 0xaf0b3972 )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, 0x277d8f1d )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, 0xf05ffc76 )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, 0xcedc9c61 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, 0x7a4ee585 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, 0x27057298 )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, 0x41e4d86b )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, 0x08d71954 )
ROM_END

ROM_START( wbmlju )
	ROM_REGION(0x40000)	/* 256k for code */
	ROM_LOAD( "wbml.01",      0x20000, 0x8000, 0x66482638 )	/* Unencrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )              /* Now load the operands in RAM */
	ROM_LOAD( "m-6.bin",      0x30000, 0x8000, 0x8c08cd11 )	/* Unencrypted opcodes */
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "m-7.bin",      0x38000, 0x8000, 0x11881703 )	/* Unencrypted opcodes */
	ROM_CONTINUE(             0x18000, 0x8000 )

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr11034.4",   0x00000, 0x8000, 0x37a2077d )
	ROM_LOAD( "epr11035.5",   0x08000, 0x8000, 0xcdf2a21b )
	ROM_LOAD( "epr11036.6",   0x10000, 0x8000, 0x644687fa )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, 0xaf0b3972 )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, 0x277d8f1d )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, 0xf05ffc76 )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, 0xcedc9c61 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, 0x7a4ee585 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, 0x27057298 )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, 0x41e4d86b )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, 0x08d71954 )
ROM_END

ROM_START( dakkochn )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "epr11224.90",  0x00000, 0x8000, 0x9fb1972b )	/* encrypted */
	ROM_LOAD( "epr11225.91",  0x10000, 0x8000, 0xc540f9e2 )	/* encrypted */
	/* 18000-1ffff empty */

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr11226.4",   0x00000, 0x8000, 0x3dbc2f78 )
	ROM_LOAD( "epr11227.5",   0x08000, 0x8000, 0x34156e8d )
	ROM_LOAD( "epr11228.6",   0x10000, 0x8000, 0xfdd5323f )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr11221.87",  0x00000, 0x8000, 0xf9a44916 )
	ROM_LOAD( "epr11220.86",  0x08000, 0x8000, 0xfdd25d8a )
	ROM_LOAD( "epr11223.89",  0x10000, 0x8000, 0x538adc55 )
	ROM_LOAD( "epr11222.88",  0x18000, 0x8000, 0x33fab0b2 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr11229.126", 0x0000, 0x8000, 0xc11648d0 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr11219.20",   0x0000, 0x0100, 0x45e252d9 ) /* palette red component */
	ROM_LOAD( "pr11218.14",   0x0100, 0x0100, 0x3eda3a1b ) /* palette green component */
	ROM_LOAD( "pr11217.8",    0x0200, 0x0100, 0x49dbde88 ) /* palette blue component */
ROM_END

ROM_START( ufosensi )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "epr11661.90",  0x00000, 0x8000, 0xf3e394e2 )	/* encrypted */
	ROM_LOAD( "epr11662.91",  0x10000, 0x8000, 0x0c2e4120 )	/* encrypted */
	ROM_LOAD( "epr11663.92",  0x18000, 0x8000, 0x4515ebae )	/* encrypted */

	ROM_REGION_DISPOSE(0x18000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "epr11664.4",   0x00000, 0x8000, 0x1b1bc3d5 )
	ROM_LOAD( "epr11665.5",   0x08000, 0x8000, 0x3659174a )
	ROM_LOAD( "epr11666.6",   0x10000, 0x8000, 0x99dcc793 )

	ROM_REGION(0x20000)	/* 128k for sprites data */
	ROM_LOAD( "epr11658.87",  0x00000, 0x8000, 0x3b5a20f7 )
	ROM_LOAD( "epr11657.86",  0x08000, 0x8000, 0x010f81a9 )
	ROM_LOAD( "epr11660.89",  0x10000, 0x8000, 0xe1e2e7c5 )
	ROM_LOAD( "epr11659.88",  0x18000, 0x8000, 0x286c7286 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	ROM_LOAD( "epr11667.126", 0x0000, 0x8000, 0x110baba9 )

	ROM_REGIONX( 0x0300, REGION_PROMS )
	ROM_LOAD( "pr11656.20",   0x0000, 0x0100, 0x640740eb ) /* palette red component */
	ROM_LOAD( "pr11655.14",   0x0100, 0x0100, 0xa0c3fa77 ) /* palette green component */
	ROM_LOAD( "pr11654.8",    0x0200, 0x0100, 0xba624305 ) /* palette blue component */
ROM_END


static void wbml_decode(void)
{
	int A;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	for (A = 0x0000;A < 0x8000;A++)
	{
		ROM[A] = RAM[A+0x20000];
	}
}

static void blckgalb_decode(void)
{
	int A;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	for (A = 0x0000;A < 0x8000;A++)
	{
		ROM[A] = RAM[A+0x10000];
	}
}



static int starjack_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xc0e4],"\x53\x2e\x54",3) == 0)
			{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xc0e1],30);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xc0db] = RAM[0xc0e1];
                        RAM[0xc0dc] = RAM[0xc0e2];
                        RAM[0xc0dd] = RAM[0xc0e3];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void starjack_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
        osd_fwrite(f,&RAM[0xc0e1],30);
		osd_fclose(f);
		RAM[0xc0e4] = 0;
	}
}

static int starjacs_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xc106],"\x53\x2e\x54",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
            osd_fread(f,&RAM[0xc103],30);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xc0fc] = RAM[0xc103];
                        RAM[0xc0dd] = RAM[0xc104];
                        RAM[0xc0de] = RAM[0xc105];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void starjacs_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
        osd_fwrite(f,&RAM[0xc103],30);
		osd_fclose(f);
		RAM[0xc106] = 0;
	}
}

static int upndown_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xc93f],"\x01\x00\x00",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xc93f],(6*10)+3);
			osd_fclose(f);
		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void upndown_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xc93f],(6*10)+3);
		osd_fclose(f);
		RAM[0xc93f] = 0;
	}
}


static int mrviking_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xd300],"\x00\x00\x02",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xd300],21);
                        osd_fread(f,&RAM[0xd42c],21);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xc086] = RAM[0xd300];
                        RAM[0xc087] = RAM[0xd301];
                        RAM[0xc088] = RAM[0xd302];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void mrviking_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0xd300],21);
                osd_fwrite(f,&RAM[0xd42c],21);
		osd_fclose(f);
	}
}


static int flicky_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xe700],"\x00\x10\x00",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xe700],49);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xC0d5] = RAM[0xe700];
                        RAM[0xC0d6] = RAM[0xe701];
                        RAM[0xC0d7] = RAM[0xe702];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void flicky_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0xe700],49);
		osd_fclose(f);
	}
}


static int bullfgtj_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xd300],"\x00\x00\x02",3) == 0 &&
			memcmp(&RAM[0xd339],"\x48\x2e\x49",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xd300],6*10);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xC014] = RAM[0xD300];
                        RAM[0xC015] = RAM[0xD301];
                        RAM[0xC016] = RAM[0xD302];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void bullfgtj_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0xd300],6*10);
		osd_fclose(f);
	}
}


static int pitfall2_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xD302],"\x02\x00\x59\x41",4) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xD300],56);
			osd_fclose(f);

			RAM[0xC000] = RAM[0xD300];
			RAM[0xC001] = RAM[0xD301];
			RAM[0xC002] = RAM[0xD302];
			RAM[0xC003] = RAM[0xD303];
		}
		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void pitfall2_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xD300],56);
		osd_fclose(f);
	}
}


static int seganinj_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xEF2E],"\x54\x41\x43\x00",4) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xEF01],48);
			osd_fclose(f);

		}
		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void seganinj_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xEF01],48);
		osd_fclose(f);
		RAM[0xEF2E] = 0;
	}
}


static int imsorry_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xd300],"\x00\x20\x00",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xd300],0x16*10);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xC017] = RAM[0xD300];
                        RAM[0xC018] = RAM[0xD301];
                        RAM[0xC019] = RAM[0xD302];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void imsorry_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0xd300],0x16*10);
		osd_fclose(f);
	}
}

static int fdwarrio_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xd300],"\x00\x00\x02",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xd300],0x16*10);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xC017] = RAM[0xD300];
                        RAM[0xC018] = RAM[0xD301];
                        RAM[0xC019] = RAM[0xD302];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}



static int teddybb_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
        if (memcmp(&RAM[0xef03],"\x00\x10\x00",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0xef03],49);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
                        RAM[0xC578] = RAM[0xef05];
                        RAM[0xC579] = RAM[0xef04];
                        RAM[0xC57a] = RAM[0xef03];

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void teddybb_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0xef03],49);
		osd_fclose(f);
	}
}


static int myhero_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xd339],"\x59\x2E\x49\x00",4) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xD300],61);
			osd_fclose(f);

			/* copy the high score to the work RAM as well */
			RAM[0xC017] = RAM[0xD300];
			RAM[0xC018] = RAM[0xD301];
		}
		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void myhero_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xD300],61);
		osd_fclose(f);
		RAM[0xd339] = 0;
	}
}


static int wbdeluxe_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xC100],"\x20\x11\x20\x20",4) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			int i;


			osd_fread(f,&RAM[0xC100],320);
			osd_fclose(f);

			/* copy the high score to the screen as well */
			for (i = 0; i < 6; i++) /* 6 digits are stored, one per byte */
			{
				if (RAM[0xC102 + i] == 0x20)  /* spaces */
					RAM[0xE858 + i * 2] = 0x01;
				else                          /* digits */
					RAM[0xE858 + i * 2] = RAM[0xC102 + i] - 0x30 + 0x10;
			}
		}
		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void wbdeluxe_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xC100],320);
		osd_fclose(f);
		RAM[0xC100] = 0;
	}
}


static int chplft_hiload(void)
{
	void *f;
	unsigned char *choplifter_ram = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&choplifter_ram[0xEF00],"\x00\x00\x05\x00",4) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&choplifter_ram[0xEF00],49);
			osd_fclose(f);
		}
		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void chplft_hisave(void)
{
	void *f;
	unsigned char *choplifter_ram = memory_region(Machine->drv->cpu[0].memory_region);


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&choplifter_ram[0xEF00],49);
		osd_fclose(f);
	}
}


static int wbml_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xc17b],"\x00\x00\x03",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xc179],8);
			osd_fclose(f);
		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void wbml_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xc179],8);
		osd_fclose(f);
	}
}

static int wboy_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xE820],"\x1c\x08\x1a",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xc100],320);
			osd_fclose(f);

			/* copy top score to the top of the screen */
			if (memcmp(&RAM[0xC102],"\x20",1 != 0))
			RAM[0xE858]=(RAM[0xc102]-32);
			if (memcmp(&RAM[0xC103],"\x20",1 != 0))
			RAM[0xE85A]=(RAM[0xc103]-32);
			if (memcmp(&RAM[0xC104],"\x20",1 != 0))
			RAM[0xE85C]=(RAM[0xc104]-32);
			if (memcmp(&RAM[0xC105],"\x20",1 != 0))
			RAM[0xE85E]=(RAM[0xc105]-32);
			if (memcmp(&RAM[0xC106],"\x20",1 != 0))
			RAM[0xE860]=(RAM[0xc106]-32);
			if (memcmp(&RAM[0xC107],"\x20",1 != 0))
			RAM[0xE862]=(RAM[0xc107]-32);

		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void wboy_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xc100],320);
		osd_fclose(f);
		RAM[0xE820] = 0 ;
	}
}

static int regulus_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xce5b],"\x4a\x4a\x4a",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xc0e1],3);
			osd_fread(f,&RAM[0xcd01],30);
			osd_fread(f,&RAM[0xce40],30);
			osd_fclose(f);

		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void regulus_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xc0e1],3);
		osd_fwrite(f,&RAM[0xcd01],30);
		osd_fwrite(f,&RAM[0xce40],30);
		osd_fclose(f);

	}
}

static int swat_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xE9FC],"\x45\x08\x01",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
/* load the high score table */
			int hi;
			osd_fread(f,&RAM[0xd300],60);
			osd_fclose(f);

/* load the top score into its place in ram */
			RAM[0xC014]=RAM[0xd300];
			RAM[0xC015]=RAM[0xd301];
			RAM[0xC016]=RAM[0xd302];

/* force the high score to display in video ram */
			hi = (RAM[0xd300] & 0x0f) +
				 (RAM[0xd300] >> 4) * 10 +
				 (RAM[0xd301] & 0x0f) * 100 +
				 (RAM[0xd301] >> 4) * 1000 +
				 (RAM[0xd302] & 0x0f) * 10000 +
				 (RAM[0xd302] >> 4) * 100000;

			if (hi > 0)
				RAM[0xEC3A] = (RAM[0xD300] & 0x0F)+'0';
			if (hi >= 10)
				RAM[0xEBFA] = (RAM[0xD300] >> 4)+'0';
			if (hi >= 100)
				RAM[0xEBBA] = (RAM[0xD301] & 0x0F)+'0';
			if (hi >= 1000)
				RAM[0xEB7A] = (RAM[0xD301] >> 4)+'0';
			if (hi >= 10000)
				RAM[0xEB3A] = (RAM[0xD302] & 0x0F)+'0';
			if (hi >= 100000)
				RAM[0xEAFA] = (RAM[0xD302] >> 4)+'0';


		}
		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void swat_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xd300],60);
		osd_fclose(f);
		/* dirty area we check against so that it loads high score on reset */
		RAM[0xE9FC] = 0;
	}
}

static int hvymetal_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xD335],"\x4d\x49\x59",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
				osd_fread(f,&RAM[0xd300],56);
				RAM[0xc00c] = RAM[0xd300];
				RAM[0xc00d] = RAM[0xd301];
				RAM[0xc00e] = RAM[0xd302];
				osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void hvymetal_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xd300],56);
		osd_fclose(f);
		RAM[0xD335] = 0;
	}
}

static int brain_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xD339],"\x59\x2E\x49",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
				osd_fread(f,&RAM[0xd300],60);
				RAM[0xc017] = RAM[0xd300];
				RAM[0xc018] = RAM[0xd301];
				RAM[0xc019] = RAM[0xd302];
				osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void brain_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xd300],60);
		osd_fclose(f);
		RAM[0xD339] = 0;
	}
}

static int tokisens_hiload(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xC09A],"\x02\x42\x41",3) == 0)
	{
		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
				osd_fread(f,&RAM[0xc04D],81);
				RAM[0xc0A3] = RAM[0xC04F];
				RAM[0xc0A4] = RAM[0xC050];
				RAM[0xc0A5] = RAM[0xC051];
				osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void tokisens_hisave(void)
{
	void *f;
	unsigned char *RAM = memory_region(Machine->drv->cpu[0].memory_region);

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xc04D],81);
		osd_fclose(f);
		RAM[0xC09A] = 0;
	}
}

#define BASE_CREDITS "Jarek Parchanski\nNicola Salmoria\nMirko Buffoni\nRoberto Ventura (hardware info)"

struct GameDriver driver_starjack =
{
	__FILE__,
	0,
	"starjack",
	"Star Jacker (Sega)",
	"1983",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_small_machine_driver,
	0,

	rom_starjack,
	0, 0,
	0,
	0,

	input_ports_starjack,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	starjack_hiload, starjack_hisave
};

struct GameDriver driver_starjacs =
{
	__FILE__,
	&driver_starjack,
	"starjacs",
	"Star Jacker (Stern)",
	"1983",
	"Stern",
	BASE_CREDITS,
	0,
	&system1_small_machine_driver,
	0,

	rom_starjacs,
	0, 0,
	0,
	0,

	input_ports_starjacs,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	starjacs_hiload, starjacs_hisave
};

struct GameDriver driver_regulus =
{
	__FILE__,
	0,
	"regulus",
	"Regulus",
	"1983",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_regulus,
	0, regulus_decode,
	0,
	0,

	input_ports_regulus,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	regulus_hiload, regulus_hisave
};

struct GameDriver driver_regulusu =
{
	__FILE__,
	&driver_regulus,
	"regulusu",
	"Regulus (not encrypted)",
	"1983",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_regulusu,
	0, 0,
	0,
	0,

	input_ports_regulus,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	regulus_hiload, regulus_hisave
};

struct GameDriver driver_upndown =
{
	__FILE__,
	0,
	"upndown",
	"Up'n Down",
	"1983",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_upndown,
	0, 0,
	0,
	0,

	input_ports_upndown,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	upndown_hiload, upndown_hisave
};

struct GameDriver driver_mrviking =
{
	__FILE__,
	0,
	"mrviking",
	"Mister Viking",
	"1984",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_small_machine_driver,
	0,

	rom_mrviking,
	0, mrviking_decode,
	0,
	0,

	input_ports_mrviking,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	mrviking_hiload, mrviking_hisave
};

struct GameDriver driver_swat =
{
	__FILE__,
	0,
	"swat",
	"SWAT",
	"1984",
	"Coreland / Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_swat,
	0, swat_decode,
	0,
	0,

	input_ports_swat,

	0, 0, 0,
	ORIENTATION_ROTATE_270,
	swat_hiload, swat_hisave
};

struct GameDriver driver_flicky =
{
	__FILE__,
	0,
	"flicky",
	"Flicky (set 1)",
	"1984",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_flicky,
	0, flicky_decode,
	0,
	0,

	input_ports_flicky,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	flicky_hiload, flicky_hisave
};

struct GameDriver driver_flicky2 =
{
	__FILE__,
	&driver_flicky,
	"flicky2",
	"Flicky (set 2)",
	"1984",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_flicky2,
	0, flicky_decode,
	0,
	0,

	input_ports_flicky,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	flicky_hiload, flicky_hisave
};

struct GameDriver driver_bullfgtj =
{
	__FILE__,
	0,
	"bullfgtj",
	"The Tougyuu (Japan)",	/* Bull Fight */
	"1984",
	"Sega / Coreland",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_bullfgtj,
	0, bullfgtj_decode,
	0,
	0,

	input_ports_bullfgtj,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	bullfgtj_hiload, bullfgtj_hisave
};

struct GameDriver driver_pitfall2 =
{
	__FILE__,
	0,
	"pitfall2",
	"Pitfall II",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&pitfall2_machine_driver,
	0,

	rom_pitfall2,
	0, pitfall2_decode,
	0,
	0,

	input_ports_pitfall2,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	pitfall2_hiload, pitfall2_hisave
};

struct GameDriver driver_pitfallu =
{
	__FILE__,
	&driver_pitfall2,
	"pitfallu",
	"Pitfall II (not encrypted)",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&pitfall2_machine_driver,
	0,

	rom_pitfallu,
	0, 0,
	0,
	0,

	input_ports_pitfallu,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	pitfall2_hiload, pitfall2_hisave
};

struct GameDriver driver_seganinj =
{
	__FILE__,
	0,
	"seganinj",
	"Sega Ninja",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_seganinj,
	0, seganinj_decode,
	0,
	0,

	input_ports_seganinj,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	seganinj_hiload, seganinj_hisave
};

struct GameDriver driver_seganinu =
{
	__FILE__,
	&driver_seganinj,
	"seganinu",
	"Sega Ninja (not encrypted)",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_seganinu,
	0, 0,
	0,
	0,

	input_ports_seganinj,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	seganinj_hiload, seganinj_hisave
};

struct GameDriver driver_nprinces =
{
	__FILE__,
	&driver_seganinj,
	"nprinces",
	"Ninja Princess",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_nprinces,
	0, nprinces_decode,
	0,
	0,

	input_ports_seganinj,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	seganinj_hiload, seganinj_hisave
};

struct GameDriver driver_nprincsu =
{
	__FILE__,
	&driver_seganinj,
	"nprincsu",
	"Ninja Princess (not encrypted)",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_nprincsu,
	0, 0,
	0,
	0,

	input_ports_seganinj,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	seganinj_hiload, seganinj_hisave
};

struct GameDriver driver_nprincsb =
{
	__FILE__,
	&driver_seganinj,
	"nprincsb",
	"Ninja Princess (bootleg?)",
	"1985",
	"bootleg?",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_nprincsb,
	0, flicky_decode,
	0,
	0,

	input_ports_seganinj,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	seganinj_hiload, seganinj_hisave
};

struct GameDriver driver_imsorry =
{
	__FILE__,
	0,
	"imsorry",
	"I'm Sorry (US)",
	"1985",
	"Coreland / Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_imsorry,
	0, imsorry_decode,
	0,
	0,

	input_ports_imsorry,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	imsorry_hiload, imsorry_hisave
};

struct GameDriver driver_imsorryj =
{
	__FILE__,
	&driver_imsorry,
	"imsorryj",
	"I'm Sorry (Japan)",
	"1985",
	"Coreland / Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_imsorryj,
	0, imsorry_decode,
	0,
	0,

	input_ports_imsorry,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	imsorry_hiload, imsorry_hisave
};

struct GameDriver driver_teddybb =
{
	__FILE__,
	0,
	"teddybb",
	"TeddyBoy Blues",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_teddybb,
	0, teddybb_decode,
	0,
	0,

	input_ports_teddybb,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	teddybb_hiload, teddybb_hisave
};

struct GameDriver driver_hvymetal =
{
	__FILE__,
	0,
	"hvymetal",
	"Heavy Metal",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&hvymetal_machine_driver,
	0,

	rom_hvymetal,
	0, hvymetal_decode,
	0,
	0,

	input_ports_hvymetal,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	hvymetal_hiload, hvymetal_hisave
};

struct GameDriver driver_myhero =
{
	__FILE__,
	0,
	"myhero",
	"My Hero (US)",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_myhero,
	0, 0,
	0,
	0,

	input_ports_myhero,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	myhero_hiload, myhero_hisave
};

struct GameDriver driver_myheroj =
{
	__FILE__,
	&driver_myhero,
	"myheroj",
	"Seishun Scandal (Japan)",
	"1985",
	"Coreland / Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_myheroj,
	0, myheroj_decode,
	0,
	0,

	input_ports_myhero,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	myhero_hiload, myhero_hisave
};

struct GameDriver driver_myherok =
{
	__FILE__,
	&driver_myhero,
	"myherok",
	"My Hero (Korea)",
	"1985",
	"Coreland / Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_myherok,
	myherok_unmangle, myheroj_decode,	/* additional data and address line scrambling */
	0,
	0,

	input_ports_myhero,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	myhero_hiload, myhero_hisave
};

/* not working due to 8751 protection */
struct GameDriver driver_shtngmst =
{
	__FILE__,
	0,
	"shtngmst",
	"Shooting Master",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&chplft_machine_driver,
	0,

	rom_shtngmst,
	0, 0,
	0,
	0,

	input_ports_chplft,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NOT_WORKING,
	0, 0
};

/* not working due to 8751 protection */
struct GameDriver driver_chplft =
{
	__FILE__,
	0,
	"chplft",
	"Choplifter",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&chplft_machine_driver,
	0,

	rom_chplft,
	0, 0,
	0,
	0,

	input_ports_chplft,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NOT_WORKING,
	chplft_hiload, chplft_hisave
};

struct GameDriver driver_chplftb =
{
	__FILE__,
	&driver_chplft,
	"chplftb",
	"Choplifter (alternate)",
	"1985",
	"Sega",
	BASE_CREDITS,
	0,
	&chplft_machine_driver,
	0,

	rom_chplftb,
	0, 0,
	0,
	0,

	input_ports_chplft,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	chplft_hiload, chplft_hisave
};

struct GameDriver driver_chplftbl =
{
	__FILE__,
	&driver_chplft,
	"chplftbl",
	"Choplifter (bootleg)",
	"1985",
	"bootleg",
	BASE_CREDITS,
	0,
	&chplft_machine_driver,
	0,

	rom_chplftbl,
	0, 0,
	0,
	0,

	input_ports_chplft,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	chplft_hiload, chplft_hisave
};

struct GameDriver driver_fdwarrio =
{
	__FILE__,
	0,
	"4dwarrio",
	"4-D Warriors",
	"1985",
	"Coreland / Sega",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_fdwarrio,
	0, fdwarrio_decode,
	0,
	0,

	input_ports_fdwarrio,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	fdwarrio_hiload, imsorry_hisave
};

struct GameDriver driver_brain =
{
	__FILE__,
	0,
	"brain",
	"Brain",
	"1986",
	"Coreland / Sega",
	BASE_CREDITS,
	0,
	&brain_machine_driver,
	0,

	rom_brain,
	0, 0,
	0,
	0,

	input_ports_brain,

	0,0,0,
	ORIENTATION_DEFAULT,
	brain_hiload, brain_hisave
};

struct GameDriver driver_wboy =
{
	__FILE__,
	0,
	"wboy",
	"Wonder Boy (set 1)",
	"1986",
	"Sega (Escape license)",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_wboy,
	0, hvymetal_decode,
	0,
	0,

	input_ports_wboy,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wboy_hiload, wboy_hisave
};

struct GameDriver driver_wboy2 =
{
	__FILE__,
	&driver_wboy,
	"wboy2",
	"Wonder Boy (set 2)",
	"1986",
	"Sega (Escape license)",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_wboy2,
	0, hvymetal_decode,
	0,
	0,

	input_ports_wboy,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wboy_hiload, wboy_hisave
};

struct GameDriver driver_wboy3 =
{
	__FILE__,
	&driver_wboy,
	"wboy3",
	"Wonder Boy (set 3)",
	"????",
	"?????",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_wboy3,
	0, wboy3_decode,
	0,
	0,

	input_ports_wboy,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NOT_WORKING,
	wboy_hiload, wboy_hisave
};

struct GameDriver driver_wboy4 =
{
	__FILE__,
	&driver_wboy,
	"wboy4",
	"Wonder Boy (set 4)",
	"1986",
	"Sega (Escape license)",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_wboy4,
	0, wboy4_decode,
	0,
	0,

	input_ports_wboy,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wboy_hiload, wboy_hisave
};

struct GameDriver driver_wboyu =
{
	__FILE__,
	&driver_wboy,
	"wboyu",
	"Wonder Boy (not encrypted)",
	"1986",
	"Sega (Escape license)",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_wboyu,
	0, 0,
	0,
	0,

	input_ports_wboyu,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wboy_hiload, wboy_hisave
};

struct GameDriver driver_wboy4u =
{
	__FILE__,
	&driver_wboy,
	"wboy4u",
	"Wonder Boy (set 4 not encrypted)",
	"1986",
	"Sega (Escape license)",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_wboy4u,
	0, 0,
	0,
	0,

	input_ports_wboy,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wboy_hiload, wboy_hisave
};

struct GameDriver driver_wbdeluxe =
{
	__FILE__,
	&driver_wboy,
	"wbdeluxe",
	"Wonder Boy Deluxe",
	"1986",
	"Sega (Escape license)",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_wbdeluxe,
	0, 0,
	0,
	0,

	input_ports_wbdeluxe,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wbdeluxe_hiload, wbdeluxe_hisave
};

struct GameDriver driver_gardia =
{
	__FILE__,
	0,
	"gardia",
	"Gardia",
	"1986",
	"Sega / Coreland",
	BASE_CREDITS,
	0,
	&brain_machine_driver,
	0,

	rom_gardia,
	0, gardia_decode,
	0,
	0,

	input_ports_wboy,

	0, 0, 0,
	ORIENTATION_ROTATE_270 | GAME_NOT_WORKING,
	0, 0
};

struct GameDriver driver_gardiab =
{
	__FILE__,
	&driver_gardia,
	"gardiab",
	"Gardia (bootleg)",
	"1986",
	"bootleg",
	BASE_CREDITS,
	0,
	&brain_machine_driver,
	0,

	rom_gardiab,
	0, gardia_decode,
	0,
	0,

	input_ports_wboy,

	0, 0, 0,
	ORIENTATION_ROTATE_270 | GAME_NOT_WORKING,
	0, 0
};

struct GameDriver driver_blockgal =
{
	__FILE__,
	0,
	"blockgal",
	"Block Gal",
	"1987",
	"Sega / Vic Tokai",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_blockgal,
	0, 0,
	0,
	0,

	input_ports_blockgal,

	0, 0, 0,
	ORIENTATION_ROTATE_90 | GAME_NOT_WORKING,
	0, 0
};

struct GameDriver driver_blckgalb =
{
	__FILE__,
	&driver_blockgal,
	"blckgalb",
	"Block Gal (bootleg)",
	"1987",
	"bootleg",
	BASE_CREDITS,
	0,
	&system1_machine_driver,
	0,

	rom_blckgalb,
	0, blckgalb_decode,
	0,
	0,

	input_ports_blockgal,

	0, 0, 0,
	ORIENTATION_ROTATE_90 | GAME_NOT_WORKING,
	0, 0
};

struct GameDriver driver_tokisens =
{
	__FILE__,
	0,
	"tokisens",
	"Toki no Senshi - Chrono Soldier",
	"1987",
	"Sega",
	BASE_CREDITS,
	0,
	&wbml_machine_driver,
	0,

	rom_tokisens,
	0, 0,
	0,
	0,

	input_ports_tokisens,

	0, 0, 0,
	ORIENTATION_ROTATE_90,
	tokisens_hiload,tokisens_hisave
};

struct GameDriver driver_wbml =
{
	__FILE__,
	0,
	"wbml",
	"Wonder Boy in Monster Land",
	"1987",
	"bootleg",
	BASE_CREDITS,
	0,
	&wbml_machine_driver,
	0,

	rom_wbml,
	0, wbml_decode,
	0,
	0,

	input_ports_wbml,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wbml_hiload, wbml_hisave
};

struct GameDriver driver_wbmlj =
{
	__FILE__,
	&driver_wbml,
	"wbmlj",
	"Wonder Boy in Monster Land (Japan set 1)",
	"1987",
	"Sega / Westone",
	BASE_CREDITS,
	0,
	&wbml_machine_driver,
	0,

	rom_wbmlj,
	0, 0,
	0,
	0,

	input_ports_wbml,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NOT_WORKING,
	wbml_hiload, wbml_hisave
};

struct GameDriver driver_wbmlj2 =
{
	__FILE__,
	&driver_wbml,
	"wbmlj2",
	"Wonder Boy in Monster Land (Japan set 2)",
	"1987",
	"Sega / Westone",
	BASE_CREDITS,
	0,
	&wbml_machine_driver,
	0,

	rom_wbmlj2,
	0, 0,
	0,
	0,

	input_ports_wbml,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NOT_WORKING,
	wbml_hiload, wbml_hisave
};

struct GameDriver driver_wbmlju =
{
	__FILE__,
	&driver_wbml,
	"wbmlju",
	"Wonder Boy in Monster Land (Japan not encrypted)",
	"1987",
	"Sega / Westone",
	BASE_CREDITS,
	0,
	&wbml_machine_driver,
	0,

	rom_wbmlju,
	0, wbml_decode,
	0,
	0,

	input_ports_wbml,

	0, 0, 0,
	ORIENTATION_DEFAULT,
	wbml_hiload, wbml_hisave
};

struct GameDriver driver_dakkochn =
{
	__FILE__,
	0,
	"dakkochn",
	"DakkoChan Jansoh",
	"1987",
	"Sega",
	BASE_CREDITS,
	0,
	&chplft_machine_driver,
	0,

	rom_dakkochn,
	0, 0,
	0,
	0,

	input_ports_chplft,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NOT_WORKING,
	0, 0
};

struct GameDriver driver_ufosensi =
{
	__FILE__,
	0,
	"ufosensi",
	"Ufo Senshi Yohko Chan",
	"1988",
	"Sega",
	BASE_CREDITS,
	0,
	&chplft_machine_driver,
	0,

	rom_ufosensi,
	0, 0,
	0,
	0,

	input_ports_chplft,

	0, 0, 0,
	ORIENTATION_DEFAULT | GAME_NOT_WORKING,
	0, 0
};
