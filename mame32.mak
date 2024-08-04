# before compiling for the first time, use "make makedir" to create all the
# necessary subdirectories

# Set the version here
VERSION = -DVERSION=36

# uncomment out the BETA_VERSION = line to build a beta version of MAME
BETA_VERSION = -DBETA_VERSION=11

# uncomment this to build an release canidate version
RELEASE_CANDIDATE = -DRELEASE_CANDIDATE=1

# uncomment out the MAME_DEBUG = line to build a version of MAME for debugging games
# MAME_DEBUG = -DMAME_DEBUG

# if MAME_MMX is defined, MMX will be compiled in; requires the Intel compiler and libmmtck.lib
# MAME_MMX = -DMAME_MMX

# if MAME_NET is defined, network support will be compiled in; requires wsock32.lib
# MAME_NET = -DMAME_NET

# whether to use fastcall or stdcall
USE_FASTCALL = 1

# uncomment to build without SEAL
NOSEAL =

# uncomment to build without MIDAS
NOMIDAS =

# uncomment to build Helpfiles
# HELPFILE = Mame32.hlp

CC      = cl.exe
LD      = link.exe
RSC     = rc.exe
ASM     = nasmw.exe

ASMFLAGS = -f win32
ASMDEFS  =

VPATH=src src/cpu/z80 src/cpu/i8085 src/cpu/m6502 src/cpu/h6280 \
	  src/cpu/i86 src/cpu/nec src/cpu/m6800 src/cpu/m6805 \
	  src/cpu/m6809 src/cpu/m68000 src/cpu/tms34010 src/cpu/tms9900 \
	  src/cpu/z8000 src/cpu/tms32010 src/cpu/ccpu

# uncomment out the DEBUG = line to build a debugging version of mame32.
# DEBUG = 1

# uncomment next line to do a smaller compile including only one driver
# TINY_COMPILE = 1
TINY_NAME = rthunder_driver
TINY_OBJS = obj/drivers/rthunder.o obj/vidhrdw/rthunder.o

# uncomment one of the two next lines to not compile the NeoGeo games or to
# compile only the NeoGeo games
# NEOFREE = 1
# NEOMAME = 1

# uncomment next line to use Assembler 68k engine
# X86_ASM_68K = 1

# uncomment next line to use Assembler z80 engine
# X86_ASM_Z80 = 1

# uncomment next line to use Assembler 6800 engine
# X86_ASM_6800 = 1

!if "$(PROCESSOR_ARCHITECTURE)" == ""
PROCESSOR_ARCHITECTURE=x86
!endif

!ifdef NEOMAME

HAS_CPUS = -DHAS_Z80=1 -DHAS_M68000=1
HAS_SOUND = -DHAS_YM2610=1

!endif

!ifdef MAME_NET
NET_LIBS = wsock32.lib
!else
NET_LIBS =
!endif

!ifdef X86_ASM_68K
M68KOBJS = obj/cpu/m68000/asmintf.o obj/cpu/m68000/68kem.oa
M68KDEF  = -DA68KEM
!else
M68KOBJS = obj/cpu/m68000/m68kops.og obj/cpu/m68000/m68kopac.og \
           obj/cpu/m68000/m68kopdm.og obj/cpu/m68000/m68kopnz.og \
           obj/cpu/m68000/m68kcpu.o obj/cpu/m68000/m68kmame.o
M68KDEF  =
!endif

!ifdef X86_ASM_Z80
Z80OBJS = obj/cpu/z80/z80.oa
80DEF = -nasm -verbose 0
!else
Z80OBJS = obj/cpu/z80/z80.o
z80DEF =
!endif

!ifdef X86_ASM_6800
M6800OBJS = obj/cpu/m6800/m6800.oa
!else
M6800OBJS = obj/cpu/m6800/m6800.o
!endif

DEFS   = -DLSB_FIRST -DWIN32 -DPI=3.1415926535 \
         -DINLINE="static __inline" -Dinline=__inline -D__inline__=__inline \
	 -DPNG_SAVE_SUPPORT -D_WINDOWS -DZLIB_DLL

AUDIOFLAGS =
AUDIOOBJS  =
AUDIOLIBS  =

!ifndef NOSEAL
AUDIOFLAGS = -ISEAL
AUDIOOBJS  = obj/Win32/SealSound.o
AUDIOLIBS  = SEAL/Audw32vc.lib 
!else
AUDIOFLAGS = -DNOSEAL
!endif

!ifndef NOMIDAS
AUDIOFLAGS = $(AUDIOFLAGS) -IMIDAS
AUDIOOBJS  = $(AUDIOOBJS) obj/Win32/MIDASSound.o
AUDIOLIBS  = $(AUDIOLIBS) MIDAS/MIDAS.lib
!else
AUDIOFLAGS = $(AUDIOFLAGS) -DNOMIDAS
!endif

CFLAGSGLOBAL = -I. -Isrc -Iobj/cpu/m68000 -Isrc/cpu/m68000 -Isrc/Win32 \
               -IZLIB $(AUDIOFLAGS) -W3 -nologo -MT \
               $(MAME_DEBUG) $(RELEASE_CANDIDATE) $(BETA_VERSION) $(VERSION) \
               $(MAME_NET) $(MAME_MMX) $(HAS_CPUS) $(HAS_SOUND) $(M68KDEF) \
			-Idx /MT

CFLAGSDEBUG = -Zi -Od

CFLAGSOPTIMIZED = -DNDEBUG -Ox -Ob2

!ifdef DEBUG
CFLAGS = $(CFLAGSGLOBAL) $(CFLAGSDEBUG)
!else
CFLAGS = $(CFLAGSGLOBAL) $(CFLAGSOPTIMIZED)
!endif

!ifdef USE_FASTCALL
CFLAGS = $(CFLAGS) -DCLIB_DECL=__cdecl -DDECL_SPEC=__cdecl
!endif

LDFLAGSGLOBAL = -subsystem:windows -machine:$(PROCESSOR_ARCHITECTURE) -nologo /NODEFAULTLIB:libc.lib

LDFLAGSDEBUG = -debug:full
#LDFLAGSOPTIMIZED = -release -incremental:no -map
LDFLAGSOPTIMIZED = -release -incremental:no

!ifdef DEBUG
LDFLAGS = $(LDFLAGSGLOBAL) $(LDFLAGSDEBUG)
!else
LDFLAGS = $(LDFLAGSGLOBAL) $(LDFLAGSOPTIMIZED)
!endif

RCFLAGS = -idx -l 0x409 -DNDEBUG -IWin32 $(MAME_NET) $(MAME_MMX)

# directx lib
# dx\dinput.lib dx\dxguid.lib

LIBS   = kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib \
         winmm.lib shell32.lib vfw32.lib ZLIB\zlib.lib \
         $(AUDIOLIBS)

!ifdef MAME_NET
NET_OBJS = obj/network.o obj/Win32/net32.o obj/Win32/netchat32.o
!else
NET_OBJS =
!endif

# direct draw
# obj/Win32/DDrawWindow.o obj/Win32/DDrawDisplay.o
# direct input
# obj/Win32/DirectInput.o obj/Win32/DIKeyboard.o obj/Win32/DIJoystick.o \
# obj/Win32/DirectDraw.o
# obj/Win32/DirectSound.o 
#          obj/Win32/Avi.o \

