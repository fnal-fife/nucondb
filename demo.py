#!/usr/bin/env python

import os
import time

if not os.environ.has_key("IFDH_BASE_URI"):
    os.environ["IFDH_BASE_URI"] = "http://samweb-minerva.fnal.gov:20004/sam/minerva/api"

import ifdh

i = ifdh.ifdh(os.environ["IFDH_BASE_URI"])

# test locate/describe
print i.locateFile("MV_00003142_0014_numil_v09_1105080215_RawDigits_v1_linjc.root")
print i.describeDefinition("mwm_test_2")

# now start a project, and consume its files, alternately skipping or consuming
# them...
projname=time.strftime("mwm_%Y%m%d%H_%%d")%os.getpid()
cpurl=i.startProject(projname,"minerva", "mwm_test_2", "mengel", "minerva")
time.sleep(2)
cpurl=i.findProject(projname,"minerva")
consumer_id=i.establishProcess( cpurl, "demo","1", "bel-kwinith.fnal.gov","mengel" )
flag=True
furi=i.getNextFile(cpurl,consumer_id)
while furi:
	fname=i.fetchInput(furi)
        if flag:
		i.updateFileStatus(cpurl, consumer_id, fname, 'transferred')
		time.sleep(1)
		i.updateFileStatus(cpurl, consumer_id, fname, 'consumed')
                flag=False
        else:
		i.updateFileStatus(cpurl, consumer_id, fname, 'skipped')
                flag=True
        furi=i.getNextFile(cpurl,consumer_id)
i.setStatus( cpurl, consumer_id, "bad")
i.endProject( cpurl )
