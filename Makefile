LIB=libnucondb.a 
UTLOBJ=../util/*.o
UTLSRC=../util/*.cc
HDR=nucondb.h ../util/*.h
OBJ=nucondb.o $(UTL)
SRC=nucondb.cc
TST=nucondb-test
TESTDEFS=-DUNITTEST
CXXFLAGS=-fPIC -g $(DEFS) 

all: $(BIN) $(TST) $(LIB)

clean:
	rm -f *.o *.a 

$(LIB): $(OBJ) $(UTLOBJ)
	rm -f $(LIB)
	ar qv $(LIB) $(OBJ) $(UTLOBJ)

$(UTLOBJ):
	cd ../util; make

%-test: %.cc
	g++ -o $@ $(TESTDEFS) $(CXXFLAGS) $(UTLOBJ) $<

