LIB=libnucondb.a 
OBJ=nucondb.o WebAPI.o
SRC=nucondb.cc WebAPI.cc demo.cc
HDR=nucondb.h WebAPI.h
DEFS=
TESTDEFS=-DUNITTEST
CFLAGS=-g $(DEFS) 
CXXFLAGS=-g $(DEFS) 
BIN= demo nucondb-test WebAPI-test

all: $(BIN) $(LIB)

clean:
	rm -f *.o *.a $(BIN) nucondb-client.tgz 

distrib: $(SRC) $(HDR)
	tar czvf nucondb-client.tgz Makefile $(SRC) $(HDR)

$(LIB): $(OBJ)
	rm -f $(LIB)
	ar qv $(LIB) $(OBJ)

nucondb-test: nucondb.cc $(LIB)
	g++ -o $@ $(TESTDEFS) $(CFLAGS) nucondb.cc $(LIB)

WebAPI-test: WebAPI.cc WebAPI.h
	g++ -o $@ $(TESTDEFS) $(CFLAGS) WebAPI.cc 

demo: demo.o $(LIB)
	g++ -o $@ $(CFLAGS) demo.o $(LIB)
  
WebAPI.o: WebAPI.h

nucondb.o: nucondb.h
