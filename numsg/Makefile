BIN= SyslogAPI-test numsg-test nulogprintf
LIB=LibNuMsg.a
OBJ=SyslogAPI.o numsg.o
LOG_OBJ= nulogprintf.o $(OBJ)
CXXFLAGS=-fpic -g

all: $(BIN) $(LIB)

clean:
	rm *.o $(BIN) $(LIB)

nulogprintf: $(LOG_OBJ)
	$(CXX) $(CXXFLAGS) -o nulogprintf $(LOG_OBJ)

LibNuMsg.a: $(OBJ)
	ar qv $(LIB) $(OBJ)
        
SyslogAPI-test:
	$(CXX) $(CXXFLAGS) -o SyslogAPI-test SyslogAPI.cc -DUNITTEST

numsg-test: SyslogAPI.o numsg.cc
	$(CXX) $(CXXFLAGS) -DUNITTEST numsg.cc -o numsg-test SyslogAPI.o
