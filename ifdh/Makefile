
CXXFLAGS=-fPIC -I../client -I../../numsg -g -I`echo /usr/include/python*`
PLAIN_OBJS=ifdh.o ../client/WebAPI.o ../../numsg/numsg.o ../../numsg/SyslogAPI.o
PYMOD_OBJS=$(PLAIN_OBJS) ifdh_wrap.o

all: ifdh python/_ifdh.so ifdh.so

ifdh_main.cc: ifdh.h h_to_main.sh
	sh h_to_main.sh ifdh.h > $@

ifdh: ifdh.so ifdh_main.o
	g++ -o $@ ifdh_main.o $(PLAIN_OBJS)

python/_ifdh.so: $(PYMOD_OBJS)
	g++ --shared -o $@ $(PYMOD_OBJS) /usr/lib/libpython*.so

ifdh.so: $(PLAIN_OBJS)
	g++ --shared -o $@ $(PLAIN_OBJS)

ifdh_wrap.cxx: ifdh.h ifdh.i
	swig -python -c++ ifdh.i
	mv ifdh.py python/ifdh.py

ifdh_wrap.o: ifdh_wrap.cxx
	g++ $(CXXFLAGS) -c -o $@ ifdh_wrap.cxx
        

ifdh.o: ifdh.cc ifdh.h
	g++ $(CXXFLAGS) -c -o $@ ifdh.cc

ifdh_main.o: ifdh_main.cc
	g++ $(CXXFLAGS) -c -o $@ ifdh_main.cc
