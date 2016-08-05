prefix?=/usr/lib
export prefix

all: ./make
	./make

clean: ./make
	clean=1 ./make
	rm -f make make.o

make: make.o
