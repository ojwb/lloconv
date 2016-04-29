CC = gcc
CXX = g++

all: lloconv inject-meta

lloconv: lloconv.o convert.o urlencode.o
	$(CXX) -o lloconv lloconv.o convert.o urlencode.o -ldl

inject-meta: inject-meta.o convert.o
	$(CXX) -o inject-meta inject-meta.o convert.o -ldl

lloconv.o: lloconv.cc convert.h urlencode.h
	$(CXX) -I. -c -W -Wall -O2 -g lloconv.cc -o lloconv.o

inject-meta.o: inject-meta.cc convert.h
	$(CXX) -I. -c -W -Wall -O2 -g inject-meta.cc -o inject-meta.o

urlencode.o: urlencode.cc urlencode.h
	$(CXX) -I. -c -W -Wall -O2 -g urlencode.cc -o urlencode.o

convert.o: convert.cc convert.h \
    LibreOfficeKit/LibreOfficeKit.h \
    LibreOfficeKit/LibreOfficeKit.hxx \
    LibreOfficeKit/LibreOfficeKitInit.h \
    LibreOfficeKit/LibreOfficeKitTypes.h
	$(CXX) -I. -c -W -Wall -O2 -g convert.cc -o convert.o

clean:
	rm -f lloconv inject-meta *.o
