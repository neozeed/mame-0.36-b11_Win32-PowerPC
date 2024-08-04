/***************************************************************************

  vidhrdw.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"

static char *rawbitmap;

static int x,y,screen_width;
static int last_command;

// We reason we need this pending business, is that otherwise, when the guy
// walks on the rainbow, he'd leave a trail behind him
static int pending, pending_x, pending_y, pending_color;

void leprechn_graphics_command_w(int offset,int data)
{
    last_command = data;
}

void leprechn_graphics_data_w(int offset,int data)
{
    int direction;

    if (pending)
    {
		plot_pixel2(tmpbitmap, Machine->scrbitmap, pending_x, pending_y, Machine->pens[pending_color]);
        rawbitmap[pending_y * screen_width + pending_x] = pending_color;

        pending = 0;
    }

    switch (last_command)
    {
    // Write Command
    case 0x00:
        direction = (data & 0xf0) >> 4;
        switch (direction)
        {
        case 0x00:
        case 0x04:
        case 0x08:
        case 0x0c:
            break;

        case 0x01:
        case 0x09:
            x++;
            break;

        case 0x02:
        case 0x06:
            y++;
            break;

        case 0x03:
            x++;
            y++;
            break;

        case 0x05:
        case 0x0d:
            x--;
            break;

        case 0x07:
            x--;
            y++;
            break;

        case 0x0a:
        case 0x0e:
            y--;
            break;

        case 0x0b:
            x++;
            y--;
            break;

        case 0x0f:
            x--;
            y--;
            break;
        }

        x = x & 0xff;
        y = y & 0xff;

        pending = 1;
        pending_x = x;
        pending_y = y;
        pending_color = data & 0x0f;

        return;

    // X Position Write
    case 0x08:
        x = data;
        return;

    // Y Position Write
    case 0x10:
        y = data;
        return;

    // Clear Bitmap
    case 0x18:
        fillbitmap(Machine->scrbitmap,Machine->pens[data],0);
        fillbitmap(tmpbitmap,Machine->pens[data],0);
        memset(rawbitmap, data, screen_width * Machine->drv->screen_height);
        osd_mark_dirty(0,0,screen_width-1,Machine->drv->screen_height-1,0);
        return;
    }

    // Just a precaution. Doesn't seem to happen.
    if (errorlog) fprintf(errorlog, "Unknown Graphics Command #%2X at %04X\n", last_command, cpu_get_pc());
}


int leprechn_graphics_data_r(int offset)
{
    return rawbitmap[y * screen_width + x];
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
int leprechn_vh_start(void)
{
    screen_width = Machine->drv->screen_width;

    if ((rawbitmap = malloc(screen_width*Machine->drv->screen_height)) == 0)
    {
        return 1;
    }

	if (generic_bitmapped_vh_start())
	{
        free(rawbitmap);
        return 1;
    }

    pending = 0;

    return 0;
}

/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/
void leprechn_vh_stop(void)
{
    free(rawbitmap);
    generic_bitmapped_vh_stop();
}
