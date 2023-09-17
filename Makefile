include source/makefile.part

TARGET	:= ledblur.tos

TOOL_PREFIX := m68k-atari-mintelf
CC		:= ${TOOL_PREFIX}-gcc

warn	:= -Wall -Wno-unused-variable
opt		:= -O2 -m68020-60 -fomit-frame-pointer
#opt		:= -O2 -mcpu=5475 -fomit-frame-pointer

PREFIX	:= $(shell $(CC) -print-sysroot)/usr

SDL_CFLAGS	:= $(shell $(PREFIX)/bin/m68020-60/sdl-config --cflags)
SDL_LIBS	:= $(shell $(PREFIX)/bin/m68020-60/sdl-config --libs)
#SDL_CFLAGS	:= $(shell $(PREFIX)/bin/m5475/sdl-config --cflags)
#SDL_LIBS	:= $(shell $(PREFIX)/bin/m5475/sdl-config --libs)

CFLAGS	:= -std=gnu99 -pedantic $(warn) $(opt) -Isource $(SDL_CFLAGS)
LDFLAGS	:= -s -m68020-60
#LDFLAGS	:= -s -mcpu=5475
LDLIBS	:= $(SDL_LIBS) -lxmp-lite -lm

$(TARGET): $(obj)
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)
	${TOOL_PREFIX}-flags -S $@

.PHONY: clean
clean:
	rm -f $(obj) $(TARGET) *~
