BIN=numsg-test nulogprintf
LIB=LibNuMsg.a
UTLOBJ=../util/SyslogAPI.o
OBJ=numsg.o
LOG_OBJ= nulogprintf.o $(OBJ)
CXXFLAGS=-fpic -g

all: $(OBJ) $(BIN) $(LIB)

clean:
	rm *.o $(BIN) $(LIB)

nulogprintf: $(LOG_OBJ)
	$(CXX) $(CXXFLAGS) -o nulogprintf $(LOG_OBJ) $(UTLOBJ)

LibNuMsg.a: $(OBJ) $(UTLOBJ)
	ar qv $(LIB) $(OBJ) $(UTLOBJ)
        
numsg-test: $(UTLOBJ) numsg.cc
	$(CXX) $(CXXFLAGS) -DUNITTEST numsg.cc -o numsg-test $(UTLOBJ)
