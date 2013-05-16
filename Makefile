
SUBDIRS= util fife_wda numsg nucondb ifbeam ifdh

all: 
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done

clean:
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done
	rm -rf build_art 

test:
	cd tests && python TestSuite.py

install: install-headers install-libs

install-libs: all
	rm -rf $(DESTDIR)lib 
	test -d $(DESTDIR)lib || mkdir -p  $(DESTDIR)lib && cp [inu]*/*.so [inu]*/*.a  $(DESTDIR)lib
	test -d $(DESTDIR)lib/python || mkdir -p  $(DESTDIR)lib/python && cp ifdh/python/*  $(DESTDIR)lib/python
	test -d $(DESTDIR)bin || mkdir -p $(DESTDIR)bin && cp ifdh/ifdh ifdh/ifdh_copyback.sh $(DESTDIR)bin

install-headers:
	rm -rf $(DESTDIR)inc
	test -d $(DESTDIR)inc || mkdir -p $(DESTDIR)inc && cp [inu]*/*.h $(DESTDIR)inc

32bit:
	ARCH="-m32 $(ARCH)" make all  install

withart:
	ARCH="-g -std=c++11 $(ARCH)" make all

distrib:
	tar czvf nucondb-client.tgz Makefile  [nu]*/*.[ch]* [nu]*/Makefile
	tar czvf ifdhc.tgz Makefile */*.[ch]* */Makefile ups
