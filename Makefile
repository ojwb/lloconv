CC = gcc
CXX = g++

lloconv: lloconv.o shim.o
	$(CXX) -o lloconv lloconv.o shim.o -ldl

lloconv.o: lloconv.cc \
    liblibreoffice.h liblibreoffice.hxx \
    LibreOfficeKit.h LibreOfficeKit.hxx
	$(CXX) -I. -c -W -Wall -O2 -g lloconv.cc -o lloconv.o

shim.o: shim.c
	$(CC) -I. -DLINUX -c -W -Wall -O2 -g shim.c -o shim.o

clean:
	rm -f lloconv *.o
