#!/usr/bin/env bash

Folder=outBFS
echo $Folder
mkdir /tmp/dsmu186blifs/$Folder
for n in {6..15}
do
   mkdir $Folder/$n
   echo "Run $n"
   for i in 1e-3 2.5e-4 7.5e-5
   do
      echo "Target Recovery Time $i"
      ./generate.py -n 2 -t -r $i -b -l $Folder/$n/$i.log -R $Folder/$n/$i.results blifs /tmp/dsmu186blifs/$Folder/
   done
done
