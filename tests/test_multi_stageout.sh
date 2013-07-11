

#
# run 5 copies of an ifdhc cp with IFDH_STAGE_VIA set
# check output files to make sure only one runs the copyback
#
export OSG_SITE_WRITE="srm://fndca1.fnal.gov:8443//pnfs/fnal.gov/usr/fermigrid/volatile"
export IFDH_STAGE_VIA='$OSG_SITE_WRITE' 

watch ps --forest &
watchpid=$!

# cleanup  past attempts...
for i in 1 2 3 4 5 
do
   srmrm srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=/grid/data/mengel/nucondb-client-$i.tgz
done

cppid=""

for i in 1 2 3 4 5 
do 
  ifdh cp nucondb-client.tgz /grid/data/mengel/nucondb-client-$i.tgz > out_$i 2>&1 &   
  cppid="$cppid $!"
done

wait $cppid
kill $watchpid

assert() {
   test "$@" || echo "Failed."
}

assert `grep -l 'someone else'  out_[1-5] | wc -w` = 4
assert `grep -l 'Obtained lock' out_[1-5] | wc -w` = 1

srmls srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=/grid/data/mengel/

# cleanup 
for i in 1 2 3 4 5 
do
   srmrm srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=/grid/data/mengel/nucondb-client-$i.tgz
done

srmls srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=/grid/data/mengel/
