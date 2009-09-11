#   Makefile - universe project
#   version 3
#   Richard Vaughan  

# this should work on Linux with MESA
#GLUTLIBS = -L/usr/X11R6/lib -lGLU -lGL -lglut -lX11 -lXext -lXmu -lXi
#GLUTFLAGS = -I/usr/local/include/GL

# this works on Mac OS X
GLUTFLAGS = -framework OpenGL -framework GLUT

CC = g++
CXXFLAGS = -g -Wall -O3 $(GLUTFLAGS)
LIBS =  -lm $(GLUTLIBS)

SRC = universe.h universe.cc controller.cc

all: universe

universe: $(SRC)
	$(CC) $(CXXFLAGS) $(LIBS) -o $@ $(SRC) 

clean:
	rm *.o universe

