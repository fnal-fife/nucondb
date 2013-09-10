#!/bin/sh

from ifdh import ifdh
import os
import re

eloc='srm://fndca1.fnal.gov:8443/pnfs/fnal.gov/usr/'

def ifdh_fetch( *flist, **kwargs ):
     ifdh_handle = ifdh()
     if kwargs.has_key("--dims"):
          flist.extend( ifdh_handle.translateConstraints(kwargs["dims"]))

     os.system("""
         voms-proxy-info -all | grep $EXPERIMENT > /dev/null || (
         kx509 && voms-proxy-init -noregen -voms fermilab:/fermilab/$EXPERIMENT/Role=Analysis)
     """)
 
     for f in flist:    
        
        flocs=ifdh_handle.locateFile(f)
        eflocs = []
        bflocs = []
        src = flocs[0]

        for l in flocs:
             if re.search('enstore:', l):
                 eflocs.append(l)
             if re.search('(_bluearc|data):', l):
                 bflocs.append(l)


        if len(eflocs) > 0:
           print "found file on enstore, using dcache srm"
           src = re.sub('enstore:/pnfs/',eloc,eflocs[0])
           src = re.sub('\(.*','',src)

        elif len(bflocs) > 0:
           print "found file on bluearc"
           src = re.sub('.*(_bluearc|data):','',bflocs[0])
           src = '%s/%s' % ( src, f )

        else:
           print "only location found was: ", src

        src = '%s/%s' % ( src, f )
        ifdh_handle.cp( [ src, "./%s" % f ] )

