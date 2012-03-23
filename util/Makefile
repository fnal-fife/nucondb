SRC=$(wildcard *.cc)
HDR=$(wildcard *.h)
OBJ=$(addsuffix .o,$(basename $(SRC)))
TST=$(addsuffix -test,$(basename($SRC)))
DEFS=
TESTDEFS=-DUNITTEST
CXXFLAGS=-fPIC -g $(DEFS) 


all: $(OBJ) $(TST)

clean:
	rm -f $(OBJ)

%-test: %.cc
	g++ -o $@ $(TESTDEFS) $(CXXFLAGS) $<

