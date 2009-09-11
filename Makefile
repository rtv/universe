#   Makefile - universe project
#   version 2
#   Richard Vaughan  

# this should work on Linux
#GLUTLIBS = -L/usr/X11R6/lib -lGLU -lGL -lglut -lX11 -lXext -lXmu -lXi
#GLUTFLAGS = -I/usr/local/include/GL

# this works on Mac OS X
GLUT = -framework OpenGL -framework GLUT

GLIB = `pkg-config --cflags --libs glib-2.0`

CC = g++
CXXFLAGS = -g -Wall $(GLUT)
LIBS =  $(GLIB) -lm

SRC = universe.h universe.cc controller.cc

all: universe

universe: $(SRC)
	$(CC) $(CXXFLAGS) $(LIBS) -o $@ $(SRC) 

clean:
	rm *.o universe

