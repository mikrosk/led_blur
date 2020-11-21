include source/makefile.part
bin = opti_ledblur

warn = -Wall -Wno-unused-variable
opt = -O3 -ffast-math -fomit-frame-pointer

CC = gcc
CFLAGS = -std=gnu99 -pedantic $(warn) $(opt) -Isource `sdl-config --cflags` `libmikmod-config --cflags`
LDFLAGS = `sdl-config --libs` `libmikmod-config --libs`

$(bin):	$(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
