#!/usr/bin/bash
for n in {1..10}
do
   mkdir out/$n
   echo "Run $n"
   for i in 1 2e-3 1e-3 9e-4 8e-4 5e-4 1e-4
   do
      echo "Target Recovery Time $i"
      ./generate.py -t -r $i -l out/$n/$i.log -R out/$n/$i.results blifs /tmp
   done
done