WIN32_OBJS = \
         obj/Win32/osdepend.o \
         obj/Win32/MAME32.o obj/Win32/M32Util.o \
         obj/Win32/uclock.o \
         obj/Win32/Display.o obj/Win32/GDIDisplay.o \
	 obj/Win32/RenderBitmap.o obj/Win32/Dirty.o \
         obj/Win32/led.o obj/Win32/status.o \
         $(AUDIOOBJS) obj/Win32/NullSound.o \
         obj/Win32/Keyboard.o obj/Win32/Joystick.o obj/Win32/Trak.o \
         obj/Win32/file.o obj/Win32/Directories.o obj/Win32/mzip.o \
         obj/Win32/debug.o \
         obj/Win32/fmsynth.o obj/Win32/NTFMSynth.o \
         obj/Win32/audit32.o \
         obj/Win32/Win32ui.o obj/Win32/Properties.o obj/Win32/ColumnEdit.o \
         obj/Win32/Screenshot.o obj/Win32/TreeView.o obj/Win32/Splitters.o \
         obj/Win32/options.o obj/Win32/Bitmask.o obj/Win32/DataMap.o \
         obj/Win32/RenderMMX.o

CPUOBJS = \
         obj/cpu/gensync/gensync.o \
         $(Z80OBJS) obj/cpu/m6502/m6502.o obj/cpu/h6280/h6280.o \
         obj/cpu/i86/i86.o obj/cpu/nec/nec.o obj/cpu/i8039/i8039.o \
	 obj/cpu/i8085/i8085.o \
         $(M6800OBJS) obj/cpu/m6805/m6805.o obj/cpu/m6809/m6809.o \
	 obj/cpu/konami/konami.o \
         $(M68KOBJS) obj/cpu/s2650/s2650.o obj/cpu/t11/t11.o \
         obj/cpu/tms32010/tms32010.o \
         obj/cpu/tms34010/tms34010.o obj/cpu/tms34010/34010fld.o \
         obj/cpu/tms9900/tms9900.o obj/cpu/z8000/z8000.o \
         obj/cpu/ccpu/ccpu.o obj/vidhrdw/cinemat.o       

SNDOBJS = \
         obj/sound/samples.o obj/sound/dac.o \
	 obj/sound/ay8910.o obj/sound/2203intf.o \
         obj/sound/2151intf.o obj/sound/fm.o \
         obj/sound/ym2151.o obj/sound/ym2413.o \
         obj/sound/2608intf.o obj/sound/2610intf.o obj/sound/2612intf.o \
         obj/sound/ym3812.o obj/sound/3812intf.o obj/sound/fmopl.o \
         obj/sound/tms5220.o obj/sound/5220intf.o obj/sound/vlm5030.o \
         obj/sound/pokey.o obj/sound/sn76496.o \
         obj/sound/nes.o obj/sound/nesintf.o obj/sound/astrocde.o \
         obj/sound/adpcm.o obj/sound/msm5205.o \
	 obj/sound/namco.o obj/sound/ymdeltat.o \
	 obj/sound/upd7759.o obj/sound/hc55516.o \
         obj/sound/k007232.o obj/sound/k005289.o obj/sound/k051649.o \
	 obj/sound/k053260.o obj/sound/segapcm.o \
	 obj/sound/rf5c68.o obj/sound/cem3394.o obj/sound/c140.o \
	 obj/sound/qsound.o


DBGOBJS = \
         obj/cpu/gensync/gensyncd.o \
         obj/cpu/z80/z80dasm.o  obj/cpu/i8085/8085dasm.o obj/cpu/m6502/6502dasm.o \
         obj/cpu/h6280/6280dasm.o obj/cpu/i86/i86dasm.o obj/cpu/nec/necdasm.o \
	 obj/cpu/i8039/8039dasm.o \
         obj/cpu/m6800/6800dasm.o obj/cpu/m6805/6805dasm.o obj/cpu/m6809/6809dasm.o \
	 obj/cpu/konami/knmidasm.o \
         obj/cpu/t11/t11dasm.o obj/cpu/s2650/2650dasm.o obj/cpu/tms34010/34010dsm.o \
         obj/cpu/tms9900/9900dasm.o obj/cpu/z8000/8000dasm.o \
         obj/cpu/tms32010/32010dsm.o obj/cpu/ccpu/ccpudasm.o obj/cpu/m68000/d68k.o

COREOBJS = \
         obj/version.o obj/driver.o obj/mame.o obj/drawgfx.o obj/common.o obj/usrintrf.o \
         obj/cpuintrf.o obj/memory.o obj/timer.o obj/palette.o \
	 obj/input.o obj/inptport.o obj/cheat.o obj/unzip.o \
         obj/audit.o obj/info.o obj/png.o obj/artwork.o \
         obj/tilemap.o obj/sprite.o  obj/gfxobj.o \
         obj/state.o obj/datafile.o \
	 $(CPUOBJS) \
         obj/sndintrf.o \
         obj/sound/streams.o obj/sound/mixer.o \
         $(SNDOBJS) \
         obj/sound/votrax.o \
         obj/machine/z80fmly.o obj/machine/6821pia.o \
         obj/vidhrdw/generic.o obj/vidhrdw/vector.o \
         obj/vidhrdw/avgdvg.o obj/machine/mathbox.o \
         obj/machine/ticket.o \
	 obj/mamedbg.o obj/window.o obj/profiler.o \
         $(DBGOBJS) \
         $(NET_OBJS)

