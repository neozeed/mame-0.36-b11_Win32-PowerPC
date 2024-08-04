#include "driver.h"
#include <ctype.h>


static struct RunningMachine machine;
struct RunningMachine *Machine = &machine;
static const struct GameDriver *gamedrv;
static const struct MachineDriver *drv;

/* Variables to hold the status of various game options */
struct GameOptions	options;

FILE *errorlog;
void *record;   /* for -record */
void *playback; /* for -playback */
int mame_debug; /* !0 when -debug option is specified */
int No_FM;

int bailing;	/* set to 1 if the startup is aborted to prevent multiple error messages */

static int settingsloaded;

int bitmap_dirty;	/* set by osd_clearbitmap() */

unsigned char *ROM;

/* Used in vh_open */
extern unsigned char *spriteram,*spriteram_2;
extern unsigned char *buffered_spriteram,*buffered_spriteram_2;
extern int spriteram_size,spriteram_2_size;

int init_machine(void);
void shutdown_machine(void);
int run_machine(void);


#ifdef MAME_DEBUG
static int validitychecks(void)
{
	int i,j;

	for (i = 0;drivers[i];i++)
	{
		const struct RomModule *romp;

		if (drivers[i]->clone_of == drivers[i])
		{
			printf("%s is set as a clone of itself\n",drivers[i]->name);
			return 1;
		}

		if (drivers[i]->clone_of && drivers[i]->clone_of->clone_of)
		{
#ifndef NEOFREE
#ifndef TINY_COMPILE
extern struct GameDriver neogeo_bios;
if (drivers[i]->clone_of->clone_of != &neogeo_bios)
{
#endif
#endif
			printf("%s is a clone of a clone\n",drivers[i]->name);
			return 1;
#ifndef NEOFREE
#ifndef TINY_COMPILE
}
#endif
#endif
		}

		for (j = i+1;drivers[j];j++)
		{
			if (!strcmp(drivers[i]->name,drivers[j]->name))
			{
				printf("%s is a duplicate name (%s, %s)\n",drivers[i]->name,drivers[i]->source_file,drivers[j]->source_file);
				return 1;
			}
			if (!strcmp(drivers[i]->description,drivers[j]->description))
			{
				printf("%s is a duplicate description (%s, %s)\n",drivers[i]->description,drivers[i]->name,drivers[j]->name);
				return 1;
			}
			if (drivers[i]->rom == drivers[j]->rom && drivers[i]->rom!=NULL)
			{
				printf("%s and %s use the same ROM set\n",drivers[i]->name,drivers[j]->name);
				return 1;
			}
		}

		romp = drivers[i]->rom;

if (romp)
{ /*MESS*/

		while (romp->name || romp->offset || romp->length)
		{
			const char *c;
			if (romp->name && romp->name != (char *)-1)
			{
				c = romp->name;
				while (*c)
				{
					if (tolower(*c) != *c)
					{
						printf("%s has upper case ROM names, please use lower case\n",drivers[i]->name);
						return 1;
					}
					c++;
				}
			}

			romp++;
		}
}/*MESS*/
	}

	return 0;
}
#endif




