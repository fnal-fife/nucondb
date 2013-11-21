import unittest
import ifdh
import socket
import os
import time

from projTests import SAMCases

from cpTests import ifdh_cp_cases

from lockTests import ifdh_lock_cases

def suite():
    basesuite = unittest.TestLoader().loadTestsFromTestCase(SAMCases)
    basesuite2 = unittest.TestLoader().loadTestsFromTestCase(SAMCases)
    basesuite3 =  unittest.TestLoader().loadTestsFromTestCase(ifdh_cp_cases)
    basesuite4 =  unittest.TestLoader().loadTestsFromTestCase(ifdh_lock_cases)
    suite = unittest.TestSuite( [basesuite,basesuite2,basesuite3,basesuite4] )
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite())

