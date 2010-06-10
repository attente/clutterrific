AM_CPPFLAGS = $(CLUTTER_CFLAGS)
AM_CFLAGS   = -ansi -pedantic -Wall

bindir  = $(GNOME_SCREENSAVER_PRIVLIBEXECDIR)
metadir = $(GNOME_SCREENSAVER_THEMESDIR)

bin_PROGRAMS = cube-twist glow-clock
meta_DATA    = cube-twist.desktop glow-clock.desktop

cube_twist_SOURCES = cube-twist.c clutterrific.c
cube_twist_LDADD   = $(LIBS) $(CLUTTER_LIBS)

glow_clock_SOURCES = glow-clock.c clutterrific.c
glow_clock_LDADD   = $(LIBS) $(CLUTTER_LIBS)

cube-twist.desktop: cube-twist.desktop.in
	cat $^ | sed -e 's|[@]BINDIR[@]|$(bindir)|g' > $@

glow-clock.desktop: glow-clock.desktop.in
	cat $^ | sed -e 's|[@]BINDIR[@]|$(bindir)|g' > $@

MOSTLYCLEANFILES = cube-twist.desktop glow-clock.desktop
