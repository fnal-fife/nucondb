SRCDIR=../../util/
VPATH=$(SRCDIR)
SRC=$(notdir $(wildcard $(SRCDIR)*.cc *.cc))
HDR=$(notdir $(wildcard $(SRCDIR)*.h *.h)
OBJ=$(addsuffix .o,$(basename $(SRC)))
TST=$(addsuffix -test,$(basename($SRC)))
DEFS=
TESTDEFS=-DUNITTEST
CXXFLAGS=-pedantic-errors -Wall -Wextra -Werror -fPIC -g $(DEFS) $(ARCH) -I../../util


all: $(OBJ) $(TST)

clean:
	rm -f $(OBJ)

%-test: %.cc
	g++ -o $@ $(TESTDEFS) $(CXXFLAGS) $< utils.o

%.o: %.cc
	g++ -c -o $@  $(CXXFLAGS) $< 
utils.o: utils.cc utils.h 
WebAPI.o: WebAPI.cc WebAPI.h 
