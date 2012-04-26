
LIB=ifdh.so
HDR=ifdh.h
CXXFLAGS=-fPIC -I../util -I../numsg -g -I`echo /usr/include/python*` $(ARCH)
OBJ=ifdh.o
ALL_OBJS=$(OBJ) ../util/WebAPI.o ../numsg/numsg.o ../util/SyslogAPI.o
PYMOD_OBJS=$(ALL_OBJS) ifdh_wrap.o

all: ifdh python/_ifdh.so ifdh.so ifdh_art_test

install:
	test -d ../lib/python || mkdir -p ../lib/python
	test -d ../bin || mkdir ../bin
	cp ifdh   ../bin
	cp $(LIB) ../lib
	cp $(HDR) ../include
	cp python/* ../lib/python

clean: 
	rm $(OBJ)

ifdh_main.cc: ifdh.h h_to_main.sh
	sh h_to_main.sh ifdh.h > $@

ifdh: ifdh.so ifdh_main.o
	g++ $(ARCH) -o $@ ifdh_main.o $(ALL_OBJS)
	./ifdh --help | perl -pe 's{\tifdh ([^ ]*) *(.*?) *$$}{* *ifdh* *$$1* _$$2_}; s{\t--}{** }' > ifdh.textile

python/_ifdh.so: $(PYMOD_OBJS)
	g++ --shared $(ARCH) -o  $@ $(PYMOD_OBJS) /usr/lib*/python*/config/libpython*.a

ifdh.so: $(ALL_OBJS)
	g++ --shared $(ARCH)  -o $@ $(ALL_OBJS)

ifdh_wrap.cxx: ifdh.h ifdh.i
	swig -python -c++ ifdh.i
	mv ifdh.py python/ifdh.py

ifdh_wrap.o: ifdh_wrap.cxx
	g++ $(CXXFLAGS) -c -o $@ ifdh_wrap.cxx
        

ifdh.o: ifdh.cc ifdh.h
	g++ $(CXXFLAGS) -c -o $@ ifdh.cc

ifdh_main.o: ifdh_main.cc
	g++ $(CXXFLAGS) -c -o $@ ifdh_main.cc

ifdh_art_test:  ifdh_art_test.cc ifdh.cc ../util/WebAPI.o ../util/utils.o ../numsg/numsg.o
	g++ $(CXXFLAGS) $(ARCH) -o ifdh_art_test ifdh_art_test.cc ifdh_art.cc ifdh.o ../util/WebAPI.o ../util/utils.o ../numsg/numsg.o ../util/SyslogAPI.o
