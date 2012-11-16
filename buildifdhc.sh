# buildifdhc.sh <opt|debug|prof> [e2|gcc47]
# default is gcc47

extraqual=${1}
basequal=${2}

fullqual=${basequal}:${extraqual}
qualdir=${basequal}-${extraqual}

if [ "x$SETUP_ART" = "x" ]
then
   echo "Must have art setup"
   exit 1
fi
if [ "x$basequal" = "x" -o "x$extraqual" = "x" ]
then
    echo "usage: $0 <debug|prof|opt> e2"
    exit 1
fi

# use the art externals python, too
setup python v2_7_3 -q gcc47
# fix hosey PYTHON_LIB
export PYTHON_LIB=$PYTHON_ROOT/lib

case $basequal in
debug) ARCH="-g";;
prof)  ARCH="-g -p -pg";;
opt)   ARCH="-O2";;
esac

make withart
make DESTDIR="`ups flavor -4`-$qualdir/" install

