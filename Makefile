
SUBDIRS= util numsg nucondb ifbeam ifdh

all: 
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done

clean:
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done
	rm -rf build_art

install: all
	rm -rf $(DESTDIR)lib $(DESTDIR)inc
	test -d $(DESTDIR)lib || mkdir -p  $(DESTDIR)lib && cp [inu]*/*.so [inu]*/*.a  $(DESTDIR)lib
	test -d $(DESTDIR)lib/python || mkdir -p  $(DESTDIR)lib/python && cp ifdh/python/*  $(DESTDIR)lib/python
	test -d $(DESTDIR)inc || mkdir -p $(DESTDIR)inc && cp [inu]*/*.h $(DESTDIR)inc
	test -d $(DESTDIR)bin || mkdir -p $(DESTDIR)bin && cp ifdh/ifdh $(DESTDIR)bin

32bit:
	ARCH="-m32 $(ARCH)" make all  install

withart:
	test x$$ART_DIR != x
	ARCH="$(ART_CXXFLAGS)" make all

distrib:
	tar czvf nucondb-client.tgz Makefile  [nu]*/*.[ch]* [nu]*/Makefile
	tar czvf ifdhc.tgz Makefile */*.[ch]* */Makefile ups
