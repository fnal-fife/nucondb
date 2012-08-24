
SUBDIRS= util numsg nucondb ifbeam ifdh

all install: 
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done
	test x$$ART_DIR != x && test -d build_art || mkdir build_art && cd build_art && cmake ../ifdh_art && make VERBOSE=1

clean:
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done
	rm -rf build_art

install: all
	rm -rf lib inc
	test -d lib || mkdir lib && mv */*.so */*.a */lib/*.so lib
	test -d inc || mkdir inc && cp */*.h */*/*.h inc

32bit:
	ARCH=-m32 make all 

distrib:
	tar czvf nucondb-client.tgz Makefile  [nu]*/*.[ch]* [nu]*/Makefile 
	tar czvf ifdhc.tgz Makefile */*.[ch] */Makefile ups
