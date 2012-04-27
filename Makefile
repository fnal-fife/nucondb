
SUBDIRS= util numsg nucondb ifbeam ifdh

all clean install: 
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done

32bit:
	ARCH=-m32 make all 

64bit:
	ARCH=-m64 make all 

distrib:
	tar czvf nucondb-client.tgz Makefile  [nu]*/*.[ch]* [nu]*/Makefile 
	tar czvf ifdhc.tgz Makefile */*.[ch] */Makefile ups
