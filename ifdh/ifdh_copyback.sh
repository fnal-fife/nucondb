
if [ ${OSG_SITE_WRITE:-x} = x ]
then
OSG_SITE_WRITE="srm://fndca1.fnal.gov:8443/pnfs/fnal.gov/usr/fermigrid/volatile"
fi

wprefix="${OSG_SITE_WRITE}/${EXPERIMENT:-nova}/ifdh_stage"

host=`hostname --fqdn`

filelist=${TMPDIR:-/tmp}/filelist$$

get_first() {
   srmls $wprefix/lock | head -2 | tail -1  || 
      srmmkdir $wprefix/lock
}

i_am_first() {
   get_first |  tee /dev/tty | grep $uniqfile > /dev/null
}

expired_lock() {
   set : `get_first`
   first_file="$3"
   if [ "x$first_file" != "x" ]
   then
       first_time=`echo $first_file | cut -c 1-19 | sed -e 's/T/ /'`
       first_secs=`date --date="$first_time" '+%s'`
       cur_secs=`date '+%s'`

       delta=$((cur_secs - first_secs))

       # if the lock is over 2 hours old, we consider it dead

       if [ $delta -gt 7200 ]
       then
	   lock_name=`basename $first_file`
	   echo "Breaking expired lock $lock_name"
	   srmrm $wprefix/lock/$lock_name
       fi
   fi
}

get_lock() {

   expired_lock

   datestamp=`date +%FT%T`
   uniqfile="${host}_${datestamp}_$$"

   echo lock > ${TMPDIR:-/tmp}/$uniqfile
   srmcp -2  file:///${TMPDIR:-/tmp}/$uniqfile $wprefix/lock/$uniqfile
   if i_am_first
   then
      sleep 5
      if i_am_first
      then
         return 0  
      fi
   fi

   echo "Unable to get lock"
   srmrm $wprefix/lock/$uniqfile
   return 1
}

clean_lock() {
   srmrm $wprefix/lock/$uniqfile
}

have_queue() {
   count=`srmls $wprefix/queue | wc -l`
   [ $count -gt 2 ]
}

copy_files() {
   srmls $wprefix/queue | (
     read dirname
     while read size filename
     do

         [ x$filename = x ] && continue

         rm -f ${filelist}
         filename=`basename $filename`
         srmcp  $wprefix/queue/$filename file:///${filelist}

         ifdh cp -f ${filelist}

         while read src dest
         do
             srmrm $src
             srcdir=`dirname $src`
         done < ${filelist}

         srmrmdir $srcdir
         srmrm    $wprefix/queue/$filename

         rm $filelist

     done
  )
}

copy_daemon() {
   trap "clean_lock" 0 1 2 3 4 5 6 7 8 10 12

   while get_lock && have_queue
   do
       copy_files
       clean_lock
   done

   trap "" 0 1 2 3 4 5 6 7 8 10 12
}

copy_daemon
