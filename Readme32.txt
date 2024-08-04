              M.A.M.E.  -  Multiple Arcade Machine Emulator
         Copyright (C) 1997-1999  by Nicola Salmoria and the MAME team

                 MAME32 Copyright (C) 1997-1999 by MAME32 team.


                          MAME32 Release Notes
                          --------------------

This is release 0.36 MAME32, the Multiple Arcade Machine Emulator for Win32.

MAME32 is simple to use--put zipped ROM files in a roms subdirectory,
zipped sample files in a samples subdirectory, and run MAME32.
MAME32 requires DirectX 3.0 or higher to be installed on your system.
If you need DirectX, go to http://www.microsoft.com/directx and download
the latest DirectX runtime (version 6) for Windows 95/98.
If you use Windows NT, install Service Pack 3 or higher.


Requirements
------------

- Windows 95, Windows 98 or Windows NT 4.
- DirectX version 3.0 or higher.
- Pentium class cpu.


Acknowledgements
----------------
Thanks to John Hardy IV for excellent testing.
Also, thanks to Mike Haaland, Jeff Miller, Andrew Kirmse,
Mohammad Rezaei and Robert Schlabbach for contributing code to MAME32.


Usage
-----

MAME32 [name of the game to run] [options]

options:
--------
-scanlines/-noscanlines (default: -scanlines)
              if the default mode doesn't work with your monitor/video card
              (double image/picture squashed vertically), use -noscanlines
              or -vesa.
-resolution XxY
              Where X and Y are width and height (ex: '-resolution 800x600')
              MAME goes some lengths to autoselect a good resolution. You can
              override MAME's choice with this option.
              You can use -XxY (e.g. -800x600) as a shortcut. Frontend
              authors are advised to use -resolution XxY, however.
-320          Discontinued. Use -320x240.
-400          same as above, use -400x300
-512          same as above, use -512x384
-640          same as above, use -640x480
-800          same as above, use -800x600.
-1024         same as above, use -1024x768
-1280         same as above, use 1280x1024.
-1600         same as above, use 1600x1200.
-skiplines N / -skipcolumns N
              if you run a game on a video mode smaller than the visible area,
              you can adjust its position using the PGUP and PGDN keys (alone
              for vertical panning, shifted for horizontal panning).
              You can also use these two parameters to set the initial
              position: 0 is the default, menaing that the screen is centered.
-double/-nodouble (default: double)
              use nodouble to disable pixel doubling in VESA modes (faster,
              but smaller picture). Use double to force pixel doubling when
              the image doesn't fit in the screen (you'll have to use PGUP and
              PGDN to scroll).
-depth n      (default: auto)
              Some games need 16-bit color modes to get accurate graphics. To
              improve speed, you can turn that off using -depth 8, which limits
              to the standard 256 color modes. You can also use -depth 16 to
              force games to use a 16-bit diplay even if they fit in 256 colors,
              but this isn't suggested.
-gamma n      (Default is 1.0)
              Set the initial gamma correction value. 8-bit depth only.
-vsync        Not implemented in MAME32.
-ror          Rotate the display clockwise by 90 degrees.
-rol          Rotate display anticlockwise.
-flipx        Flip display horizontally.
-flipy        Flip display vertically.
              -ror and -rol provide authentic *vertical* scanlines, given you
			  turn your monitor to its side.
              CAUTION:
              A monitor is a complicated, high voltage electronic device.
              There are some monitors which were designed to be rotated.
              If yours is _not_ one of those, but you absolutely must
              turn it to its side, you do so at your own risk.

              ******************************************************
              PLEASE DO NOT LET YOUR MONITOR WITHOUT ATTENTION IF IT
              IS PLUGGED IN AND TURNED TO ITS SIDE
              ******************************************************

-soundcard n  Select sound card (if this is not specified, you will be asked
              interactively). SEAL only.
-sr n         Set the audio sample rate. The default is 22050. Smaller values
              (e.g. 11025) will cause lower audio quality but faster emulation
              speed. Higher values (e.g. 44100) will cause higher audio quality
              but slower emulation speed.
-sb n         Set the audio sample bits, either 8 or 16. The default is 8.
              16 will increase quality with certain games, but decrease speed.
              This is a software setting, not hardware. The sound card will
              always be used in 16 bit mode, if possible.
-stereo/-nostereo (default: -stereo)
              enables stereo output for games supporting it.
-volume n     (default: 0) set the startup volume. It can later be changed
              using the On Screen Display. The volume is an attenuation in dB,
              e.g. "-volume -12" will start with a -12dB attenuation.
-nosamples    Turn off sample loading.
-noartwork    Turn off artwork loading.
-joy/-nojoy   (default: -nojoy) allow joystick input.
              Use the Windows control panel to calibrate the joystick.
-ym3812opl/-noym3812opl (default: -ym3812opl) use the SoundBlaster OPL chip for
              music emulation of the YM3812 chip. This is faster, and is
              reasonably accurate since the chips are 100% compatible. However,
              the pitch might be wrong. Also note that with -no3812opl you need
              some external drum samples.
              On NT, you will need to install mameopl.sys for the -ym3812opl option.