DRV_OBJS = \
         obj/machine/pacman.o obj/drivers/pacman.o \
         obj/machine/pacplus.o \
         obj/machine/theglob.o \
         obj/machine/jrpacman.o obj/drivers/jrpacman.o obj/vidhrdw/jrpacman.o \
         obj/vidhrdw/pengo.o obj/drivers/pengo.o \
         obj/vidhrdw/galaxian.o obj/drivers/galaxian.o \
         obj/sndhrdw/mooncrst.o obj/drivers/mooncrst.o \
         obj/machine/scramble.o obj/sndhrdw/scramble.o obj/drivers/scramble.o \
         obj/vidhrdw/frogger.o obj/sndhrdw/frogger.o obj/drivers/frogger.o \
         obj/drivers/scobra.o \
         obj/vidhrdw/amidar.o obj/drivers/amidar.o \
         obj/vidhrdw/fastfred.o obj/drivers/fastfred.o \
         obj/vidhrdw/cclimber.o obj/sndhrdw/cclimber.o obj/drivers/cclimber.o \
         obj/drivers/yamato.o \
         obj/vidhrdw/seicross.o obj/sndhrdw/wiping.o obj/drivers/seicross.o \
         obj/vidhrdw/wiping.o obj/drivers/wiping.o \
         obj/vidhrdw/cop01.o obj/drivers/cop01.o \
         obj/vidhrdw/terracre.o obj/drivers/terracre.o \
         obj/vidhrdw/galivan.o obj/drivers/galivan.o \
         obj/vidhrdw/armedf.o obj/drivers/armedf.o \
         obj/vidhrdw/phoenix.o obj/sndhrdw/phoenix.o obj/drivers/phoenix.o \
         obj/sndhrdw/pleiads.o \
         obj/vidhrdw/naughtyb.o obj/drivers/naughtyb.o \
         obj/vidhrdw/rallyx.o obj/drivers/rallyx.o \
         obj/drivers/locomotn.o \
         obj/machine/bosco.o obj/sndhrdw/bosco.o obj/vidhrdw/bosco.o obj/drivers/bosco.o \
         obj/machine/galaga.o obj/vidhrdw/galaga.o obj/drivers/galaga.o \
         obj/machine/digdug.o obj/vidhrdw/digdug.o obj/drivers/digdug.o \
         obj/vidhrdw/xevious.o obj/machine/xevious.o obj/drivers/xevious.o \
         obj/machine/superpac.o obj/vidhrdw/superpac.o obj/drivers/superpac.o \
         obj/machine/phozon.o obj/vidhrdw/phozon.o obj/drivers/phozon.o \
         obj/machine/mappy.o obj/vidhrdw/mappy.o obj/drivers/mappy.o \
         obj/machine/grobda.o obj/vidhrdw/grobda.o obj/drivers/grobda.o \
         obj/machine/gaplus.o obj/vidhrdw/gaplus.o obj/drivers/gaplus.o \
         obj/machine/polepos.o obj/vidhrdw/polepos.o obj/sndhrdw/polepos.o obj/drivers/polepos.o \
         obj/vidhrdw/pacland.o obj/drivers/pacland.o \
         obj/vidhrdw/skykid.o obj/drivers/skykid.o \
         obj/vidhrdw/baraduke.o obj/drivers/baraduke.o \
         obj/vidhrdw/namcos86.o obj/drivers/namcos86.o \
         obj/machine/namcos1.o obj/vidhrdw/namcos1.o obj/drivers/namcos1.o \
         obj/machine/namcos2.o obj/vidhrdw/namcos2.o obj/drivers/namcos2.o \
         obj/vidhrdw/cosmic.o obj/drivers/cosmic.o \
         obj/vidhrdw/cheekyms.o obj/drivers/cheekyms.o \
         obj/vidhrdw/ladybug.o obj/drivers/ladybug.o \
         obj/vidhrdw/mrdo.o obj/drivers/mrdo.o \
         obj/machine/docastle.o obj/vidhrdw/docastle.o obj/drivers/docastle.o \
         obj/vidhrdw/dkong.o obj/sndhrdw/dkong.o obj/drivers/dkong.o \
         obj/vidhrdw/mario.o obj/sndhrdw/mario.o obj/drivers/mario.o \
         obj/vidhrdw/popeye.o obj/drivers/popeye.o \
         obj/vidhrdw/punchout.o obj/sndhrdw/punchout.o obj/drivers/punchout.o \
         obj/machine/8080bw.o obj/vidhrdw/8080bw.o obj/sndhrdw/8080bw.o obj/drivers/8080bw.o \
         obj/vidhrdw/m79amb.o obj/drivers/m79amb.o \
         obj/vidhrdw/z80bw.o obj/sndhrdw/z80bw.o obj/drivers/z80bw.o \
         obj/drivers/lazercmd.o obj/vidhrdw/lazercmd.o \
         obj/drivers/meadows.o obj/sndhrdw/meadows.o obj/vidhrdw/meadows.o \
         obj/machine/wow.o obj/vidhrdw/wow.o obj/sndhrdw/wow.o obj/drivers/wow.o \
         obj/sndhrdw/gorf.o \
         obj/machine/mcr.o obj/sndhrdw/mcr.o \
         obj/vidhrdw/mcr1.o obj/vidhrdw/mcr2.o obj/vidhrdw/mcr3.o \
         obj/drivers/mcr1.o obj/drivers/mcr2.o obj/drivers/mcr3.o \
         obj/vidhrdw/mcr68.o obj/drivers/mcr68.o \
	 obj/vidhrdw/balsente.o obj/drivers/balsente.o \
         obj/vidhrdw/skychut.o obj/drivers/skychut.o \
         obj/sndhrdw/irem.o \
         obj/vidhrdw/mpatrol.o obj/drivers/mpatrol.o \
         obj/vidhrdw/troangel.o obj/drivers/troangel.o \
         obj/vidhrdw/yard.o obj/drivers/yard.o \
         obj/vidhrdw/travrusa.o obj/drivers/travrusa.o \
         obj/vidhrdw/m62.o obj/drivers/m62.o \
         obj/vidhrdw/vigilant.o obj/drivers/vigilant.o \
	 obj/vidhrdw/m72.o obj/sndhrdw/m72.o obj/drivers/m72.o \
         obj/vidhrdw/shisen.o obj/drivers/shisen.o \
         obj/vidhrdw/m92.o obj/drivers/m92.o \
         obj/drivers/m97.o \
         obj/vidhrdw/m107.o obj/drivers/m107.o \
         obj/vidhrdw/gottlieb.o obj/sndhrdw/gottlieb.o obj/drivers/gottlieb.o \
         obj/vidhrdw/crbaloon.o obj/drivers/crbaloon.o \
         obj/machine/qix.o obj/vidhrdw/qix.o obj/drivers/qix.o \
         obj/machine/taitosj.o obj/vidhrdw/taitosj.o obj/drivers/taitosj.o \
         obj/vidhrdw/bking2.o obj/drivers/bking2.o \
         obj/vidhrdw/gsword.o obj/drivers/gsword.o obj/machine/tait8741.o \
         obj/vidhrdw/retofinv.o obj/drivers/retofinv.o \
         obj/vidhrdw/tsamurai.o obj/drivers/tsamurai.o \
         obj/machine/flstory.o obj/vidhrdw/flstory.o obj/drivers/flstory.o \
         obj/vidhrdw/gladiatr.o obj/drivers/gladiatr.o \
         obj/machine/bublbobl.o obj/vidhrdw/bublbobl.o obj/drivers/bublbobl.o \
         obj/machine/mexico86.o obj/vidhrdw/mexico86.o obj/drivers/mexico86.o \
         obj/vidhrdw/rastan.o obj/sndhrdw/rastan.o obj/drivers/rastan.o \
         obj/machine/rainbow.o obj/drivers/rainbow.o \
         obj/machine/arkanoid.o obj/vidhrdw/arkanoid.o obj/drivers/arkanoid.o \
         obj/vidhrdw/superqix.o obj/drivers/superqix.o \
         obj/machine/tnzs.o obj/vidhrdw/tnzs.o obj/drivers/tnzs.o \
         obj/vidhrdw/superman.o obj/drivers/superman.o obj/machine/cchip.o \
         obj/vidhrdw/footchmp.o obj/drivers/footchmp.o \
         obj/drivers/lkage.o obj/vidhrdw/lkage.o \
         obj/vidhrdw/taitol.o obj/drivers/taitol.o \
         obj/vidhrdw/taitof2.o obj/drivers/taitof2.o \
         obj/vidhrdw/ssi.o obj/drivers/ssi.o \
         obj/machine/slapfght.o obj/vidhrdw/slapfght.o obj/drivers/slapfght.o \
         obj/machine/twincobr.o obj/vidhrdw/twincobr.o \
         obj/drivers/twincobr.o obj/drivers/wardner.o \
         obj/machine/toaplan1.o obj/vidhrdw/toaplan1.o obj/drivers/toaplan1.o \
         obj/vidhrdw/snowbros.o obj/drivers/snowbros.o \
         obj/drivers/kyugo.o obj/vidhrdw/kyugo.o \
         obj/machine/williams.o obj/vidhrdw/williams.o obj/sndhrdw/williams.o obj/drivers/williams.o \
         obj/vidhrdw/vulgus.o obj/drivers/vulgus.o \
         obj/vidhrdw/sonson.o obj/drivers/sonson.o \
         obj/vidhrdw/higemaru.o obj/drivers/higemaru.o \
         obj/vidhrdw/1942.o obj/drivers/1942.o \
         obj/vidhrdw/exedexes.o obj/drivers/exedexes.o \
         obj/vidhrdw/commando.o obj/drivers/commando.o \
         obj/vidhrdw/gng.o obj/drivers/gng.o \
         obj/vidhrdw/gunsmoke.o obj/drivers/gunsmoke.o \
         obj/vidhrdw/srumbler.o obj/drivers/srumbler.o \
         obj/machine/lwings.o obj/vidhrdw/lwings.o obj/drivers/lwings.o \
         obj/vidhrdw/sidearms.o obj/drivers/sidearms.o \
         obj/vidhrdw/bionicc.o obj/drivers/bionicc.o \
         obj/vidhrdw/1943.o obj/drivers/1943.o \
         obj/vidhrdw/blktiger.o obj/drivers/blktiger.o \
         obj/vidhrdw/tigeroad.o obj/drivers/tigeroad.o \
         obj/vidhrdw/lastduel.o obj/drivers/lastduel.o \
         obj/vidhrdw/sf1.o obj/drivers/sf1.o \
         obj/machine/kabuki.o obj/machine/eeprom.o \
         obj/vidhrdw/mitchell.o obj/drivers/mitchell.o \
         obj/vidhrdw/cbasebal.o obj/drivers/cbasebal.o \
         obj/vidhrdw/cps1.o obj/drivers/cps1.o \
         obj/machine/capbowl.o obj/vidhrdw/capbowl.o obj/vidhrdw/tms34061.o obj/drivers/capbowl.o \
         obj/vidhrdw/blockade.o obj/drivers/blockade.o \
         obj/vidhrdw/vicdual.o obj/drivers/vicdual.o \
         obj/sndhrdw/carnival.o obj/sndhrdw/depthch.o obj/sndhrdw/invinco.o obj/sndhrdw/pulsar.o \
         obj/machine/segacrpt.o \
         obj/vidhrdw/sega.o obj/sndhrdw/sega.o obj/machine/sega.o obj/drivers/sega.o \
         obj/vidhrdw/segar.o obj/sndhrdw/segar.o obj/machine/segar.o obj/drivers/segar.o \
         obj/sndhrdw/monsterb.o \
         obj/vidhrdw/zaxxon.o obj/sndhrdw/zaxxon.o obj/drivers/zaxxon.o \
         obj/sndhrdw/congo.o obj/drivers/congo.o \
         obj/machine/turbo.o obj/vidhrdw/turbo.o obj/drivers/turbo.o obj/machine/8255ppi.o \
         obj/drivers/kopunch.o \
         obj/vidhrdw/suprloco.o obj/drivers/suprloco.o \
         obj/vidhrdw/champbas.o obj/drivers/champbas.o \
         obj/vidhrdw/appoooh.o obj/drivers/appoooh.o \
         obj/vidhrdw/bankp.o obj/drivers/bankp.o \
         obj/vidhrdw/system1.o obj/drivers/system1.o \
         obj/machine/system16.o obj/vidhrdw/system16.o obj/sndhrdw/system16.o obj/drivers/system16.o \
         obj/machine/btime.o obj/vidhrdw/btime.o obj/drivers/btime.o \
         obj/vidhrdw/astrof.o obj/sndhrdw/astrof.o obj/drivers/astrof.o \
         obj/vidhrdw/kchamp.o obj/drivers/kchamp.o \
         obj/vidhrdw/firetrap.o obj/drivers/firetrap.o \
         obj/vidhrdw/brkthru.o obj/drivers/brkthru.o \
         obj/vidhrdw/shootout.o obj/drivers/shootout.o \
         obj/vidhrdw/sidepckt.o obj/drivers/sidepckt.o \
         obj/vidhrdw/exprraid.o obj/drivers/exprraid.o \
         obj/vidhrdw/pcktgal.o obj/drivers/pcktgal.o \
         obj/vidhrdw/actfancr.o obj/drivers/actfancr.o \
         obj/vidhrdw/dec8.o obj/drivers/dec8.o \
         obj/vidhrdw/karnov.o obj/drivers/karnov.o \
         obj/machine/dec0.o obj/vidhrdw/dec0.o obj/drivers/dec0.o \
         obj/vidhrdw/stadhero.o obj/drivers/stadhero.o \
		 obj/vidhrdw/madmotor.o obj/drivers/madmotor.o \
         obj/vidhrdw/vaportra.o obj/drivers/vaportra.o \
         obj/vidhrdw/cbuster.o obj/drivers/cbuster.o \
         obj/vidhrdw/darkseal.o obj/drivers/darkseal.o \
		 obj/vidhrdw/supbtime.o obj/drivers/supbtime.o \
         obj/vidhrdw/cninja.o obj/drivers/cninja.o \
         obj/vidhrdw/tumblep.o obj/drivers/tumblep.o \
		 obj/vidhrdw/funkyjet.o obj/drivers/funkyjet.o \
         obj/sndhrdw/senjyo.o obj/vidhrdw/senjyo.o obj/drivers/senjyo.o \
         obj/vidhrdw/bombjack.o obj/drivers/bombjack.o \
         obj/vidhrdw/pbaction.o obj/drivers/pbaction.o \
         obj/vidhrdw/pontoon.o obj/drivers/pontoon.o \
         obj/vidhrdw/tehkanwc.o obj/drivers/tehkanwc.o \
         obj/vidhrdw/solomon.o obj/drivers/solomon.o \
         obj/vidhrdw/tecmo.o obj/drivers/tecmo.o \
         obj/vidhrdw/gaiden.o obj/drivers/gaiden.o \
         obj/vidhrdw/wc90.o obj/drivers/wc90.o \
         obj/vidhrdw/wc90b.o obj/drivers/wc90b.o \
         obj/sndhrdw/timeplt.o \
         obj/vidhrdw/tutankhm.o obj/drivers/tutankhm.o \
         obj/drivers/junofrst.o \
         obj/vidhrdw/pooyan.o obj/drivers/pooyan.o \
         obj/vidhrdw/timeplt.o obj/drivers/timeplt.o \
         obj/vidhrdw/megazone.o obj/drivers/megazone.o \
         obj/vidhrdw/pandoras.o obj/drivers/pandoras.o \
         obj/sndhrdw/gyruss.o obj/vidhrdw/gyruss.o obj/drivers/gyruss.o \
         obj/machine/konami.o obj/vidhrdw/trackfld.o obj/sndhrdw/trackfld.o obj/drivers/trackfld.o \
		 obj/vidhrdw/rocnrope.o obj/drivers/rocnrope.o \
         obj/vidhrdw/circusc.o obj/drivers/circusc.o \
         obj/machine/tp84.o obj/vidhrdw/tp84.o obj/drivers/tp84.o \
         obj/vidhrdw/hyperspt.o obj/drivers/hyperspt.o \
         obj/vidhrdw/sbasketb.o obj/drivers/sbasketb.o \
         obj/vidhrdw/mikie.o obj/drivers/mikie.o \
         obj/vidhrdw/yiear.o obj/drivers/yiear.o \
         obj/vidhrdw/shaolins.o obj/drivers/shaolins.o \
         obj/vidhrdw/pingpong.o obj/drivers/pingpong.o \
         obj/vidhrdw/gberet.o obj/drivers/gberet.o \
         obj/vidhrdw/jailbrek.o obj/drivers/jailbrek.o \
         obj/vidhrdw/finalizr.o obj/drivers/finalizr.o \
         obj/vidhrdw/ironhors.o obj/drivers/ironhors.o \
         obj/machine/jackal.o obj/vidhrdw/jackal.o obj/drivers/jackal.o \
         obj/machine/ddrible.o obj/vidhrdw/ddrible.o obj/drivers/ddrible.o \
         obj/vidhrdw/contra.o obj/drivers/contra.o \
         obj/machine/combatsc.o obj/vidhrdw/combatsc.o obj/drivers/combatsc.o \
		 obj/vidhrdw/hcastle.o obj/drivers/hcastle.o \
         obj/vidhrdw/nemesis.o obj/drivers/nemesis.o \
         obj/vidhrdw/konamiic.o \
         obj/vidhrdw/rockrage.o obj/drivers/rockrage.o \
		 obj/vidhrdw/flkatck.o obj/drivers/flkatck.o \
		 obj/vidhrdw/battlnts.o obj/drivers/battlnts.o \
         obj/vidhrdw/bladestl.o obj/drivers/bladestl.o \
         obj/machine/ajax.o obj/vidhrdw/ajax.o obj/drivers/ajax.o \
         obj/vidhrdw/thunderx.o obj/drivers/thunderx.o \
         obj/vidhrdw/mainevt.o obj/drivers/mainevt.o \
         obj/vidhrdw/88games.o obj/drivers/88games.o \
		 obj/vidhrdw/gbusters.o obj/drivers/gbusters.o \
         obj/vidhrdw/crimfght.o obj/drivers/crimfght.o \
         obj/vidhrdw/bottom9.o obj/drivers/bottom9.o \
		 obj/vidhrdw/blockhl.o obj/drivers/blockhl.o \
         obj/vidhrdw/aliens.o obj/drivers/aliens.o \
         obj/vidhrdw/surpratk.o obj/drivers/surpratk.o \
         obj/vidhrdw/parodius.o obj/drivers/parodius.o \
         obj/vidhrdw/rollerg.o obj/drivers/rollerg.o \
         obj/vidhrdw/xexex.o obj/drivers/xexex.o \
         obj/machine/simpsons.o obj/vidhrdw/simpsons.o obj/drivers/simpsons.o \
         obj/vidhrdw/vendetta.o obj/drivers/vendetta.o \
		 obj/vidhrdw/twin16.o obj/drivers/twin16.o \
         obj/vidhrdw/tmnt.o obj/drivers/tmnt.o \
         obj/vidhrdw/xmen.o obj/drivers/xmen.o \
         obj/vidhrdw/wecleman.o obj/drivers/wecleman.o \
         obj/machine/exidy.o obj/vidhrdw/exidy.o obj/sndhrdw/exidy.o obj/drivers/exidy.o \
         obj/sndhrdw/targ.o \
         obj/vidhrdw/circus.o obj/drivers/circus.o \
         obj/machine/starfire.o obj/vidhrdw/starfire.o obj/drivers/starfire.o \
         obj/sndhrdw/exidy440.o obj/vidhrdw/exidy440.o obj/drivers/exidy440.o \
         obj/machine/atari_vg.o \
         obj/machine/asteroid.o obj/sndhrdw/asteroid.o \
         obj/vidhrdw/llander.o obj/drivers/asteroid.o \
         obj/drivers/bwidow.o \
         obj/sndhrdw/bzone.o  obj/drivers/bzone.o \
         obj/sndhrdw/redbaron.o \
         obj/drivers/tempest.o \
         obj/machine/starwars.o obj/machine/swmathbx.o \
         obj/drivers/starwars.o obj/sndhrdw/starwars.o \
         obj/machine/mhavoc.o obj/drivers/mhavoc.o \
         obj/machine/quantum.o obj/drivers/quantum.o \
         obj/machine/atarifb.o obj/vidhrdw/atarifb.o obj/drivers/atarifb.o \
         obj/machine/sprint2.o obj/vidhrdw/sprint2.o obj/drivers/sprint2.o \
         obj/machine/sbrkout.o obj/vidhrdw/sbrkout.o obj/drivers/sbrkout.o \
         obj/machine/dominos.o obj/vidhrdw/dominos.o obj/drivers/dominos.o \
         obj/vidhrdw/nitedrvr.o obj/machine/nitedrvr.o obj/drivers/nitedrvr.o \
         obj/vidhrdw/bsktball.o obj/machine/bsktball.o obj/drivers/bsktball.o \
         obj/vidhrdw/copsnrob.o obj/machine/copsnrob.o obj/drivers/copsnrob.o \
         obj/machine/avalnche.o obj/vidhrdw/avalnche.o obj/drivers/avalnche.o \
         obj/machine/subs.o obj/vidhrdw/subs.o obj/drivers/subs.o \
         obj/vidhrdw/canyon.o obj/drivers/canyon.o \
         obj/vidhrdw/skydiver.o obj/drivers/skydiver.o \
         obj/vidhrdw/warlord.o obj/drivers/warlord.o \
         obj/machine/centiped.o obj/vidhrdw/centiped.o obj/drivers/centiped.o \
         obj/machine/milliped.o obj/vidhrdw/milliped.o obj/drivers/milliped.o \
         obj/vidhrdw/qwakprot.o obj/drivers/qwakprot.o \
         obj/machine/kangaroo.o obj/vidhrdw/kangaroo.o obj/drivers/kangaroo.o \
         obj/machine/arabian.o obj/vidhrdw/arabian.o obj/drivers/arabian.o \
         obj/machine/missile.o obj/vidhrdw/missile.o obj/drivers/missile.o \
         obj/machine/foodf.o obj/vidhrdw/foodf.o obj/drivers/foodf.o \
         obj/vidhrdw/liberatr.o obj/machine/liberatr.o obj/drivers/liberatr.o \
         obj/vidhrdw/ccastles.o obj/drivers/ccastles.o \
         obj/machine/cloak.o obj/vidhrdw/cloak.o obj/drivers/cloak.o \
         obj/vidhrdw/cloud9.o obj/drivers/cloud9.o \
         obj/machine/jedi.o obj/vidhrdw/jedi.o obj/sndhrdw/jedi.o obj/drivers/jedi.o \
         obj/machine/atarigen.o obj/sndhrdw/atarijsa.o \
         obj/machine/slapstic.o \
         obj/vidhrdw/atarisy1.o obj/drivers/atarisy1.o \
         obj/vidhrdw/atarisy2.o obj/drivers/atarisy2.o \
         obj/vidhrdw/gauntlet.o obj/drivers/gauntlet.o \
         obj/vidhrdw/atetris.o obj/drivers/atetris.o \
         obj/vidhrdw/toobin.o obj/drivers/toobin.o \
         obj/vidhrdw/vindictr.o obj/drivers/vindictr.o \
         obj/vidhrdw/klax.o obj/drivers/klax.o \
         obj/vidhrdw/blstroid.o obj/drivers/blstroid.o \
         obj/vidhrdw/xybots.o obj/drivers/xybots.o \
         obj/vidhrdw/eprom.o obj/drivers/eprom.o \
         obj/vidhrdw/skullxbo.o obj/drivers/skullxbo.o \
         obj/vidhrdw/badlands.o obj/drivers/badlands.o \
         obj/vidhrdw/cyberbal.o obj/drivers/cyberbal.o \
         obj/vidhrdw/rampart.o obj/drivers/rampart.o \
         obj/vidhrdw/shuuz.o obj/drivers/shuuz.o \
         obj/vidhrdw/hydra.o obj/drivers/hydra.o \
         obj/vidhrdw/thunderj.o obj/drivers/thunderj.o \
         obj/vidhrdw/batman.o obj/drivers/batman.o \
         obj/vidhrdw/relief.o obj/drivers/relief.o \
         obj/vidhrdw/offtwall.o obj/drivers/offtwall.o \
         obj/vidhrdw/arcadecl.o obj/drivers/arcadecl.o \
         obj/vidhrdw/rockola.o obj/sndhrdw/rockola.o obj/drivers/rockola.o \
         obj/vidhrdw/warpwarp.o obj/drivers/warpwarp.o \
         obj/drivers/munchmo.o \
         obj/vidhrdw/marvins.o obj/drivers/marvins.o \
         obj/vidhrdw/snk.o obj/drivers/snk.o \
         obj/vidhrdw/snk68.o obj/drivers/snk68.o \
         obj/vidhrdw/prehisle.o obj/drivers/prehisle.o \
         obj/vidhrdw/alpha68k.o obj/drivers/alpha68k.o \
         obj/drivers/scregg.o \
         obj/vidhrdw/tagteam.o obj/drivers/tagteam.o \
         obj/vidhrdw/ssozumo.o obj/drivers/ssozumo.o \
         obj/vidhrdw/mystston.o obj/drivers/mystston.o \
		 obj/vidhrdw/bogeyman.o obj/drivers/bogeyman.o \
         obj/vidhrdw/matmania.o obj/drivers/matmania.o obj/machine/maniach.o \
         obj/vidhrdw/renegade.o obj/drivers/renegade.o \
         obj/vidhrdw/xain.o obj/drivers/xain.o \
         obj/vidhrdw/battlane.o obj/drivers/battlane.o \
         obj/vidhrdw/ddragon.o obj/drivers/ddragon.o \
		 obj/vidhrdw/ddragon3.o obj/drivers/ddragon3.o \
         obj/vidhrdw/blockout.o obj/drivers/blockout.o \
         obj/machine/berzerk.o obj/vidhrdw/berzerk.o obj/sndhrdw/berzerk.o obj/drivers/berzerk.o \
         obj/vidhrdw/gameplan.o obj/drivers/gameplan.o \
         obj/vidhrdw/route16.o obj/drivers/route16.o \
         obj/vidhrdw/zaccaria.o obj/drivers/zaccaria.o \
         obj/vidhrdw/nova2001.o obj/drivers/nova2001.o \
         obj/vidhrdw/pkunwar.o obj/drivers/pkunwar.o \
         obj/vidhrdw/ninjakd2.o obj/drivers/ninjakd2.o \
         obj/vidhrdw/mnight.o obj/drivers/mnight.o \
         obj/machine/exterm.o obj/vidhrdw/exterm.o obj/drivers/exterm.o \
         obj/machine/smashtv.o obj/vidhrdw/smashtv.o obj/drivers/smashtv.o \
         obj/vidhrdw/jack.o obj/drivers/jack.o \
         obj/sndhrdw/cinemat.o obj/drivers/cinemat.o \
         obj/machine/cchasm.o obj/vidhrdw/cchasm.o obj/sndhrdw/cchasm.o obj/drivers/cchasm.o \
         obj/vidhrdw/thepit.o obj/drivers/thepit.o \
         obj/machine/bagman.o obj/vidhrdw/bagman.o obj/drivers/bagman.o \
         obj/vidhrdw/wiz.o obj/drivers/wiz.o \
         obj/machine/stfight.o obj/vidhrdw/stfight.o obj/drivers/stfight.o \
         obj/vidhrdw/dynduke.o obj/drivers/dynduke.o \
         obj/vidhrdw/raiden.o obj/drivers/raiden.o \
         obj/vidhrdw/dcon.o obj/drivers/dcon.o \
         obj/vidhrdw/cabal.o obj/drivers/cabal.o \
         obj/vidhrdw/toki.o obj/drivers/toki.o \
         obj/vidhrdw/bloodbro.o obj/drivers/bloodbro.o \
         obj/vidhrdw/exerion.o obj/drivers/exerion.o \
         obj/vidhrdw/aeroboto.o obj/drivers/aeroboto.o \
         obj/vidhrdw/citycon.o obj/drivers/citycon.o \
         obj/vidhrdw/psychic5.o obj/drivers/psychic5.o \
         obj/vidhrdw/ginganin.o obj/drivers/ginganin.o \
         obj/vidhrdw/megasys1.o obj/drivers/megasys1.o \
		 obj/vidhrdw/cischeat.o obj/drivers/cischeat.o \
         obj/vidhrdw/aerofgt.o obj/drivers/aerofgt.o \
         obj/machine/8254pit.o obj/vidhrdw/leland.o obj/drivers/leland.o \
         obj/drivers/ataxx.o \
         obj/vidhrdw/marineb.o obj/drivers/marineb.o \
         obj/vidhrdw/funkybee.o obj/drivers/funkybee.o \
         obj/vidhrdw/zodiack.o obj/drivers/zodiack.o \
         obj/machine/espial.o obj/vidhrdw/espial.o obj/drivers/espial.o \
		 obj/machine/vastar.o obj/vidhrdw/vastar.o obj/drivers/vastar.o \
		 obj/vidhrdw/pong.o obj/sndhrdw/pong.o obj/drivers/pong.o \
         obj/vidhrdw/spacefb.o obj/sndhrdw/spacefb.o obj/drivers/spacefb.o \
		 obj/vidhrdw/blueprnt.o obj/drivers/blueprnt.o \
         obj/drivers/omegrace.o \
         obj/vidhrdw/tankbatt.o obj/drivers/tankbatt.o \
         obj/vidhrdw/dday.o obj/sndhrdw/dday.o obj/drivers/dday.o \
         obj/vidhrdw/gundealr.o obj/drivers/gundealr.o \
         obj/machine/leprechn.o obj/vidhrdw/leprechn.o obj/drivers/leprechn.o \
         obj/vidhrdw/hexa.o obj/drivers/hexa.o \
         obj/vidhrdw/redalert.o obj/sndhrdw/redalert.o obj/drivers/redalert.o \
         obj/machine/irobot.o obj/vidhrdw/irobot.o obj/drivers/irobot.o \
         obj/machine/spiders.o obj/vidhrdw/crtc6845.o obj/vidhrdw/spiders.o obj/drivers/spiders.o \
         obj/machine/stactics.o obj/vidhrdw/stactics.o obj/drivers/stactics.o \
         obj/vidhrdw/sharkatt.o obj/drivers/sharkatt.o \
         obj/vidhrdw/kingobox.o obj/drivers/kingobox.o \
         obj/vidhrdw/zerozone.o obj/drivers/zerozone.o \
         obj/machine/exctsccr.o obj/vidhrdw/exctsccr.o obj/drivers/exctsccr.o \
         obj/vidhrdw/speedbal.o obj/drivers/speedbal.o \
         obj/vidhrdw/sauro.o obj/drivers/sauro.o \
         obj/vidhrdw/galpanic.o obj/drivers/galpanic.o \
         obj/vidhrdw/airbustr.o obj/drivers/airbustr.o \
         obj/vidhrdw/ambush.o obj/drivers/ambush.o \
         obj/vidhrdw/starcrus.o obj/drivers/starcrus.o \
         obj/drivers/shanghai.o \
         obj/vidhrdw/goindol.o obj/drivers/goindol.o \
         obj/drivers/dlair.o \
         obj/vidhrdw/goldstar.o obj/drivers/goldstar.o \
         obj/vidhrdw/csk.o obj/drivers/csk.o \
         obj/vidhrdw/meteor.o obj/drivers/meteor.o \
         obj/vidhrdw/bjtwin.o obj/drivers/bjtwin.o \
         obj/vidhrdw/aztarac.o obj/sndhrdw/aztarac.o obj/drivers/aztarac.o \


