#!/usr/bin/python

#
# script to present data from a python shelf file
# to net-snmp in a table format.
#
# most of the code is parsing OIDs and finding the
# "next" oid.
#
# The rest is using a hash function to present the
# keys of the shelf in a consistent position.
#

def parseoid(oid):
    list = oid.split('.')
    while len(list) < 12:
        list.append('0')
    oid = '.'.join(list)
    return oid, list[10], list[11]

def replaceoid(oid, row, col):
    list = oid.split('.')
    list[10] = row
    list[11] = col
    list = list[:12]
    return '.'.join(list)

def nextoid(oid, row, col):

    if row == '' and col == '':
        return oid + '.1.0'

    if col == '':
        return oid + '.0'

    col = int(col) + 1

    if col > maxcols: 
        row = int(row) + 1
        col = 1

    row = str(row)
    col = str(col)
    return replaceoid(oid, row, col), row, col

def check(row, col):
    if row == '' or col == '':
        sys.exit()
    if int(row) > maxrows  or int(col) > maxcols:
        sys.exit()

import math
def hash(s,k=5):
   k2 = 2 * k
   k2s = k2 * k2
   sum = 0
   for c in s:
      sum = (sum * 2) + ord(c)
   return  int(round((math.log(sum,2) + sum % k2s)/k))-k2


import syslog
import sys
import shelve

# log our call...
syslog.syslog(' '.join(sys.argv))

counters =  shelve.open('/tmp/jobstats.shelf','r')

tags = counters.keys()
maxcols = 0
a = []
for k in counters.keys():
    h = hash(k)
    if h > maxcols:
        maxcols = h

for i in range(0,maxcols+1):
    a.append("NA")

for k in counters.keys():
    h = hash(k)
    a[h] = k

if len(sys.argv) != 3:
   sys.exit()

flag = sys.argv[1]
oid = sys.argv[2]
maxrows = 3

oid, row, col = parseoid(oid)

if flag == '-n':
    oid, row, col  = nextoid(oid, row, col)

check(row, col)

print oid
if row == '0':
    print 'integer'
    print maxrows
if row == '1':
    print 'integer'
    print col
if row == '2':
    print 'string'
    print a[int(col)]
if row == '3':
    print 'integer'
    print counters[a[int(col)]]
