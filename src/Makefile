.PHONY: all install uninstall clean

CFLAGS = -ansi -pedantic -Wall $(shell pkg-config --cflags clutter-1.0)
LIBS   = $(shell pkg-config --libs clutter-1.0)

all: cube-twister

cube-twister: cube-twister.o clutterrific.o
	gcc $(LIBS) -o cube-twister cube-twister.o clutterrific.o

install:
	cp cube-twister /usr/lib/gnome-screensaver/gnome-screensaver/
	cp cube-twister.desktop /usr/share/applications/screensavers/
	cp -r data/ /usr/lib/gnome-screensaver/gnome-screensaver/

uninstall:
	-rm -r /usr/lib/gnome-screensaver/gnome-screensaver/data/
	-rm /usr/lib/gnome-screensaver/gnome-screensaver/cube-twister
	-rm /usr/share/applications/screensavers/cube-twister.desktop

clean:
	-rm cube-twister *.o