NEO_OBJS = obj/machine/neogeo.o obj/machine/pd4990a.o obj/vidhrdw/neogeo.o obj/drivers/neogeo.o

!ifdef TINY_COMPILE
OBJS = $(TINY_OBJS)
TINYFLAGS = -DTINY_COMPILE -DTINY_NAME=$(TINY_NAME)
!else
!ifdef NEOFREE
OBJS = $(DRV_OBJS)
TINYFLAGS = -DNEOFREE
!else
!ifdef NEOMAME
OBJS = $(NEO_OBJS)
TINYFLAGS = -DNEOMAME
!else
OBJS = $(DRV_OBJS) $(NEO_OBJS)
TINYFLAGS =
!endif
!endif
!endif
CFLAGS = $(CFLAGS) $(TINYFLAGS)

RES    = # obj/Win32/MAME32.res


!ifdef HELPFILE
all: mame32.exe $(HELPFILE)
!else
all: mame32.exe
!endif

# workaround for vc4.2 compiler optimization bug:
!ifndef DEBUG
obj/vidhrdw/timeplt.o: src/vidhrdw/timeplt.c
	$(CC) $(DEFS) $(CFLAGSGLOBAL) -Oi -Ot -Oy -Ob1 -Gs -G5 -Gr -Fo$@ -c src/vidhrdw/timeplt.c
