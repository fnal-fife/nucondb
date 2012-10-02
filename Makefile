LIB=libnucondb.a
SHLIB=libnucondb.so
UTLOBJ=../util/*.o
UTLSRC=../util/*.cc
HDR=nucondb.h ../util/*.h
OBJ=nucondb.o $(UTL)
SRC=nucondb.cc
TST=nucondb-test 
TESTDEFS=-DUNITTEST
CXXFLAGS=-fPIC -g $(DEFS) $(ARCH)

all: $(BIN) $(TST) $(LIB) $(SHLIB)

install:
	test -d ../lib || mkdir -p ../lib
	cp $(LIB) $(SHLIB) ../lib
	cp $(HDR) ../include
clean:
	rm -f *.o *.a 

$(LIB): $(OBJ) $(UTLOBJ)
	rm -f $(LIB)
	ar qv $(LIB) $(OBJ) $(UTLOBJ)

$(SHLIB): $(OBJ) $(UTLOBJ)
	rm -f $(SHLIB)
	g++ --shared -o $(SHLIB) $(OBJ) $(UTLOBJ)

$(UTLOBJ):
	cd ../util; make

%-test: %.cc
	g++ -o $@ $(TESTDEFS) $(CXXFLAGS) $(UTLOBJ) $<

