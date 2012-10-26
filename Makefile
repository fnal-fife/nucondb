
SUBDIRS= util numsg nucondb ifbeam ifdh

all: 
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done

clean:
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done
	rm -rf build_art

install: all
	rm -rf $(DESTDIR)lib $(DESTDIR)inc
	test -d $(DESTDIR)lib || mkdir -p  $(DESTDIR)lib && cp */*.so */*.a  $(DESTDIR)lib
	test -d $(DESTDIR)lib/python || mkdir -p  $(DESTDIR)lib/python && cp ifdh/python/*  $(DESTDIR)lib/python
	test -d $(DESTDIR)inc || mkdir -p $(DESTDIR)inc && cp */*.h $(DESTDIR)inc
	test -d $(DESTDIR)bin || mkdir -p $(DESTDIR)bin && cp ifdh/ifdh $(DESTDIR)bin

32bit:
	ARCH="-m32 $(ARCH)" make all  install

withart:
	test x$$ART_DIR != x
	ARCH="-std=c++11 -g -O0" make all install
	# later this will be ARCH="$(ART_CXXFLAGS)"

distrib:
	tar czvf nucondb-client.tgz Makefile  [nu]*/*.[ch]* [nu]*/Makefile
	tar czvf ifdhc.tgz Makefile */*.[ch]* */Makefile ups
