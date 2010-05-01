.PHONY: all clean

CFLAGS = -ansi -pedantic $(shell pkg-config --cflags clutter-1.0)
LIBS   = $(shell pkg-config --libs clutter-1.0)

all: cube

cube: cube.o
	gcc $(LIBS) -o cube cube.o

clean:
	-rm cube cube.o
