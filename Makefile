
SUBDIRS= util numsg nucondb ifbeam ifdh

all: 
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done
	test x$$ART_DIR != x && test -d build_art || mkdir build_art && cd build_art && cmake ../ifdh_art && make VERBOSE=1

clean:
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done
	rm -rf build_art

install: all
	rm -rf $(DESTDIR)lib $(DESTDIR)inc
	test -d $(DESTDIR)lib || mkdir -p  $(DESTDIR)lib && mv */*.so */*.a */lib/*.so $(DESTDIR)lib
	test -d $(DESTDIR)inc || mkdir -p $(DESTDIR)inc && cp */*.h */*/*.h $(DESTDIR)inc

32bit:
	ARCH="-m32 $(ARCH)" make all 

distrib:
	tar czvf nucondb-client.tgz Makefile  [nu]*/*.[ch]* [nu]*/Makefile 
	tar czvf ifdhc.tgz Makefile */*.[ch] */Makefile ups