!endif

mame32.exe: $(COREOBJS) $(WIN32_OBJS) $(OBJS) $(RES)
	$(LD) @<<
        $(LDFLAGS) -out:mame32.exe $(COREOBJS) $(WIN32_OBJS) $(OBJS) $(RES) $(LIBS) $(NET_LIBS)
<<

!ifdef HELPFILE
$(HELPFILE): src\Win32\Hlp\Mame32.cnt src\Win32\Hlp\Mame32.hlp
	@Makehelp.bat
!endif

.asm.oa:
	$(ASM) -o $@ $(ASMFLAGS) $(ASMDEFS) $<

{src}.c{obj}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/gensync}.c{obj/cpu/gensync}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/z80}.c{obj/cpu/z80}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/z8000}.c{obj/cpu/z8000}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/m6502}.c{obj/cpu/m6502}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/h6280}.c{obj/cpu/h6280}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/i86}.c{obj/cpu/i86}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/nec}.c{obj/cpu/nec}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/i8039}.c{obj/cpu/i8039}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/i8085}.c{obj/cpu/i8085}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/m6809}.c{obj/cpu/m6809}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/konami}.c{obj/cpu/konami}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/m68000}.c{obj/cpu/m68000}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/s2650}.c{obj/cpu/s2650}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/T11}.c{obj/cpu/T11}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/tms34010}.c{obj/cpu/tms34010}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/tms32010}.c{obj/cpu/tms32010}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/tms9900}.c{obj/cpu/tms9900}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/m6800}.c{obj/cpu/m6800}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/m6805}.c{obj/cpu/m6805}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/cpu/ccpu}.c{obj/cpu/ccpu}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/vidhrdw}.c{obj/vidhrdw}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/sndhrdw}.c{obj/sndhrdw}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/machine}.c{obj/machine}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/drivers}.c{obj/drivers}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/sound}.c{obj/sound}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/Win32}.c{obj/Win32}.o:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

