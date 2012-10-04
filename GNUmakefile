CC = $(CROSS)gcc
LD = $(CROSS)gcc

CFLAGS = -I. -Os -g0 -std=gnu99 -march=i686 -mfpmath=sse -msse -ffast-math -fexcess-precision=fast
LDFLAGS = -shared -Wl,--dll,--add-stdcall-alias -Wl,-s -L.

.Phony: all clean

all: p216reader.aui

p216reader.aui: p216reader.o
	$(LD) $(LDFLAGS) $(XLDFLAGS) -o $@ $^ -lavifil32

p216reader.o: p216reader.c input.h
	$(CC) -c $(CFLAGS) $(XCFLAGS) -o $@ $<

clean:
	$(RM) *.o *.aui
