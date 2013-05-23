#!/usr/bin/env bash
for n in {1..10}
do
   mkdir out/$n
   echo "Run $n"
   for i in 1 2e-3 1e-3 9e-4 8.88e-4 8e-4 5e-4 2.5e-4 1e-4 5e-5 4.4e-5 1.49e-5 1.48e-5 1.47e-5 1e-5
   do
      echo "Target Recovery Time $i"
      ./generate.py -n 8 -t -r $i -l out/$n/$i.log -R out/$n/$i.results blifs /tmp
   done
done