int run_game(int game)
{
	int err;


#ifdef MAME_DEBUG
	/* validity checks */
	if (validitychecks()) return 1;
#endif


	/* copy some settings into easier-to-handle variables */
	errorlog   = options.errorlog;
	record     = options.record;
	playback   = options.playback;
	mame_debug = options.mame_debug;
	No_FM      = options.no_fm;

	Machine->gamedrv = gamedrv = drivers[game];
	Machine->drv = drv = gamedrv->drv;

	/* copy configuration */
	if (options.color_depth == 16 ||
			(options.color_depth != 8 && (Machine->gamedrv->flags & GAME_REQUIRES_16BIT)))
		Machine->color_depth = 16;
	else
		Machine->color_depth = 8;
	Machine->sample_rate = options.samplerate;
	Machine->sample_bits = options.samplebits;

	/* get orientation right */
	Machine->orientation = gamedrv->flags & ORIENTATION_MASK;
	Machine->ui_orientation = ORIENTATION_DEFAULT;
	if (options.norotate)
		Machine->orientation = ORIENTATION_DEFAULT;
	if (options.ror)
	{
		/* if only one of the components is inverted, switch them */
		if ((Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_X ||
				(Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_Y)
			Machine->orientation ^= ORIENTATION_ROTATE_180;

		Machine->orientation ^= ORIENTATION_ROTATE_90;

		/* if only one of the components is inverted, switch them */
		if ((Machine->ui_orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_X ||
				(Machine->ui_orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_Y)
			Machine->ui_orientation ^= ORIENTATION_ROTATE_180;

		Machine->ui_orientation ^= ORIENTATION_ROTATE_90;
	}
	if (options.rol)
	{
		/* if only one of the components is inverted, switch them */
		if ((Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_X ||
				(Machine->orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_Y)
			Machine->orientation ^= ORIENTATION_ROTATE_180;

		Machine->orientation ^= ORIENTATION_ROTATE_270;

		/* if only one of the components is inverted, switch them */
		if ((Machine->ui_orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_X ||
				(Machine->ui_orientation & ORIENTATION_ROTATE_180) == ORIENTATION_FLIP_Y)
			Machine->ui_orientation ^= ORIENTATION_ROTATE_180;

		Machine->ui_orientation ^= ORIENTATION_ROTATE_270;
	}
	if (options.flipx)
	{
		Machine->orientation ^= ORIENTATION_FLIP_X;
		Machine->ui_orientation ^= ORIENTATION_FLIP_X;
	}
	if (options.flipy)
	{
		Machine->orientation ^= ORIENTATION_FLIP_Y;
		Machine->ui_orientation ^= ORIENTATION_FLIP_Y;
	}

	set_pixel_functions();

	/* Do the work*/
	err = 1;
	bailing = 0;

   #ifdef MESS
   {
   int i;
	/* MESS - set up the storage peripherals */
	for (i = 0; i < MAX_ROM; i ++)
		strcpy (rom_name[i], options.rom_name[i]);
	for (i = 0; i < MAX_FLOPPY; i ++)
		strcpy (floppy_name[i], options.floppy_name[i]);
	for (i = 0; i < MAX_HARD; i ++)
		strcpy (hard_name[i], options.hard_name[i]);
	for (i = 0; i < MAX_CASSETTE; i ++)
		strcpy (cassette_name[i], options.cassette_name[i]);
   }
   #endif

	if (osd_init() == 0)
	{
		if (init_machine() == 0)
		{
			if (run_machine() == 0)
				err = 0;
			else if (!bailing)
			{
				bailing = 1;
				printf("Unable to start machine emulation\n");
			}

			shutdown_machine();
		}
		else if (!bailing)
		{
			bailing = 1;
			printf("Unable to initialize machine emulation\n");
		}

		osd_exit();
	}
	else if (!bailing)
	{
		bailing = 1;
		printf ("Unable to initialize system\n");
	}

	return err;
}



/***************************************************************************

  Initialize the emulated machine (load the roms, initialize the various
  subsystems...). Returns 0 if successful.

***************************************************************************/
int init_machine(void)
{
	int i;

	for (i = 0;i < MAX_MEMORY_REGIONS;i++)
	{
		Machine->memory_region[i] = 0;
		Machine->memory_region_length[i] = 0;
		Machine->memory_region_type[i] = 0;
	}

	if (gamedrv->input_ports)
	{
		int total;
		const struct InputPort *from;
		struct InputPort *to;

		from = gamedrv->input_ports;

		total = 0;
		do
		{
			total++;
		} while ((from++)->type != IPT_END);

		if ((Machine->input_ports = malloc(total * sizeof(struct InputPort))) == 0)
			return 1;

		from = gamedrv->input_ports;
		to = Machine->input_ports;

		do
		{
			memcpy(to,from,sizeof(struct InputPort));

			to++;
		} while ((from++)->type != IPT_END);
	}


	#ifdef MESS
   	if (!gamedrv->rom)
        {
      	  if(errorlog) fprintf(errorlog, "Going to load_next tag\n");
          goto load_next;
        }
   #endif

	if (readroms() != 0)
	{
		free(Machine->input_ports);
		Machine->input_ports = 0;
		return 1;
	}


	{
		extern unsigned char *RAM;
		RAM = memory_region(drv->cpu[0].memory_region);
		ROM = RAM;
	}

	/* decrypt the ROMs if necessary */
	if (gamedrv->rom_decode) (*gamedrv->rom_decode)();

	if (gamedrv->opcode_decode)
	{
		int j;
		extern int encrypted_cpu;


		/* find the first available memory region pointer */
		j = 0;
		while (Machine->memory_region[j]) j++;

		/* allocate a ROM array of the same length of cpu #0 memory region */
		if ((ROM = malloc(memory_region_length(drv->cpu[0].memory_region))) == 0)
		{
			free(Machine->input_ports);
			Machine->input_ports = 0;
			/* TODO: should also free the allocated memory regions */
			return 1;
		}

		Machine->memory_region[j] = ROM;
		Machine->memory_region_length[j] = memory_region_length(drv->cpu[0].memory_region);

		encrypted_cpu = 0;
		(*gamedrv->opcode_decode)();
	}

   #ifdef MESS
	load_next:
	/* The ROM loading routine should allocate the ROM and RAM space and */
	/* assign them to the appropriate Machine->memory_region blocks. */
        if (gamedrv->rom_load && (*gamedrv->rom_load)() != 0)
	{
		free(Machine->input_ports);
		printf("Image load failed.\n");
		return 1;
	}
	#endif

	/* Mish:  Multi-session safety - set spriteram size to zero before memory map is set up */
	spriteram_size=spriteram_2_size=0;

	/* first of all initialize the memory handlers, which could be used by the */
	/* other initialization routines */
	cpu_init();

	/* load input ports settings (keys, dip switches, and so on) */
	settingsloaded = load_input_port_settings();

	/* ASG 971007 move from mame.c */
	if( !initmemoryhandlers() )
	{
		free(Machine->input_ports);
		Machine->input_ports = 0;
		return 1;
	}

	if (gamedrv->driver_init) (*gamedrv->driver_init)();

	return 0;
}



void shutdown_machine(void)
{
	int i;


	/* ASG 971007 free memory element map */
	shutdownmemoryhandler();

	/* free the memory allocated for ROM and RAM */
	for (i = 0;i < MAX_MEMORY_REGIONS;i++)
	{
		free(Machine->memory_region[i]);
		Machine->memory_region[i] = 0;
		Machine->memory_region_length[i] = 0;
		Machine->memory_region_type[i] = 0;
	}

	/* free the memory allocated for input ports definition */
	free(Machine->input_ports);
	Machine->input_ports = 0;
}



static void vh_close(void)
{
	int i;


	for (i = 0;i < MAX_GFX_ELEMENTS;i++)
	{
		freegfx(Machine->gfx[i]);
		Machine->gfx[i] = 0;
	}
	freegfx(Machine->uifont);
	Machine->uifont = 0;
	osd_close_display();
	palette_stop();

	if (drv->video_attributes & VIDEO_BUFFERS_SPRITERAM) {
		if (buffered_spriteram) free(buffered_spriteram);
		if (buffered_spriteram_2) free(buffered_spriteram_2);
		buffered_spriteram=NULL;
		buffered_spriteram_2=NULL;
	}
}



static int vh_open(void)
{
	int i;


	for (i = 0;i < MAX_GFX_ELEMENTS;i++) Machine->gfx[i] = 0;
	Machine->uifont = 0;

	if (palette_start() != 0)
	{
		vh_close();
		return 1;
	}


	/* convert the gfx ROMs into character sets. This is done BEFORE calling the driver's */
	/* convert_color_prom() routine (in palette_init()) because it might need to check the */
	/* Machine->gfx[] data */
	if (drv->gfxdecodeinfo)
	{
		for (i = 0;i < MAX_GFX_ELEMENTS && drv->gfxdecodeinfo[i].memory_region != -1;i++)
		{
			int len,avail,j,start;

			start = 0;
			for (j = 0;j < MAX_GFX_PLANES;j++)
			{
				if (drv->gfxdecodeinfo[i].gfxlayout->planeoffset[j] > start)
					start = drv->gfxdecodeinfo[i].gfxlayout->planeoffset[j];
			}
			start &= ~(drv->gfxdecodeinfo[i].gfxlayout->charincrement-1);
			len = drv->gfxdecodeinfo[i].gfxlayout->total *
					drv->gfxdecodeinfo[i].gfxlayout->charincrement;
			avail = memory_region_length(drv->gfxdecodeinfo[i].memory_region)
					- (drv->gfxdecodeinfo[i].start & ~(drv->gfxdecodeinfo[i].gfxlayout->charincrement/8-1));
			if ((start + len) / 8 > avail)
			{
				bailing = 1;
				printf ("Error: gfx[%d] extends past allocated memory\n",i);
				vh_close();
				return 1;
			}

			if ((Machine->gfx[i] = decodegfx(memory_region(drv->gfxdecodeinfo[i].memory_region)
					+ drv->gfxdecodeinfo[i].start,
					drv->gfxdecodeinfo[i].gfxlayout)) == 0)
			{
				vh_close();
				return 1;
			}
			if (Machine->remapped_colortable)
				Machine->gfx[i]->colortable = &Machine->remapped_colortable[drv->gfxdecodeinfo[i].color_codes_start];
			Machine->gfx[i]->total_colors = drv->gfxdecodeinfo[i].total_color_codes;
		}
	}


	/* create the display bitmap, and allocate the palette */
	if ((Machine->scrbitmap = osd_create_display(
			drv->screen_width,drv->screen_height,
			Machine->color_depth,
			drv->video_attributes)) == 0)
	{
		vh_close();
		return 1;
	}

	/* create spriteram buffers if necessary */
	if (drv->video_attributes & VIDEO_BUFFERS_SPRITERAM) {
		if (spriteram_size!=0) {
			buffered_spriteram= malloc(spriteram_size);
			if (!buffered_spriteram) { vh_close(); return 1; }
			if (spriteram_2_size!=0) buffered_spriteram_2 = malloc(spriteram_2_size);
			if (spriteram_2_size && !buffered_spriteram_2) { vh_close(); return 1; }
		} else {
			if (errorlog) fprintf(errorlog,"vh_open():  Video buffers spriteram but spriteram_size is 0\n");
			buffered_spriteram=NULL;
			buffered_spriteram_2=NULL;
		}
	}

	/* build our private user interface font */
	/* This must be done AFTER osd_create_display() so the function knows the */
	/* resolution we are running at and can pick a different font depending on it. */
	/* It must be done BEFORE palette_init() because that will also initialize */
	/* (through osd_allocate_colors()) the uifont colortable. */
	if ((Machine->uifont = builduifont()) == 0)
	{
		vh_close();
		return 1;
	}

	/* initialize the palette - must be done after osd_create_display() */
	palette_init();

	return 0;
}



/***************************************************************************

  This function takes care of refreshing the screen, processing user input,
  and throttling the emulation speed to obtain the required frames per second.

***************************************************************************/

int need_to_clear_bitmap;	/* set by the user interface */

int updatescreen(void)
{
	/* update sound */
	sound_update();

	if (osd_skip_this_frame() == 0)
	{
		profiler_mark(PROFILER_VIDEO);
		if (need_to_clear_bitmap)
		{
			osd_clearbitmap(Machine->scrbitmap);
			need_to_clear_bitmap = 0;
		}
		(*drv->vh_update)(Machine->scrbitmap,bitmap_dirty);  /* update screen */
		bitmap_dirty = 0;
		profiler_mark(PROFILER_END);
	}

	/* the user interface must be called between vh_update() and osd_update_video_and_audio(), */
	/* to allow it to overlay things on the game display. We must call it even */
	/* if the frame is skipped, to keep a consistent timing. */
	if (handle_user_interface())
		/* quit if the user asked to */
		return 1;

	osd_update_video_and_audio();

	if (drv->vh_eof_callback) (*drv->vh_eof_callback)();

	return 0;
}


/***************************************************************************

  Run the emulation. Start the various subsystems and the CPU emulation.
  Returns non zero in case of error.

***************************************************************************/
int run_machine(void)
{
	int res = 1;


	if (vh_open() == 0)
	{
		tilemap_init();
		sprite_init();
		gfxobj_init();
		if (drv->vh_start == 0 || (*drv->vh_start)() == 0)      /* start the video hardware */
		{
			if (sound_start() == 0) /* start the audio hardware */
			{
				int	region;

				/* free memory regions allocated with ROM_REGION_DISPOSE (typically gfx roms) */
				for (region = 0; region < MAX_MEMORY_REGIONS; region++)
				{
					if (Machine->memory_region_type[region] & REGIONFLAG_DISPOSE)
					{
						free (Machine->memory_region[region]);
						Machine->memory_region[region] = 0;
					}
				}

				if (settingsloaded == 0)
				{
					/* if there is no saved config, it must be first time we run this game, */
					/* so show the disclaimer. */
					if (showcopyright()) goto userquit;
				}

				if (showgamewarnings() == 0)  /* show info about incorrect behaviour (wrong colors etc.) */
				{
					/* shut down the leds (work around Allegro hanging bug in the DOS port) */
					osd_led_w(0,1);
					osd_led_w(1,1);
					osd_led_w(2,1);
					osd_led_w(3,1);
					osd_led_w(0,0);
					osd_led_w(1,0);
					osd_led_w(2,0);
					osd_led_w(3,0);

					init_user_interface();

					if (options.cheat) InitCheat();

					cpu_run();      /* run the emulation! */

					if (options.cheat) StopCheat();

					/* save input ports settings */
					save_input_port_settings();
				}

userquit:
				/* the following MUST be done after hiscore_save() otherwise */
				/* some 68000 games will not work */
				sound_stop();
				if (drv->vh_stop) (*drv->vh_stop)();

				res = 0;
			}
			else if (!bailing)
			{
				bailing = 1;
				printf("Unable to start audio emulation\n");
			}
		}
		else if (!bailing)
		{
			bailing = 1;
			printf("Unable to start video emulation\n");
		}

		gfxobj_close();
		sprite_close();
		tilemap_close();
		vh_close();
	}
	else if (!bailing)
	{
		bailing = 1;
		printf("Unable to initialize display\n");
	}

	return res;
}



int mame_highscore_enabled(void)
{
	/* disable high score when record/playback is on */
	if (record != 0 || playback != 0) return 0;

	/* disable high score when cheats are used */
	if (he_did_cheat != 0) return 0;

#ifdef MAME_NET
    /* disable high score when playing network game */
    /* (this forces all networked machines to start from the same state!) */
    if (net_active()) return 0;
#endif /* MAME_NET */

	return 1;
}
