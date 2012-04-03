LIB=libnucondb.a 
UTLOBJ=../util/*.o
UTLSRC=../util/*.cc
HDR=nucondb.h ../util/*.h
OBJ=nucondb.o $(UTL)
SRC=nucondb.cc
TST=nucondb-test nucondb_art_test
TESTDEFS=-DUNITTEST
CXXFLAGS=-fPIC -g $(DEFS) 

all: $(BIN) $(TST) $(LIB)

install:
	test -d ../lib || mkdir -p ../lib
	cp $(LIB) ../lib
	cp $(HDR) ../include
clean:
	rm -f *.o *.a 

$(LIB): $(OBJ) $(UTLOBJ)
	rm -f $(LIB)
	ar qv $(LIB) $(OBJ) $(UTLOBJ)

$(UTLOBJ):
	cd ../util; make

%-test: %.cc
	g++ -o $@ $(TESTDEFS) $(CXXFLAGS) $(UTLOBJ) $<


nucondb_art_test: nucondb_art_test.cc nucondb_art.cc nucondb.o ../util/WebAPI.o ../util/utils.o
	g++ -o nucondb_art_test nucondb_art_test.cc nucondb_art.cc nucondb.o ../util/WebAPI.o ../util/utils.o
