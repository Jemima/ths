#!/usr/bin/env bash

Folder=outBFS
echo $Folder
for n in {1..10}
do
   mkdir $Folder/$n
   echo "Run $n"
   for i in 1e-3 2.5e-4 7.5e-5
   do
      echo "Target Recovery Time $i"
      ./generate.py -n 4 -t -r $i -b -l $Folder/$n/$i.log -R $Folder/$n/$i.results blifs /tmp
   done
done


Folder=outFixedChWidth66
echo $Folder
for n in {1..10}
do
   mkdir $Folder/$n
   echo "Run $n"
   for i in 7.5e-5
   do
      echo "Target Recovery Time $i"
      ./generate.py -n 4 -t -r $i -w 66 -l $Folder/$n/$i.log -R $Folder/$n/$i.results blifs /tmp
   done
done

