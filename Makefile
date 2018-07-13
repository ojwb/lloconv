CC = gcc
CXX = g++

INCLUDES = -I.
ifneq ($(LO_INCLUDE_PATH),)
INCLUDES += -I$(LO_INCLUDE_PATH)
endif

all: lloconv inject-meta

lloconv: lloconv.o convert.o urlencode.o
	$(CXX) -o lloconv lloconv.o convert.o urlencode.o -ldl

inject-meta: inject-meta.o convert.o urlencode.o
	$(CXX) -o inject-meta inject-meta.o convert.o urlencode.o -ldl

lloconv.o: lloconv.cc \
    convert.h
	$(CXX) $(INCLUDES) -c -W -Wall -O2 -g lloconv.cc -o lloconv.o

inject-meta.o: inject-meta.cc \
    convert.h
	$(CXX) $(INCLUDES) -c -W -Wall -O2 -g inject-meta.cc -o inject-meta.o

urlencode.o: urlencode.cc \
    urlencode.h
	$(CXX) $(INCLUDES) -c -W -Wall -O2 -g urlencode.cc -o urlencode.o

convert.o: convert.cc \
    convert.h \
    urlencode.h
	$(CXX) $(INCLUDES) -c -W -Wall -O2 -g convert.cc -o convert.o

clean:
	rm -f lloconv inject-meta *.o
