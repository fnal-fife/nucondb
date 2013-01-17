SRCDIR=../../nucondb/
VPATH=$(SRCDIR)
LIB=libnucondb.a
SHLIB=libnucondb.so
UTLOBJ=../util/*.o
UTLSRC=../util/*.cc
HDR=nucondb.h ../util/*.h
OBJ=nucondb.o $(UTL)
SRC=nucondb.cc
TST=nucondb-test 
TESTDEFS=-DUNITTEST
CXXFLAGS=-pedantic-errors -Wall -Wextra -Werror -fPIC -g $(DEFS) $(ARCH) -I$(SRCDIR)

VPATH=../../nucondb

all: $(BIN) $(TST) $(LIB) $(SHLIB)

install:
	test -d ../lib || mkdir -p ../lib
	cp $(LIB) $(SHLIB) ../lib
	cp $(HDR) ../include
clean:
	rm -f *.o *.a  *.so

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

%.o: %.cc
	g++ -c -o $@  $(CXXFLAGS) $< 

nucondb.o: nucondb.cc ../util/WebAPI.h ../util/utils.h