-log          Create a log of illegal memory accesses in ERROR.LOG
-help, -?     Display current mame version and copyright notice.
-mouse/-nomouse (default: -mouse) enable/disable mouse support.
-frameskip n  (default: auto)
              Skip frames to speed up the emulation. The argument is the number
              of frames to skip out of 12. For example, if the game normally
              runs at 60 fps, "-frameskip 2" will make it run at 50 fps, and
              "-frameskip 6" at 30 fps. Use F11 to check the speed your
              computer is actually reaching. If it is below 100%, increase the
              frameskip value. You can press F9 to change frameskip while
              running the game.
              When set to auto (the default), the frameskip setting is
              dynamically adjusted at run time to display the maximum possible
              frames without dropping below 100% speed.
-antialias/-noantialias (default: -antialias)
              Antialiasing for the vector games.
-beam n       Sets the width in pixels of the vectors. n is a float in the
              range of 1.00 through 16.00.
-flicker n    Make the vectors flicker. n is an optional argument, a float in
              the range range 0.00 - 100.00 (0=none 100=maximum).
-translucency/-notranslucency (default: -translucency)
              enables or disables vector translucency.
-cheat        Cheats like the speedup in Pac Man or the level skip in many
              other games are disabled by default. Use this switch to turn
              them on.
-debug        Activate the integrated debugger. During the emulation, press
              tilde to enter the debugger. This is only available if the
              program is compiled with MAME_DEBUG defined.


MAME32 specific options. All the above plus:
--------------------------------------------
-noddraw      Uses Windows GDI to display in a window instead of using DirectDraw.
-window       Display in a window. The opposite of -screen.
-screen       Display fullscreen (default). The opposite of -window.
-nosound      Turn off audio.
-midas        Use the MIDAS Digital Audio System for audio. Try this option
              if the default SEAL audio sounds choppy or slows down mame. This can
              happen if your audio drivers don't provide DirectSound hardware support.
              This is the best option for Windows NT users.
-directsound  Use an alternate sound subsystem which uses the DirectSound API.
              Note that SEAL and MIDAS can also use DirectSound without this option.
-vscanlines/-novscanlines
              Use vertical scanlines. Simulates scanlines of vertical monitors.
-quit         Quit after running the first game. Useful when specifying a game on the command line.
-noautopause  Disables pausing the game when MAME32 loses focus.
-useblit      Use a BitBlit to transfer image data to the screen in fullscreen mode.
              This only applies to the double option with no scanlines.
              This may be faster or slower depending on your system.
-nommx        Disable the MMX code.
-nocpudetect  Do not try to detect an MMX processor.
-dikbd        Use DirectInput for keyboard input.
-dijoy        Use DirectInput for joystick input. Only avalible with DirectX 5 or above.
-joygui       Enable the joystick to select games in the UI.
-dijoygui     Use DirectInput for the joystick to select games in the UI.
-ignorebadoptions Allows MAME32 to run with bogus options.

Keys
----
Tab          Toggles the configuration menu.
Tilde        Toggles the On Screen Display.
             Use the up and down arrow keys to select the parameter (global
             volume, mixing level, gamma correction etc.), left and right to
             arrow keys to modify it.
P            Pause.
Shift+P      While paused, advance to next frame.
F3           Reset.
F4           Show the game graphics. Use cursor keys to change set/color,
             F4 or Esc to return to the emulation.
F9           Change frame skip on the fly.
F10          Toggle speed throttling.
F11          Toggle speed display.
Shift+F11    Toggle profiler display
F12          Save a screen snapshot. The default target directory is 'images'.
ESC          Exit emulator.


Known problems 
--------------

MAME32 may perform poorly under Windows NT. DirectX under NT can be slow
when emulating hardware, especially sound and joysticks. You may get better
performance by disabling sound, joysticks, or by changing video modes. Sorry,
Chris loves NT, but its hardware support of DirectX isn't very fast right now. 

MAME32 may run slow in a window if your desktop is at 24 bit or 
32 bit color depth.  This isn't a problem, it's just a fact--moving around 
that much more video memory takes a long time.  Chris optimized the 16 bit color
depth version a bit, so it's about as fast as it can get. 

Some DirectDraw display drivers don't support locking the primary surface
in fullscreen mode. MAME32 will perform blits from a back buffer in this case.
Note that this is slower than writing directly to the primary surface.

16-bit color is not supported in windowed mode with the DirectDraw option when
the desktop display depth is not set to 16 bit. 

When an error occurs starting a game, a spurious dialog box may appear with the
message that the roms may be corrupt. In most cases, this message can be ignored.

There are some games which use an OPL chip for music. These are identified in the
readme.txt file as requiring a Sound Blaster OPL chip. MAME32 will not produce
music for these games on Windows NT unless you install the mameopl.sys driver or
run MAME32 with administrative privledges.

The joystick mapping settings only work with the DirectInput Joystick option.

Contacts
--------
Michael Soderstrom:        ichael@geocities.com
Christopher Kirmse         ckirmse@pobox.com
John L. Hardy IV:          mame32qa@yahoo.com
MAME home page:            http://www.mame.net
Michael's MAME32 page:     http://www.classicgaming.com/mame32
Christopher's MAME32 page: http://www.geocities.com/TimesSquare/Lair/8706/mame32.html
John L. Hardy IV's page:   http://www.classicgaming.com/mame32qa
Compiling MAME32 page:     http://www.hypertech.com/mame
MIDAS home page:           http://www.s2.org/midas
SEAL home page:            http://www.egerter.com/seal
