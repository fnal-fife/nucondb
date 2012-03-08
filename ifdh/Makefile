
CFLAGS=-I../client -I../../numsg -g
SHLIB_OBJS=ifdh.o ../client/WebAPI.o ../../numsg/numsg.o ../../numsg/SyslogAPI.o

ifdh: ifdh.so ifdh_main.o
	g++ -o $@ ifdh_main.o ifdh.so

ifdh.so: $(SHLIB_OBJS)
	g++ --shared -o $@ $(SHLIB_OBJS)

ifdh.o: ifdh.cc ifdh.h
	g++ $(CFLAGS) -c -o $@ ifdh.cc

ifdh_main.o: ifdh_main.cc
	g++ $(CFLAGS) -c -o $@ ifdh_main.cc
