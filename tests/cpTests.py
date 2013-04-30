import unittest
import ifdh
import socket
import os
import time
import glob

base_uri_fmt = "http://samweb.fnal.gov:8480/sam/%s/api"


class ifdh_cp_cases(unittest.TestCase):
    experiment = "nova"
    tc = 0
    buffer = True


    def list_remote_dir(self):
        f = os.popen('srmls "srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=%s" 2>&1' % self.data_dir, "r")
        # print f.read()
        f.close()

    def make_test_txt(self):
        # clean out copy on data dir...
        os.system('srmrm -2 "srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=%s/test.txt" 2>&1' % self.data_dir)
        ifdh_cp_cases.tc = ifdh_cp_cases.tc + 1
        out = open("%s/test.txt" % self.work, "w")
        out.write("testing testing %d \n" % ifdh_cp_cases.tc)
        out.close()

    def check_test_txt(self):
        fin = open("%s/test.txt" % self.work, "r")
        l = fin.read()
        fin.close
        return ifdh_cp_cases.tc == int(l[16:])
       
    def setUp(self):
        os.environ['EXPERIMENT'] =  ifdh_cp_cases.experiment
        self.ifdh_handle = ifdh.ifdh(base_uri_fmt % ifdh_cp_cases.experiment)
        self.hostname = socket.gethostname()
        self.work="/tmp/work%d" % os.getpid()
	self.data_dir="/grid/data/%s" % os.environ['USER']
        if not  os.environ.has_key('X509_USER_PROXY'):
            print "please run: /scratch/grid/kproxy %s" % ifdh_cp_cases.experiment
            print "and export X509_USER_PROXY=/scratch/%s/grid/%s.%s.proxy" % ( 
		os.environ['USER'], os.environ['USER'],ifdh_cp_cases.experiment)

        # setup test directory tree..
        count = 0
        os.mkdir("%s" % (self.work))
        for sd in [ 'a', 'a/b', 'a/c' ]:
            os.mkdir("%s/%s" % (self.work,sd))
            for i in [1 , 2]:
                count = count + 1
                f = open("%s/%s/f%d" % (self.work,sd,count),"w")
                f.write("foo\n")
                f.close()
        # os.system("ls -R %s" % self.work)

    def tearDown(self):
        os.system("rm -rf %s" % self.work)

    def test_0_OuputFiles(self):
        self.ifdh_handle.cleanup()
        self.ifdh_handle.addOutputFile('%s/a/f1' % self.work)
        self.ifdh_handle.addOutputFile('%s/a/f2' % self.work)
        self.ifdh_handle.copyBackOutput(self.data_dir)
        self.ifdh_handle.cleanup()

    def test_0_OuputRenameFiles(self):
        self.ifdh_handle.cleanup()
        self.ifdh_handle.addOutputFile('%s/a/f1' % self.work)
        self.ifdh_handle.addOutputFile('%s/a/f2' % self.work)
        self.ifdh_handle.renameOutput('unique')
        self.ifdh_handle.copyBackOutput(self.data_dir)
        self.ifdh_handle.cleanup()

    def test_01_OuputRenameFiles(self):
        self.ifdh_handle.cleanup()
        self.ifdh_handle.addOutputFile('%s/a/f1' % self.work)
        self.ifdh_handle.addOutputFile('%s/a/f2' % self.work)
        self.ifdh_handle.renameOutput('s/f/g/')
        self.ifdh_handle.copyBackOutput(self.data_dir)
        self.ifdh_handle.cleanup()

    def test_1_OuputFiles(self):
        self.ifdh_handle.cleanup()
        self.ifdh_handle.addOutputFile('%s/a/f1' % self.work)
        self.ifdh_handle.addOutputFile('%s/a/f2' % self.work)
        self.ifdh_handle.copyBackOutput(self.work)
        self.ifdh_handle.cleanup()

    def test_1_OuputRenameFiles(self):
        self.ifdh_handle.cleanup()
        self.ifdh_handle.addOutputFile('%s/a/f1' % self.work)
        self.ifdh_handle.addOutputFile('%s/a/f2' % self.work)
        self.ifdh_handle.renameOutput('unique')
        self.ifdh_handle.copyBackOutput(self.work)
        self.ifdh_handle.cleanup()

    def test_11_OuputRenameFiles(self):
        self.ifdh_handle.cleanup()
        self.ifdh_handle.addOutputFile('%s/a/f1' % self.work)
        self.ifdh_handle.addOutputFile('%s/a/f2' % self.work)
        self.ifdh_handle.renameOutput('s/f/g/')
        self.ifdh_handle.copyBackOutput(self.work)
        self.ifdh_handle.cleanup()

    def test_1_list_copy(self):
        self.make_test_txt()
        f = open("%s/list" % self.work,"w")
        f.write("%s/test.txt %s/test2.txt\n%s/test2.txt %s/test3.txt\n" % (self.work, self.work, self.work, self.work))
        f.close()
        self.ifdh_handle.cp([ "-f", "%s/list" % self.work])
        os.system("mv %s/test3.txt %s/test.txt" % (self.work, self.work))
        self.assertEqual(self.check_test_txt(), True)
         

    def test_gsiftp__out(self):
        self.make_test_txt()
        self.ifdh_handle.cp([ "--force=gridftp", "%s/test.txt"%self.work, "%s/test.txt" % self.data_dir])
        self.assertEqual(0,0)  # not sure how to verify if it is remote..

    def test_gsiftp_in(self):
        self.list_remote_dir()
        self.ifdh_handle.cp([ "--force=gridftp" , "%s/test.txt" % self.data_dir, "%s/test.txt"%self.work])
        self.assertEqual(self.check_test_txt(), True)

    def test_explicit_gsiftp__out(self):
        self.make_test_txt()
        self.ifdh_handle.cp([ "%s/test.txt"%self.work, "gsiftp://if-gridftp-nova.fnal.gov%s/test.txt" % self.data_dir])
        self.assertEqual(0,0)  # not sure how to verify if it is remote..

    def test_explicit_gsiftp_in(self):
        self.list_remote_dir()
        self.ifdh_handle.cp([ "gsiftp://if-gridftp-nova.fnal.gov%s/test.txt" % self.data_dir, "%s/test.txt"%self.work])
        self.assertEqual(self.check_test_txt(), True)

    def test_expftp__out(self):
        self.make_test_txt()
        self.ifdh_handle.cp([ "--force=expftp", "%s/test.txt"%self.work, "%s/test.txt" % self.data_dir])
        self.assertEqual(0,0)  # not sure how to verify if it is remote..

    def test_expftp_in(self):
        self.list_remote_dir()
        self.ifdh_handle.cp([ "--force=expftp" , "%s/test.txt"%self.data_dir, "%s/test.txt"%self.work])
        self.assertEqual(self.check_test_txt(), True)

    def test_srm__out(self):
        self.make_test_txt()
        self.ifdh_handle.cp([ "--force=srm", "%s/test.txt"%self.work, "%s/test.txt" % self.data_dir])
        self.assertEqual(0,0)  # not sure how to verify if it is remote..

    def test_srm_in(self):
        self.list_remote_dir()
        self.ifdh_handle.cp([ "--force=srm" , "%s/test.txt"%self.data_dir, "%s/test.txt"%self.work])
        self.assertEqual(self.check_test_txt(), True)

    def test_explicit_srm__out(self):
        self.make_test_txt()
        self.ifdh_handle.cp([ "%s/test.txt"%self.work, "srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=%s/test.txt" % self.data_dir])
        self.assertEqual(0,0)  # not sure how to verify if it is remote..

    def test_explicit_srm_in(self):
        self.list_remote_dir()
        self.ifdh_handle.cp([ "srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=%s/test.txt"%self.data_dir, "%s/test.txt"%self.work])
        self.assertEqual(self.check_test_txt(), True)

    def test_default__out(self):
        self.make_test_txt()
        self.ifdh_handle.cp([ "%s/test.txt"%self.work, "%s/test.txt" % self.data_dir])
        self.assertEqual(0,0)  # not sure how to verify if it is remote..

    def test_default_in(self):
        self.list_remote_dir()
        self.ifdh_handle.cp([ "%s/test.txt"%self.data_dir, "%s/test.txt"%self.work])
        res = os.stat("%s/test.txt" % self.work)
        self.assertEqual(self.check_test_txt(), True)

    def test_dd(self):
        os.mkdir("%s/d" % self.work)

        self.ifdh_handle.cp(["--force=dd", "-D", "%s/a/b/f3"%self.work, "%s/a/b/f4"%self.work, "%s/d"%self.work])

        #afterwards, should have 2 files in work/d
        l4 = glob.glob("%s/d/f*" % self.work)
        self.assertEqual(len(l4), 2)

    def test_recursive(self):

        self.ifdh_handle.cp(["-r", "%s/a"%self.work, "%s/d"%self.work])
        
        #afterwards, should have 6 files in work/d
        l4 = glob.glob("%s/d/f*" % self.work)
        l5 = glob.glob("%s/d/b/f*" % self.work)
        l6 = glob.glob("%s/d/c/f*" % self.work)
        self.assertEqual(len(l4)+len(l5)+len(l6), 6)

    def test_recursive_globus(self):

        # local to local hoses up, try copying out and back in
        # force copy outbound by experiment ftp server, so we
        # own the files and can clean them out when done.
        self.ifdh_handle.cp([ "--force=expftp", "-r", "%s/a"%self.work, "%s/a"%self.data_dir])
        
        self.ifdh_handle.cp([ "--force=gridftp", "-r", "%s/a"%self.data_dir, "%s/d"%self.work])

 
        #afterwards, should have 6 files in work/d
        l4 = glob.glob("%s/d/f*" % self.work)
        l5 = glob.glob("%s/d/b/f*" % self.work)
        l6 = glob.glob("%s/d/c/f*" % self.work)
        self.assertEqual(len(l4)+len(l5)+len(l6), 6)

    def test_recursive_globus_w_args(self):

        # local to local hoses up, try copying out and back in
        # force copy outbound by experiment ftp server, so we
        # own the files and can clean them out when done.
        self.ifdh_handle.cp([ "-r", "--force=expftp", "%s/a"%self.work, "%s/a"%self.data_dir])
        
        self.ifdh_handle.cp([ "-r", "--force=gridftp", "%s/a"%self.data_dir, "%s/d"%self.work])

        #afterwards, should have 6 files in work/d
        l4 = glob.glob("%s/d/f*" % self.work)
        l5 = glob.glob("%s/d/b/f*" % self.work)
        l6 = glob.glob("%s/d/c/f*" % self.work)
        self.assertEqual(len(l4)+len(l5)+len(l6), 6)
        
        
    def test_dirmode(self):
        l1 = glob.glob("%s/a/f*" % self.work)
        l2 = glob.glob("%s/a/b/f*" % self.work)
        l3 = glob.glob("%s/a/c/f*" % self.work)
        os.mkdir('%s/d' % self.work)
        os.mkdir('%s/d/b' % self.work)
        os.mkdir('%s/d/c' % self.work)

        list = (["-D"] +
               l1 + ['%s/d'%self.work , ';'] +
               l2 + ['%s/d/b'%self.work, ';'] +
               l3 + ['%s/d/c'%self.work ] )
        
        self.ifdh_handle.cp(list)

        #afterwards, should have 6 files in work/d
        l4 = glob.glob("%s/d/f*" % self.work)
        l5 = glob.glob("%s/d/b/f*" % self.work)
        l6 = glob.glob("%s/d/c/f*" % self.work)
        self.assertEqual(len(l4)+len(l5)+len(l6), 6)

    def test_dirmode_globus(self):

        l1 = glob.glob("%s/a/f*" % self.work)
        l2 = glob.glob("%s/a/b/f*" % self.work)
        l3 = glob.glob("%s/a/c/f*" % self.work)
        os.mkdir('%s/d' % self.work)
        os.mkdir('%s/d/b' % self.work)
        os.mkdir('%s/d/c' % self.work)

        list = (["--force=gridftp", "-D"] +
               l1 + ['%s/d'%self.work , ';'] +
               l2 + ['%s/d/b'%self.work, ';'] +
               l3 + ['%s/d/c'%self.work ] )

        self.ifdh_handle.cp(list)

        #afterwards, should have 6 files in work/d
        l4 = glob.glob("%s/d/f*" % self.work)
        l5 = glob.glob("%s/d/b/f*" % self.work)
        l6 = glob.glob("%s/d/c/f*" % self.work)
        self.assertEqual(len(l4)+len(l5)+len(l6), 6)
        
    def test_dirmode_globus_args_order(self):

        l1 = glob.glob("%s/a/f*" % self.work)
        l2 = glob.glob("%s/a/b/f*" % self.work)
        l3 = glob.glob("%s/a/c/f*" % self.work)
        os.mkdir('%s/d' % self.work)
        os.mkdir('%s/d/b' % self.work)
        os.mkdir('%s/d/c' % self.work)

        list = (["-D",  "--force=gridftp" ] +
               l1 + ['%s/d'%self.work , ';'] +
               l2 + ['%s/d/b'%self.work, ';'] +
               l3 + ['%s/d/c'%self.work ] )

        self.ifdh_handle.cp(list)

        #afterwards, should have 6 files in work/d
        l4 = glob.glob("%s/d/f*" % self.work)
        l5 = glob.glob("%s/d/b/f*" % self.work)
        l6 = glob.glob("%s/d/c/f*" % self.work)
        self.assertEqual(len(l4)+len(l5)+len(l6), 6)
 
    
        
def suite():
    suite =  unittest.TestLoader().loadTestsFromTestCase(ifdh_cp_cases)
    return suite

if __name__ == '__main__':
    unittest.main()

