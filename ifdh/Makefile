
CXXFLAGS=-fPIC -I../util -I../numsg -g -I`echo /usr/include/python*`
OBJ=ifdh.o
ALL_OBJS=$(OBJ) ../util/WebAPI.o ../numsg/numsg.o ../util/SyslogAPI.o
PYMOD_OBJS=$(ALL_OBJS) ifdh_wrap.o

all: ifdh python/_ifdh.so ifdh.so

clean: 
	rm $(OBJ)

ifdh_main.cc: ifdh.h h_to_main.sh
	sh h_to_main.sh ifdh.h > $@

ifdh: ifdh.so ifdh_main.o
	g++ -o $@ ifdh_main.o $(ALL_OBJS)

python/_ifdh.so: $(PYMOD_OBJS)
	g++ --shared -o $@ $(PYMOD_OBJS) /usr/lib/libpython*.so

ifdh.so: $(ALL_OBJS)
	g++ --shared -o $@ $(ALL_OBJS)

ifdh_wrap.cxx: ifdh.h ifdh.i
	swig -python -c++ ifdh.i
	mv ifdh.py python/ifdh.py

ifdh_wrap.o: ifdh_wrap.cxx
	g++ $(CXXFLAGS) -c -o $@ ifdh_wrap.cxx
        

ifdh.o: ifdh.cc ifdh.h
	g++ $(CXXFLAGS) -c -o $@ ifdh.cc

ifdh_main.o: ifdh_main.cc
	g++ $(CXXFLAGS) -c -o $@ ifdh_main.cc