{src/Win32}.rc{obj/Win32}.res:
	$(RSC) $(RCFLAGS) -Fo$@ $<

obj/cpu/m6800/m6800.asm:  src/cpu/m6800/make6808.c
	$(CC) /Feobj/cpu/6800/make6808.exe /Foobj/cpu/m6800/make6808.obj src/cpu/m6800/make6808.c
	obj\\cpu\\m6800\\make6808 $@ -m -h -w

CFLAGS_MAKE_68K = $(CFLAGS) -DWIN32
!ifdef USE_FASTCALL
CFLAGS_MAKE_68K = $(CFLAGS_MAKE_68K) -DFASTCALL
!endif

# generate C source files for the 68000 emulator
obj/cpu/m68000/m68kops.h \
obj/cpu/m68000/m68kops.c \
obj/cpu/m68000/m68kopac.c \
obj/cpu/m68000/m68kopdm.c \
obj/cpu/m68000/m68kopnz.c : src/cpu/m68000/m68kmake.c src/cpu/m68000/m68k_in.c
	$(CC) $(CFLAGS_MAKE_68K) /Feobj/cpu/m68000/m68kmake.exe /Foobj/cpu/m68000/m68kmake.obj \
	src/cpu/m68000/m68kmake.c
	obj\\cpu\\m68000\\m68kmake obj/cpu/m68000 src/cpu/m68000/m68k_in.c

