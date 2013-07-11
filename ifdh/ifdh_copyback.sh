#!/bin/sh

#set -x

#
# IFDH_STAGE_VIA is allowed to be a literal '$OSG_SITE_WRITE'
# (or generally $E for some environment variable E)
# so we eval it to expand that
#

init() {
 
    printf "ifdh copyback script starting for experiment %s\n" "${EXPERIMENT:-nova}"

    IFDH_STAGE_VIA="${IFDH_STAGE_VIA:-$OSG_SITE_WRITE}"
    eval IFDH_STAGE_VIA=\""${IFDH_STAGE_VIA}"\"

    host=`hostname --fqdn`
    filelist=${TMPDIR:-/tmp}/filelist$$
    wprefix="${IFDH_STAGE_VIA}/${EXPERIMENT:-nova}/ifdh_stage"

    #
    # paths with double slashes confuse srmls...
    #
    while :
    do
      case "$wprefix" in
      *//*//*)  wprefix=`echo "$wprefix" | sed -e 's;\(//.*/\)/;\1;'`;;
      *)  break;;
      esac
    done

    ifdh log "ifdh_copyback.sh: starting for ${EXPERIMENT} on $host location $wprefix"
    # avoid falling into a hall of mirrors
    unset IFDH_STAGE_VIA
}


get_first() {
   #debugging
   printf "Lock dir listing output:\n" >&2
   srmls -2 "$wprefix/lock" >&2
   printf "::\n" >&2


   srmls -2 "$wprefix/lock" | tail -2 | head -1  || 
      srmmkdir -2 "$wprefix/lock"
}

i_am_first() {
   get_first |  grep $uniqfile > /dev/null
}

expired_lock() {
   set : `get_first`
   first_file="$3"
   if [ "x$first_file" != "x" ]
   then
       printf "Checking lock file: $first_file ... "
       first_time=`echo $first_file | sed -e 's/.*_\(.*\)_.*/\1/' -e 's/T/ /'`
       first_secs=`date --date="$first_time" '+%s'`
       cur_secs=`date '+%s'`


       delta=$((cur_secs - first_secs))

       printf "$delta seconds old: "

       # if the lock is over 4 hours old, we consider it dead

       if [ $delta -gt 14400 ]
       then
	   lock_name=`basename $first_file`
	   printf "Breaking expired lock.\n"
           ifdh log "ifdh_copyback.sh: Breaking expired lock $lock_name\n"
	   srmrm $wprefix/lock/$lock_name
       else
           printf "Still valid.\n"
       fi
   fi
}

get_lock() {

   expired_lock

   datestamp=`date +%FT%T`
   uniqfile="t_${datestamp}_${host}_$$"

   echo lock > ${TMPDIR:-/tmp}/$uniqfile
   srmcp -2  file:///${TMPDIR:-/tmp}/$uniqfile $wprefix/lock/$uniqfile
   if i_am_first
   then
      sleep 5
      if i_am_first
      then
         echo "Obtained lock $uniquefile at `date`"
         ifdh log "ifdh_copyback.sh: Obtained lock $uniquefile at `date`"
         return 0  
      fi
   fi

   printf "Lock $lockfile already exists, someone else is already copying files.\n"
   srmrm $wprefix/lock/$uniqfile
   return 1
}

clean_lock() {
   printf "Removing lock: $wprefix/lock/$uniqfile\n"
   srmrm $wprefix/lock/$uniqfile
}

have_queue() {
   count=`srmls -2  $wprefix/queue | wc -l`
   [ $count -gt 2 ]
}

copy_files() {
   srmls -2 $wprefix/queue | (
     read dirname
     while read size filename
     do

         [ x$filename = x ] && continue

         rm -f ${filelist}
         filename=`basename $filename`

         printf "Fetching queue entry: $filename\n"
         srmcp  $wprefix/queue/$filename file:///${filelist}

         printf "starting copy..."
         ifdh log "ifdh_copyback.sh: starting copies for $filename"

         ifdh cp -f ${filelist}

         printf "completed.\n"
         ifdh log "ifdh_copyback.sh: finished copies for $filename"

         printf "Cleaning up for $filename:\n"
         while read src dest
         do
             printf "removing: $src\n"
             ifdh log "ifdh_copyback.sh: cleaned up $src"

             srmrm $src
             srcdir=`dirname $src`
         done < ${filelist}

         printf "removing: $srcdir\n"
         srmrmdir $srcdir

         printf "removing: $wprefix/queue/$filename\n"
         srmrm    $wprefix/queue/$filename
         ifdh log "ifdh_copyback.sh: cleaned up $filename"

         rm $filelist

     done
  )
}

copy_daemon() {
   init

   trap "clean_lock" 0 1 2 3 4 5 6 7 8 10 12

   while get_lock && have_queue
   do
       copy_files
       clean_lock
   done

   
   printf "Finished on $host at `date`\n"

   ifdh log "ifdh_copyback.sh: finished on $host"

   trap "" 0 1 2 3 4 5 6 7 8 10 12
}

copy_daemon
