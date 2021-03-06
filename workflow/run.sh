#!/usr/bin/env bash

Folder=outBFS
echo $Folder
mkdir /tmp/dsmu186blifs/$Folder
for n in {1..15}
do
   mkdir $Folder/$n
   echo "Run $n"
   for i in 1.2e-4
   do
      echo "Target Recovery Time $i"
      ./generate.py -n 4 -t -r $i -b -l $Folder/$n/$i.log -R $Folder/$n/$i.results blifs /tmp/dsmu186blifs/$Folder/
   done
done
