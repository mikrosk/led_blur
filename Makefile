include source/makefile.part
bin = ledblur.tos

warn = -Wall -Wno-unused-variable
opt = -O2 -m68020-60 -fomit-frame-pointer

PREFIX = $(shell $(CC) -print-sysroot)/usr

CC = m68k-atari-mint-gcc
CFLAGS = -std=gnu99 -pedantic $(warn) $(opt) -Isource `$(PREFIX)/bin/m68020-60/sdl-config --cflags` #`libmikmod-config --cflags`
LDFLAGS = -s -m68020-60 `$(PREFIX)/bin/m68020-60/sdl-config --libs` #`libmikmod-config --libs`

$(bin):	$(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
