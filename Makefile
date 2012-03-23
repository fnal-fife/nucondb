
SUBDIRS= util numsg nucondb ifbeam ifdh

all clean install: 
	for d in $(SUBDIRS); do ([ -d $$d ] && cd $$d && make $@); done

distrib:
	tar czvf nucondb-client.tgz Makefile  [nu]/*.[ch]* [nu]*/Makefile 
	tar czvf ifdhc.tgz Makefile */*.[ch] */Makefile
