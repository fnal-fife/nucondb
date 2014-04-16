SRCDIR=../../nucondb/
DEFS=-I$(IFDHC_DIR)/inc -I$(LIBWDA_DIR)/include
VPATH=$(SRCDIR)
LIB=libnucondb.a
SHLIB=libnucondb.so
OTHER_LIB=-L$(IFDHC_DIR) -lifdh
HDR=nucondb.h 
OBJ=nucondb.o
SRC=nucondb.cc
TST=nucondb-test 
LDFLAGS=-L$(LIBWDA_DIR)/lib -L$(IFDHC_LIB) -lifdh -lwda -lcurl
TESTDEFS=-DUNITTEST
CXXFLAGS=-pedantic-errors -Wall -Wextra -Werror -fPIC -g $(DEFS) $(ARCH) -I$(SRCDIR)

DESTDIR=..
VPATH=../../src

all: $(BIN) $(TST) $(LIB) $(SHLIB)

install: install-lib install-headers

install-lib:
	test -d $(DESTDIR)/lib || mkdir -p $(DESTDIR)/lib
	cp $(LIB) $(SHLIB) $(DESTDIR)/lib

install-headers:
	test -d $(DESTDIR)/include || mkdir -p $(DESTDIR)/include
	cp $(HDR) $(DESTDIR)/include

clean:
	rm -f *.o *.a  *.so

$(LIB): $(OBJ) 
	rm -f $(LIB)
	ar qv $(LIB) $(OBJ)

$(SHLIB): $(OBJ) $(UTLOBJ)
	rm -f $(SHLIB)
	g++ --shared -o $(SHLIB) $(OBJ) $(OTHER_LIB) $(LDFLAGS)

%-test: %.cc
	g++ -o $@ $(TESTDEFS) $(CXXFLAGS) $(OTHER_LIB) $< $(LDFLAGS)

%.o: %.cc
	g++ -c -o $@  $(CXXFLAGS) $< 

nucondb.o: nucondb.cc 
