# Idea from: https://wiki.hpcc.msu.edu/display/~colbrydi@msu.edu/2010/08/19/Cmake+Makefile 

all: build make 

make: build/Makefile 
	(cd build; make; cp ./universe ..;)

build/Makefile: build
	(cd build; cmake ..;)

build:   
	mkdir build

clean:
	rm -f  -r build
	rm -f ./universe
