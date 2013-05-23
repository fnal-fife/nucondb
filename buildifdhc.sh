# buildifdhc.sh <opt|debug|prof> [e2|gcc47]
# default is gcc47


extraqual=${2}
basequal=${1}

fullqual=${basequal}:${extraqual}
qualdir=${basequal}-${extraqual}

if [ "x$basequal" = "x" -o "x$extraqual" = "x" ]
then
    echo "usage: $0 <debug|prof|opt> e2"
    exit 1
fi

# use the art externals python, too
. `ups setup python v2_7_3 -q gcc47` || true

case $basequal in
debug) ARCH="-std=c++11 -O0 -g";;
prof)  ARCH="-std=c++11 -g -p -pg";;
opt)   ARCH="-std=c++11 -O2";;
esac
export ARCH

export PYTHON_CONF=`echo $PYTHON_LIB/python*/config`

fqdir="`ups flavor -4`-$qualdir" 

for d in ifbeam ifdh nucondb numsg ups util
do
   mkdir -p $fqdir/$d
   cp $d/Makefile $fqdir/$d/Makefile
done

(cd $fqdir && make -f ../Makefile all install-libs)

make install-headers
