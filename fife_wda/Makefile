SRCDIR=../../fife_wda/
VPATH=$(SRCDIR)
LIB=libwda.a
HDR=ifbeam.h wda.h
OBJ=ifbeam.o wda.o
SRC= ifbeam.c wda.c
TST=test_ifbeam
CXXFLAGS=-fPIC -g $(DEFS) $(ARCH) -I$(SRCDIR)

VPATH=../../ifbeam

all: $(BIN) $(TST) $(LIB) $(SHLIB)

install:
	test -d ../lib || mkdir ../lib
	test -d ../include/wda || mkdir -p ../include/wda
	cp $(LIB) ../lib
	cp $(HDR) ../include/wda

clean:
	rm -f *.o *.a 

$(LIB): $(OBJ) $(UTLOBJ)
	rm -f $(LIB)
	ar qv $(LIB) $(OBJ) 

%.o: %.c
	gcc -c -o $@  $(CXXFLAGS) $< 

test_ifbeam: test_ifbeam.o $(OBJ)
	gcc -o ifbeam_test test_ifbeam.o $(OBJ) `test -r /usr/lib/libcurl.a && echo --static` -lcurl --dynamic 
