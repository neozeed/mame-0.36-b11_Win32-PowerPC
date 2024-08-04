/***************************************************************************

	Atari Audio Board II Interface

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"


void atarijsa_init(int cpunum, int inputport, int testport, int testmask);
void atarijsa_reset(void);


extern struct MemoryReadAddress atarijsa1_readmem[];
extern struct MemoryWriteAddress atarijsa1_writemem[];
extern struct MemoryReadAddress atarijsa2_readmem[];
extern struct MemoryWriteAddress atarijsa2_writemem[];
extern struct MemoryReadAddress atarijsa3_readmem[];
extern struct MemoryWriteAddress atarijsa3_writemem[];

extern struct TMS5220interface atarijsa_tms5220_interface;
extern struct YM2151interface atarijsa_ym2151_interface_mono;
extern struct YM2151interface atarijsa_ym2151_interface_stereo;
extern struct POKEYinterface atarijsa_pokey_interface;
extern struct OKIM6295interface atarijsa_okim6295_interface_2;
extern struct OKIM6295interface atarijsa_okim6295_interface_3;
extern struct OKIM6295interface atarijsa_okim6295_interface_4;
extern struct OKIM6295interface atarijsa_okim6295_interface_5;
extern struct OKIM6295interface atarijsa_okim6295_interface_6;


/* Used by Xybots, Blasteroids, Badlands(?) */
#define JSA_I_STEREO										\
	SOUND_SUPPORTS_STEREO,0,0,0,							\
	{														\
		{													\
			SOUND_YM2151, 									\
			&atarijsa_ym2151_interface_stereo				\
		}													\
	}

/* Used by Toobin', Vindicators */
#define JSA_I_STEREO_WITH_POKEY								\
	SOUND_SUPPORTS_STEREO,0,0,0,							\
	{														\
		{													\
			SOUND_YM2151, 									\
			&atarijsa_ym2151_interface_stereo				\
		},													\
		{													\
			SOUND_POKEY, 									\
			&atarijsa_pokey_interface 						\
		}													\
	}

/* Used by Escape from the Planet of the Robot Monsters */
#define JSA_I_STEREO_WITH_SPEECH							\
	SOUND_SUPPORTS_STEREO,0,0,0,							\
	{														\
		{													\
			SOUND_YM2151, 									\
			&atarijsa_ym2151_interface_stereo				\
		},													\
		{													\
			SOUND_TMS5220, 									\
			&atarijsa_tms5220_interface 					\
		}													\
	}

/* Used by Cyberball 2072, Skull & Crossbones, ThunderJaws, Hydra, Pit Fighter */
#define JSA_II_MONO(x)										\
	0,0,0,0,												\
	{														\
		{													\
			SOUND_YM2151, 									\
			&atarijsa_ym2151_interface_mono					\
		},													\
		{													\
			SOUND_OKIM6295,									\
			&atarijsa_okim6295_interface_##x				\
		}													\
	}

/* Used by Guardians of the 'Hood, Road Riot 4WD(?), Moto Frenzy */
#define JSA_III_MONO(x)										\
	0,0,0,0,												\
	{														\
		{													\
			SOUND_YM2151, 									\
			&atarijsa_ym2151_interface_mono					\
		},													\
		{													\
			SOUND_OKIM6295,									\
			&atarijsa_okim6295_interface_##x				\
		}													\
	}


/* Used by Off the Wall */
#define JSA_III_MONO_NO_SPEECH								\
	0,0,0,0,												\
	{														\
		{													\
			SOUND_YM2151, 									\
			&atarijsa_ym2151_interface_mono					\
		}													\
	}


/* Common CPU definitions */
#define JSA_I_CPU(mem_region)								\
		CPU_M6502,											\
		1789790,											\
		mem_region,											\
		atarijsa1_readmem,atarijsa1_writemem,0,0,			\
		0,0,												\
		atarigen_6502_irq_gen,250

#define JSA_II_CPU(mem_region)								\
		CPU_M6502,											\
		1789790,											\
		mem_region,											\
		atarijsa2_readmem,atarijsa2_writemem,0,0,			\
		0,0,												\
		atarigen_6502_irq_gen,250

#define JSA_III_CPU(mem_region)								\
		CPU_M6502,											\
		1789790,											\
		mem_region,											\
		atarijsa3_readmem,atarijsa3_writemem,0,0,			\
		0,0,												\
		atarigen_6502_irq_gen,250



/* Board-specific port definitions */
#define JSA_I_PORT											\
	PORT_START												\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* speech chip ready */\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

/* used by Xybots */
#define JSA_I_PORT_SWAPPED									\
	PORT_START												\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* speech chip ready */\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

#define JSA_II_PORT											\
	PORT_START												\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

#define JSA_III_PORT										\
	PORT_START												\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )	/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

