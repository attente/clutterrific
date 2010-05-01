.PHONY: all clean

CFLAGS = -ansi -pedantic $(shell pkg-config --cflags clutter-1.0)
LIBS   = $(shell pkg-config --libs clutter-1.0)

all: test

test: test.o
	gcc $(LIBS) -o test test.o

clean:
	-rm test test.o
