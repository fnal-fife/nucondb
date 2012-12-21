#!/bin/sh

#
# This script assumes it is run under a jobsub-generated wrapper
# which sets environment variables:
#  $SAM_PROJECT_NAME 
#  $EXPERIMENT
# and also has setup whatever is needed to find the art executable
# (i.e. "nova" or "gm2") for that experiment
#

# defaults
dest=""
conf=/dev/null
exe=$EXPERIMENT
vers=v1_0
renam=""
limit=""

datadir=$TMPDIR/ifdh_$$

#
# parse options we know, collect rest in $args
#
while [ $# -gt 0 ]
do
    case "x$1" in
    x-q|x--quals)   quals="$2"; shift; shift; continue;;
    x-t|x--template)templat="$2";  shift; shift; continue;;
    x-D|x--dest)    dest="$2";  shift; shift; continue;;
    x-R|x--rename)  renam="$2";   shift; shift; continue;;
    x-X|x--exe)     cmd="$2";   shift; shift; continue;;
    x-v|x--vers)    vers="$2";  shift; shift; continue;;
    x-R|x--rename)  renam="$2"; shift; shift; continue;;
    x-L|x--limit)   limit="$2"; shift; shift; continue
    *)              args="$args \"$1\""; shift; continue;;
    esac
    break
done

if [ "x$IFDHC_DIR" = "x" ] 
then
    . `ups setup ifdhc $vers -q $quals:`
fi

if [ "x$CPN_DIR" = "x" ] 
then
    . `ups setup cpn -z /grid/fermiapp/products/common/db`
fi

if [ "x$IFDH_BASE_URI" = "x" ]
then
    export IFDH_BASE_URI=http://samweb.fnal.gov:8480/sam/$EXPERIMENT/api
fi

hostname=`hostname --fqdn`
projurl=`ifdh findProject $SAM_PROJECT_NAME $EXPERIMENT`
sleep 5
consumer_id=`ifdh establishProcess $projurl demo 1 $hostname $GRID_USER "" "" "$limit"`

count=0
while :
do
    count=(($count + 1))

    IFDH_INPUT_FILE=`ifdh getNextFile $projurl/$consumer_id`

    if [ "x$IFDH_INPUT_FILE" = "x" ]
    then
        break
    fi

    conf=`echo $template | sed -e "s/\./_$count./"

    IFDH_OUTPUT_FILE=`echo $IFDH_INPUT_FILE | sed -e "$renam"`

    export IFDH_INPUT_FILE IFDH_OUTPUT_FILE

    sed -e "s/@IFDH_INPUT_FILE@/$IFDH_INPUT_FILE/g" \
        -e "s/@IFDH_OUTPUT_FILE@/$IFDH_OUTPUT_FILE/g"  < $template > $conf

    ifdh addOuputFile $IFDH_OUTPUT_FILE

    command="\"${cmd}\" -c \"$conf\" $args"
    echo "Running: $command"
    eval "$command"
    res=$?

done

if [ "x$renam" != "x" -a "$res" = "0" ]
then
    ifdh renameOutput $renam
fi

if [ "x$dest" != "x" -a "$res" = "0" ]
then
    ifdh copyBackOutput "$dest"
fi

if $update_via_fcl
then
    rm ${conf}
fi

# ifdh cleanup

exit $res