# generated C files for the 68000 emulator
{obj/cpu/m68000}.c{obj/cpu/m68000}.og:
	$(CC) $(DEFS) $(CFLAGS) -Fo$@ -c $<

# generate asm source files for the 68000 emulator
obj/cpu/m68000/68kem.asm:  src/cpu/m68000/make68k.c
	$(CC) $(CFLAGS_MAKE_68K) /Feobj/cpu/m68000/make68k.exe /Foobj/cpu/m68000/make68k.obj \
	src/cpu/m68000/make68k.c
	obj\\cpu\\m68000\\make68k $@

obj/cpu/z80/z80.asm:  src/cpu/z80/makez80.c
	$(CC) $(CDEFS) $(CFLAGS) -Foobj/cpu/z80/makez80.exe src/cpu/z80/makez80.c
	obj\\cpu\\z80\\makez80 $(Z80DEF) $(CDEFS) $(CFLAGS) $@

# dependencies
obj/cpu/z80/z80.o:           src/cpu/z80/z80.c src/cpu/z80/z80.h src/cpu/z80/z80daa.h
obj/cpu/i8085/i8085.o:       src/cpu/i8085/i8085.c src/cpu/i8085/i8085.h src/cpu/i8085/i8085cpu.h src/cpu/i8085/i8085daa.h
obj/cpu/m6502/m6502.o:       src/cpu/m6502/m6502.c src/cpu/m6502/m6502.h src/cpu/m6502/m6502ops.h src/cpu/m6502/tbl6502.c src/cpu/m6502/tbl65c02.c src/cpu/m6502/tbl6510.c
obj/cpu/h6280/h6280.o:       src/cpu/h6280/h6280.c src/cpu/h6280/h6280.h src/cpu/h6280/h6280ops.h src/cpu/h6280/tblh6280.c
obj/cpu/i86/i86.o:           src/cpu/i86/i86.c src/cpu/i86/i86.h src/cpu/i86/i86intrf.h src/cpu/i86/ea.h src/cpu/i86/host.h src/cpu/i86/instr.h src/cpu/i86/modrm.h
obj/cpu/m6800/m6800.o:	     src/cpu/m6800/m6800.c src/cpu/m6800/m6800.h src/cpu/m6800/6800ops.c
obj/cpu/m6805/m6805.o:	     src/cpu/m6805/m6805.c src/cpu/m6805/m6805.h src/cpu/m6805/6805ops.c
obj/cpu/m6809/m6809.o:	     src/cpu/m6809/m6809.c src/cpu/m6809/m6809.h src/cpu/m6809/6809ops.c src/cpu/m6809/6809tbl.c
obj/cpu/tms34010/tms34010.o: src/cpu/tms34010/tms34010.c src/cpu/tms34010/tms34010.h src/cpu/tms34010/34010ops.c src/cpu/tms34010/34010tbl.c
obj/cpu/tms9900/tms9900.o:   src/cpu/tms9900/tms9900.c src/cpu/tms9900/tms9900.h src/cpu/tms9900/9900stat.h
obj/cpu/z8000/z8000.o:       src/cpu/z8000/z8000.c src/cpu/z8000/z8000.h src/cpu/z8000/z8000cpu.h src/cpu/z8000/z8000dab.h src/cpu/z8000/z8000ops.c src/cpu/z8000/z8000tbl.c
obj/cpu/tms32010/tms32010.o: src/cpu/tms32010/tms32010.c src/cpu/tms32010/tms32010.h
obj/cpu/ccpu/ccpu.o:         src/cpu/ccpu/ccpu.h src/cpu/ccpu/ccpudasm.c
obj/cpu/m68000/m68kcpu.o:    obj/cpu/m68000/m68kops.c src/cpu/m68000/m68kmake.c src/cpu/m68000/m68k_in.c

