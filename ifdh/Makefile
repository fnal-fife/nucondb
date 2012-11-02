
LIB=libifdh.so
HDR=ifdh.h
CXXFLAGS=-fPIC -I../util -I../numsg -g -I`echo /usr/include/python*` $(ARCH)
OBJ=ifdh.o
ALL_OBJS=$(OBJ) ../util/WebAPI.o ../numsg/numsg.o ../util/SyslogAPI.o ../util/utils.o
PYMOD_OBJS=$(ALL_OBJS) ifdh_wrap.o

all: ifdh python python/_ifdh.so libifdh.so libifdh.a

install:
	test -d ../lib/python || mkdir -p ../lib/python
	test -d ../bin || mkdir ../bin
	cp ifdh   ../bin
	cp $(LIB) ../lib
	cp $(HDR) ../include
	cp python/* ../lib/python

clean: 
	rm $(OBJ)

python:
	mkdir python

ifdh_main.cc: ifdh.h h_to_main.sh
	sh h_to_main.sh ifdh.h > $@

ifdh: libifdh.so ifdh_main.o
	g++ $(ARCH) -o $@ ifdh_main.o $(ALL_OBJS)
	./ifdh --help | perl -pe 's{\tifdh ([^ ]*) *(.*?) *$$}{* *ifdh* *$$1* _$$2_}; s{\t--}{** }' > ifdh.textile

python/_ifdh.so: $(PYMOD_OBJS)
	g++ --shared $(ARCH) -o  $@ $(PYMOD_OBJS) /usr/lib*/python*/config/libpython*.[as]* || g++ --shared $(ARCH) -o  $@ $(PYMOD_OBJS) /usr/lib/python*/config/libpython*.[as]*

libifdh.so: $(ALL_OBJS)
	g++ --shared $(ARCH) $(CPPFLAGS)  -o $@ $(ALL_OBJS)

libifdh.a: $(ALL_OBJS)
	rm -f $@
	ar qv $@ $(ALL_OBJS)
	ranlib $@ 

ifdh_wrap.cxx: ifdh.h ifdh.i
	swig -python -c++ ifdh.i
	mv ifdh.py python/ifdh.py

ifdh_wrap.o: ifdh_wrap.cxx
	g++ $(CXXFLAGS) $(ARCH) -c -o $@ ifdh_wrap.cxx
        

ifdh.o: ifdh.cc ifdh.h
	g++ $(CXXFLAGS) -c -o $@ ifdh.cc

ifdh_main.o: ifdh_main.cc
	g++ $(CXXFLAGS) -c -o $@ ifdh_main.cc

%.o: %.cc
	g++ -c -o $@  $(CXXFLAGS) $< 
