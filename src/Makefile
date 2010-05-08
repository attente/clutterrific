.PHONY: all clean

CFLAGS = -ansi -pedantic $(shell pkg-config --cflags clutter-1.0)
LIBS   = $(shell pkg-config --libs clutter-1.0)

all: cube

cube: cube.o clutterrific.o
	gcc $(LIBS) -o cube cube.o clutterrific.o

clean:
	-rm cube *.o
