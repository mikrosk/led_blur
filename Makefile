include source/makefile.part
bin = ledblur.tos

warn = -Wall -Wno-unused-variable
opt = -O2 -m68020-60 -fomit-frame-pointer
#opt = -O2 -mcpu=5475 -fomit-frame-pointer

PREFIX = $(shell $(CC) -print-sysroot)/usr

CC = m68k-atari-mint-gcc
CFLAGS = -std=gnu99 -pedantic $(warn) $(opt) -Isource `$(PREFIX)/bin/m68020-60/sdl-config --cflags` #`libmikmod-config --cflags`
LDFLAGS = -s -m68020-60 `$(PREFIX)/bin/m68020-60/sdl-config --libs` #`libmikmod-config --libs`
#CFLAGS = -std=gnu99 -pedantic $(warn) $(opt) -Isource `$(PREFIX)/bin/m475/sdl-config --cflags`
#LDFLAGS = -s -mcpu=5475 `$(PREFIX)/bin/m5475/sdl-config --libs`

$(bin):	$(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)
	m68k-atari-mint-flags -S $@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
