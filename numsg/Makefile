BIN=numsg-test nulogprintf
LIB=LibNuMsg.a
UTLOBJ=../util/SyslogAPI.o ../util/utils.o
OBJ=numsg.o
HDR=numsg.h
LOG_OBJ= nulogprintf.o $(OBJ)
CXXFLAGS=-fpic -g $(ARCH) -I../util

all: $(OBJ) $(BIN) $(LIB)

install:
	test -d ../lib || mkdir -p ../lib
	test -d ../include || mkdir -p ../include
	cp $(LIB) ../lib
	cp $(HDR) ../include

clean:
	rm *.o $(BIN) $(LIB)

nulogprintf: $(LOG_OBJ)
	$(CXX) $(CXXFLAGS) -o nulogprintf $(LOG_OBJ) $(UTLOBJ)

LibNuMsg.a: $(OBJ) $(UTLOBJ)
	ar qv $(LIB) $(OBJ) $(UTLOBJ)
        
numsg-test: $(UTLOBJ) numsg.cc
	$(CXX) $(CXXFLAGS) -DUNITTEST numsg.cc -o numsg-test $(UTLOBJ)

%.o: %.cc
	g++ -c -o $@  $(CXXFLAGS) $< 
