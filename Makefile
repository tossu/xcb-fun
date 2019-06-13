CFLAGS = -Wall -Wextra -pedantic
LIBS = `pkg-config --cflags --libs xcb`

.PHONY: draw stdin image main font 3d

image:
	gcc $(CFLAGS) -lxcb-icccm -lxcb -lxcb-image -lpng -o image image.c

xcb:
	gcc $(CFLAGS) xcb.c -o xcb $(LIBS)

font:
	gcc $(CFLAGS) font.c -o font $(LIBS) -std=c99

simple:
	gcc $(CFLAGS) simple.c -o simple $(LIBS)

lol:
	gcc $(CFLAGS) lol.c -o lol $(LIBS)

white:
	gcc $(CFLAGS) white.c -o white $(LIBS)

solid:
	gcc $(CFLAGS) solid.c -o solid $(LIBS)

pixmap:
	gcc $(CFLAGS) pixmap.c -o pixmap $(LIBS)

draw:
	gcc $(CFLAGS) draw.c -o draw $(LIBS)

3d:
	gcc $(CFLAGS) 3d.c -lm -o 3d $(LIBS) -lm -pthread

main:
	gcc $(CFLAGS) main.c -o main $(LIBS)

stdin:
	gcc $(CFLAGS) stdin.c -o stdin

clean:
	rm -f white solid pixmap xcb font simple lol draw stdin
