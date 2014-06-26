lloconv: lloconv.o shim.o shim-llo.o
	g++ -o lloconv lloconv.o shim.o shim-llo.o -ldl

lloconv.o: lloconv.cc \
    liblibreoffice.h liblibreoffice.hxx \
    LibreOfficeKit.h LibreOfficeKit.hxx
	g++ -I. -c -W -Wall -O2 -g lloconv.cc -o lloconv.o

shim.o: shim.c
	gcc -I. -DLINUX -c -W -Wall -O2 -g shim.c -o shim.o

shim-llo.o: shim-llo.c
	gcc -I. -DLINUX -c -W -Wall -O2 -g shim-llo.c -o shim-llo.o

clean:
	rm -f lloconv *.o
