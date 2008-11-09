#   Makefile - universe project
#   version 2
#   Richard Vaughan  

# this should work on Linux
#GLUTLIBS = -L/usr/X11R6/lib -lGLU -lGL -lglut -lX11 -lXext -lXmu -lXi
#GLUTFLAGS = -I/usr/local/include/GL

# this works on Mac OS X
GLUT = -framework OpenGL -framework GLUT

GLIB = `pkg-config --cflags --libs glib-2.0`

CC = gcc
#CFLAGS = -g -Wall $(GLUTFLAGS)
#LIBS =  $(GLIB) $(GLUTLIBS) -lm
CFLAGS = -g -Wall $(GLUT)
LIBS =  $(GLIB) -lm

SRC = universe.h universe.c controller.c

all: universe

universe: $(SRC)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(SRC) 

clean:
	rm *.o universe

