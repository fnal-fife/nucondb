#!/usr/bin/env python

import unittest
import ifdh
import socket
import os
import time
import glob
from subprocess import Popen, PIPE
import stat

base_uri_fmt = "http://samweb.fnal.gov:8480/sam/%s/api"


class ifdh_lock_cases(unittest.TestCase):

    def setUp(self):
        """
           private lock stuff, based on:
           https://cdcvs.fnal.gov/redmine/projects/ifdhcpn/wiki/DEVELOPMENT#Move-to-private-test-LOCKs
        """
        self.lockbase = '/nusoft/app/home'
        self.lockgroup = 'ifmon'
        self.lock = '%s/%s/LOCK' % (self.lockbase, self.lockgroup)
        os.environ['CPN_LOCK_BASE'] = self.lockbase
        os.environ['CPN_LOCK_GROUP'] = self.lockgroup
        self.limit = '%s/limit' % self.lock
        self.wait = '%s/wait' % self.lock
        self.ifdh = ifdh.ifdh()
        # always put stats back...
        f = open(self.limit,"w")
        f.write("5\n")
        f.close()
        f = open(self.wait,"w")
        f.write("5\n")
        f.close()
        
    def tearDown(self):
        # always put stats back...
        f = open(self.limit,"w")
        f.write("5\n")
        f.close()
        f = open(self.wait,"w")
        f.write("5\n")
        f.close()
 
    def test_01_simple_lock(self):
        """
           https://cdcvs.fnal.gov/redmine/projects/ifdhcpn/wiki/DEVELOPMENT#Simple-lock-and-release
           but we do an empty copy to do the lock/unlock
        """
        self.ifdh.cp( ['--force=cpn', '/dev/null', '/dev/null'])
        f = Popen("ls -l %s/LOG" % self.lock, shell=True, bufsize=1024, stdout=PIPE).stdout
        for line in f.readlines():
            last = line
        f.close()
        self.assertTrue('ifmon' in last)

    def test_02_single_queued(self):
        # set param files to force waiting...
        f = open(self.limit,"w")
        f.write("0\n")
        f.close()
        f = open(self.wait,"w")
        f.write("200\n")
        f.close()

        # start a copy which will sleep
        cp_s = Popen("ifdh cp --force=cpn /dev/null /dev/null 2>&1",shell=True, bufsize=1024, stdout=PIPE)
        cp_p = cp_s.stdout
        s = cp_p.readline()
        print "read line: ", s
        while "already in progress" in s:
            s = cp_p.readline()
            print "read line: ", s
        self.assertTrue("sleeping" in s)
        s = cp_p.readline()
        print "read line: ", s
        self.assertTrue("queue" in s)
        p = s.find("queue")

        print "checking queue..."
        # make sure queue file exists, and has time updated
        qf = "%s/QUEUE/%s" % ( self.lock , s[p+6:-1])
        sb = os.stat(qf)
        t1 = sb[stat.ST_MTIME]
        print "time before: " , t1
        time.sleep(90)
        sb = os.stat(qf)
        t2 = sb[stat.ST_MTIME]
        print "time after: " , t2
        self.assertTrue(t1 != t2)

        # set parameters to let it continue
        f = open(self.limit,"w")
        f.write("5\n")
        f.close()
        f = open(self.wait,"w")
        f.write("5\n")
        f.close()

        portnum = cp_s.pid + 1800
        os.system('echo "wakeup" > /dev/udp/127.0.0.1/%d' % portnum)

        s = cp_p.readline()
        print "read line: ", s
        self.assertTrue(" lock " in s)
 
        cp_p.readlines()

def suite():
    suite =  unittest.TestLoader().loadTestsFromTestCase(ifdh_lock_cases)
    return suite

if __name__ == '__main__':
    unittest.main()
