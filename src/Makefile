UNAME:= $(shell uname -s)
ifeq ($(UNAME),Darwin)
SHLIB=dylib
else
SHLIB=so 
endif

##SRCDIR=../../nucondb/
DEFS=-I$(IFDHC_FQ_DIR)/inc -I$(LIBWDA_FQ_DIR)/include
##VPATH=$(SRCDIR)
LIB=libnucondb.a
NUCLIB=libnucondb.${SHLIB}
OTHER_LIB=-L$(IFDHC_DIR) -lifdh
HDR=nucondb.h 
OBJ=nucondb.o
SRC=nucondb.cc
TST=nucondb-test 
LDFLAGS=-L$(LIBWDA_LIB) -L$(IFDHC_LIB) -lifdh -lwda -lcurl
TESTDEFS=-DUNITTEST
CXXFLAGS=-pedantic-errors -Wall -Wextra -Werror -fPIC -g $(DEFS) $(ARCH) -I.

DESTDIR=..
VPATH=../../src

all: $(BIN) $(TST) $(LIB) $(NUCLIB)

install: install-lib install-headers

install-lib:
	test -d $(DESTDIR)/lib || mkdir -p $(DESTDIR)/lib
	cp $(LIB) $(NUCLIB) $(DESTDIR)/lib

install-headers:
	test -d $(DESTDIR)/include || mkdir -p $(DESTDIR)/include
	cp $(HDR) $(DESTDIR)/include

clean:
	rm -f *.o *.a  ${NUCLIB}

$(LIB): $(OBJ) 
	rm -f $(LIB)
	ar qv $(LIB) $(OBJ)

$(NUCLIB): $(OBJ) $(UTLOBJ)
	rm -f $(NUCLIB)
	g++ --shared -o $(NUCLIB) $(OBJ) $(OTHER_LIB) $(LDFLAGS)

%-test: %.cc
	g++ -o $@ $(TESTDEFS) $(CXXFLAGS) $(OTHER_LIB) $< $(LDFLAGS)

%.o: %.cc
	g++ -c -o $@  $(CXXFLAGS) $< 

nucondb.o: nucondb.cc 
