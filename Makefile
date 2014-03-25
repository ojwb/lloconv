lloconv: lloconv.o shim.o
	g++ -o lloconv lloconv.o shim.o -ldl

lloconv.o: lloconv.cc liblibreoffice.h liblibreoffice.hxx
	g++ -I. -c -W -Wall -O2 -g lloconv.cc -o lloconv.o

shim.o: shim.c
	gcc -I. -DLINUX -c -W -Wall -O2 -g shim.c -o shim.o

clean:
	rm -f lloconv lloconv.o shim.o
