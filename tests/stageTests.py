import unittest
import ifdh
import socket
import os
import time
import glob

base_uri_fmt = "http://samweb.fnal.gov:8480/sam/%s/api"

class Skipped(EnvironmentError):
    pass

class ifdh_stage_cases(unittest.TestCase):
    experiment = "nova"
    tc = 0
    buffer = True

    osg_site_write="srm://fndca1.fnal.gov:8443//pnfs/fnal.gov/usr/fermigrid/volatile"

    def list_stage_dir(self):
        f = os.popen('srmls -2 -recursion_depth=5 "%s" 2>&1' % ifdh_stage_cases.osg_site_write, "r")
        print f.read()
        f.close()

    def setUp(self):
        os.environ['EXPERIMENT'] =  ifdh_stage_cases.experiment
	self.data_dir="/grid/data/%s" % os.environ['USER']
        self.work = "/tmp/work%d" % os.getpid()
        os.mkdir(self.work)
        if not  os.environ.has_key('X509_USER_PROXY'):
            print "please run: /scratch/grid/kproxy %s" % ifdh_stage_cases.experiment
            print "and export X509_USER_PROXY=/scratch/%s/grid/%s.%s.proxy" % ( 
		os.environ['USER'], os.environ['USER'],ifdh_stage_cases.experiment)

    def tearDown(self):
        os.system("rm -rf %s" % self.work)

    def test_0_stage1(self):
        os.system("IFDH_STAGE_VIA=\"%s\" ifdh cp $IFDHC_DIR/ifdh/ifdh_cp.cc %s/test1.txt" % 
	 	   (ifdh_stage_cases.osg_site_write, self.data_dir))
        os.system("$IFDHC_DIR/bin/ifdh_copyback.sh")
        os.system("ifdh cp %s/test1.txt %s/test.txt" % (self.data_dir, self.work))
        os.system("diff %s/test.txt $IFDHC_DIR/ifdh/ifdh_cp.cc" % self.work)
        
def suite():
    suite =  unittest.TestLoader().loadTestsFromTestCase(ifdh_stage_cases)
    return suite

if __name__ == '__main__':
    unittest.main()

