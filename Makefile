CC=gcc
CXX=g++

CSOURCES=clib.c
CPPSOURCES=Db.cpp Main.cpp ExpListView.cpp
COBJECTS=$(patsubst %.c, %.o, $(CSOURCES))
CPPOBJECTS=$(patsubst %.cpp, %.o, $(CPPSOURCES))
OBJECTS=$(COBJECTS) $(CPPOBJECTS) sqlite3.o

# wx-config --cxxflags --libs std,propgrid
WX_CXXFLAGS=`wx-config --cxxflags`

# wx-config --libs all  (all wx libs)
# wx-config --libs std  (default wx libs - core,base)
# wx-config --libs std,propgrid
WX_LIBS=`wx-config --libs std`

CFLAGS= -std=gnu99 -Wall -Werror -Wno-unused -Wno-deprecated-declarations
CPPFLAGS=-g -Wall -Werror -Wno-deprecated-declarations
CPPFLAGS+= $(WX_CXXFLAGS)
LDFLAGS=$(WX_LIBS)

#.SILENT:
all: t

dep:
	echo No dependencies

sqlite3.o: sqlite3/sqlite3.c
	$(CC) -c -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $<

t: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

expconv: expconv.c clib.o db.o sqlite3.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf t $(OBJECTS)

