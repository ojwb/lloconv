CC = gcc
CXX = g++

all: lloconv inject-meta

lloconv: lloconv.o convert.o shim.o
	$(CXX) -o lloconv lloconv.o convert.o shim.o -ldl

inject-meta: inject-meta.o convert.o shim.o
	$(CXX) -o inject-meta inject-meta.o convert.o shim.o -ldl

lloconv.o: lloconv.cc convert.h
	$(CXX) -I. -c -W -Wall -O2 -g lloconv.cc -o lloconv.o

inject-meta.o: inject-meta.cc convert.h
	$(CXX) -I. -c -W -Wall -O2 -g inject-meta.cc -o inject-meta.o

convert.o: convert.cc convert.h \
    liblibreoffice.h liblibreoffice.hxx \
    LibreOfficeKit.h LibreOfficeKit.hxx
	$(CXX) -I. -c -W -Wall -O2 -g convert.cc -o convert.o

shim.o: shim.c
	$(CC) -I. -DLINUX -c -W -Wall -O2 -g shim.c -o shim.o

clean:
	rm -f lloconv inject-meta *.o
