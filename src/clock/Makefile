.PHONY: all clean

CFLAGS = -ansi -pedantic $(shell pkg-config --cflags clutter-1.0)
LIBS   = $(shell pkg-config --libs clutter-1.0)

all: clock

clock: clock.o
	gcc $(LIBS) -o clock clock.o

clean:
	-rm clock clock.o
