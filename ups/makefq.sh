#!/bin/sh

for eq in e2 e4
do
for q in debug prof debug:nu prof:nu
do
   for bf in Linux Linux64bit
   do
      for tf in 2.6-2.12 2.6-2.5
      do
         subdir=/`echo "$bf+$tf-$eq-$q" | sed -e 's/[^A-Za-z0-9]/-/g' -e 's/-nu//'`
         echo "  Flavor=$bf+$tf"
         echo "  Qualifiers=$eq:$q"
         echo "    _fq_subdir=$subdir"
         echo
         case $eq in
         e2) echo "    Action=setupbuild"
             echo "       setupRequired( gcc v4_7_1 )"
             echo "       setupRequired( python v2_7_3 -q gcc47 )"
             ;;
         e4) echo "    Action=setupbuild"
             echo "       setupRequired( gcc v4_8_1 )"
             echo "       setupRequired( python v2_7_5 )"
             ;;
         *)  echo "    Action=setupbuild"
             ;;
         esac
         echo 
      done
   done
done
done
