OBJS = obj\cpu01.lib obj\cpu02.lib obj\cpu03.lib obj\cpu04.lib obj\cpu05.lib obj\cpu06.lib obj\cpu07.lib obj\cpu08.lib \
	obj\cpu09.lib obj\cpu10.lib obj\cpu11.lib obj\cpu12.lib obj\cpu13.lib obj\cpu15.lib obj\cpu16.lib obj\cpu17.lib \
	obj\cpu18.lib obj\cpu19.lib obj\cpu20.lib obj\cpu21.lib obj\drivers.lib obj\machine.lib obj\obj.lib obj\sndhrdw.lib \
	obj\sound.lib obj\vidhrdw.lib obj\win32.lib

LIBS = kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib  winmm.lib shell32.lib \
	vfw32.lib ZLIB\zlib.lib

mame.exe:
	link -machine:ppc -out:mame32.exe obj\Win32ui.obj $(OBJS) $(LIBS)