.IGNORE:

maketree:
	md obj
	md obj\cpu
	md obj\cpu\gensync
	md obj\cpu\z80
	md obj\cpu\m6502
	md obj\cpu\h6280
	md obj\cpu\i86
	md obj\cpu\nec
	md obj\cpu\i8039
	md obj\cpu\i8085
	md obj\cpu\m6800
	md obj\cpu\m6805
	md obj\cpu\m6809
	md obj\cpu\konami
	md obj\cpu\m68000
	md obj\cpu\s2650
	md obj\cpu\t11
	md obj\cpu\tms34010
	md obj\cpu\tms9900
	md obj\cpu\z8000
	md obj\cpu\tms32010
	md obj\cpu\ccpu
	md obj\cpu\pdp1
	md obj\sound
	md obj\drivers
	md obj\machine
	md obj\vidhrdw
	md obj\sndhrdw
	md obj\Win32

clean:
	del obj\*.o
	del obj\*.a
	del obj\*.obj
	del obj\*.lib
	del obj\cpu\gensync\*.o
	del obj\cpu\z80\*.o
	del obj\cpu\z80\*.oa
	del obj\cpu\z80\*.asm
	del obj\cpu\z80\*.exe
	del obj\cpu\m6502\*.o
	del obj\cpu\h6280\*.o
	del obj\cpu\i86\*.o
	del obj\cpu\nec\*.o
	del obj\cpu\i8039\*.o
	del obj\cpu\i8085\*.o
	del obj\cpu\m6800\*.o
	del obj\cpu\m6800\*.oa
	del obj\cpu\m6800\*.exe
	del obj\cpu\m6805\*.o
	del obj\cpu\m6809\*.o
	del obj\cpu\konami\*.o
	del obj\cpu\m68000\*.o
	del obj\cpu\m68000\*.c
	del obj\cpu\m68000\*.h
	del obj\cpu\m68000\*.oa
	del obj\cpu\m68000\*.og
	del obj\cpu\m68000\*.asm
	del obj\cpu\m68000\*.exe
	del obj\cpu\s2650\*.o
	del obj\cpu\t11\*.o
	del obj\cpu\tms34010\*.o
	del obj\cpu\tms9900\*.o
	del obj\cpu\z8000\*.o
	del obj\cpu\tms32010\*.o
	del obj\cpu\ccpu\*.o
	del obj\cpu\pdp1\*.o
	del obj\sound\*.o
	del obj\drivers\*.o
	del obj\machine\*.o
	del obj\vidhrdw\*.o
	del obj\sndhrdw\*.o
	del obj\Win32\*.o
	del obj\Win32\*.res
        del obj\*.o
	del mame32.exe
!ifdef HELPFILE
	del mame32.hlp
	del mame32.cnt
!endif

cleandebug:
	del obj\*.o
	del obj\cpu\gensync\*.o
	del obj\cpu\z80\*.o
	del obj\cpu\z80\*.oa
	del obj\cpu\z80\*.asm
	del obj\cpu\z80\*.exe
	del obj\cpu\m6502\*.o
	del obj\cpu\h6280\*.o
	del obj\cpu\i86\*.o
	del obj\cpu\nec\*.0
	del obj\cpu\i8039\*.o
	del obj\cpu\i8085\*.o
	del obj\cpu\m6800\*.o
	del obj\cpu\m6800\*.oa
	del obj\cpu\m6800\*.exe
	del obj\cpu\m6805\*.o
	del obj\cpu\m6809\*.o
	del obj\cpu\konami\*.o
	del obj\cpu\m68000\*.o
	del obj\cpu\m68000\*.c
	del obj\cpu\m68000\*.h
	del obj\cpu\m68000\*.oa
	del obj\cpu\m68000\*.og
	del obj\cpu\m68000\*.asm
	del obj\cpu\m68000\*.exe
	del obj\cpu\s2650\*.o
	del obj\cpu\t11\*.o
	del obj\cpu\tms34010\*.o
	del obj\cpu\tms9900\*.o
	del obj\cpu\z8000\*.o
	del obj\cpu\tms32010\*.o
	del obj\cpu\ccpu\*.o
	del obj\cpu\pdp1\*.o
	del mame32.exe

cleantiny:
	del obj\mame.o
	del obj\driver.o
	del obj\usrintrf.o
	del obj\cheat.o
	del obj\Win32\*.o
