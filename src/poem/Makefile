.PHONY: all clean

CFLAGS = -ansi -pedantic $(shell pkg-config --cflags clutter-1.0)
LIBS   = $(shell pkg-config --libs clutter-1.0)

all: poem

poem: poem.o
	gcc $(LIBS) -o poem poem.o

clean:
	-rm poem poem.o
