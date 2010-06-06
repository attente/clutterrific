.PHONY: all install uninstall clean

CFLAGS = -ansi -pedantic -Wall $(shell pkg-config --cflags clutter-1.0)
LIBS   = -lm $(shell pkg-config --libs clutter-1.0)

all: cube-twist glow-clock

cube-twist: cube-twist.o clutterrific.o
	gcc $(LIBS) -o cube-twist cube-twist.o clutterrific.o

glow-clock: glow-clock.o clutterrific.o
	gcc $(LIBS) -o glow-clock glow-clock.o clutterrific.o

install:
	cp cube-twist /usr/lib/gnome-screensaver/gnome-screensaver/
	cp cube-twist.desktop /usr/share/applications/screensavers/
	cp glow-clock /usr/lib/gnome-screensaver/gnome-screensaver/
	cp glow-clock.desktop /usr/share/applications/screensavers/
	cp -r data/ /usr/lib/gnome-screensaver/gnome-screensaver/

uninstall:
	-rm -r /usr/lib/gnome-screensaver/gnome-screensaver/data/
	-rm /usr/lib/gnome-screensaver/gnome-screensaver/cube-twist
	-rm /usr/share/applications/screensavers/cube-twist.desktop
	-rm /usr/lib/gnome-screensaver/gnome-screensaver/glow-clock
	-rm /usr/share/applications/screensavers/glow-clock.desktop

clean:
	-rm cube-twist glow-clock *.o
