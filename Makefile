
SUBDIRS= util numsg ifdh nucondb ifbeam 

all clean install: FORCE
	for d in $(SUBDIRS); do cd $d && make $@
