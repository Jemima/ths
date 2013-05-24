#!/usr/bin/env python

import os
import argparse
import sys
import pprint
from fnmatch import fnmatch

def parse(fileName):
   file = open(fileName, "r")
   state = 0 #0 -> header, 1, second header, 2->read line, 3->skip line
   lines = {}
   for line in file:
      if line[0] == '/':
         line = line.split()
         lines[line[0]] = [line[4], line[19], line[20]]

   return lines
if __name__ == "__main__":
   parser = argparse.ArgumentParser(description="Collate multiple results files, and generate a summary")
   parser.add_argument("indir", type=str, help="Input directory")

   params = parser.parse_args()
   files = [os.path.join(params.indir, f) for f in os.listdir(params.indir)]
   files.sort(key=os.path.getsize)
   files.reverse()
   info = {}
   for f in files:
      if fnmatch(f, "*.results"):
         info[f] = parse(f)
   
   results = {}
   for outer in info:
      for inner in info[outer]:
         results[inner] = {}

   times = []
   for outer in info:
      time = outer[len(params.indir)+1:outer.rindex('.')]
      times.append(time)
      
      for inner in info[outer]:
         results[inner][time] = info[outer][inner]

   times.sort()
   times = sorted(times, key=lambda x:-float(x))
   for time in times:
      sys.stdout.write(time+",NumPartitions,Frequency Base (s),Frequency TMR (ns),Slowdown Factor,,")

   sys.stdout.write("\n")
   for file in results:
      sys.stdout.write(file)
      sys.stdout.write(",")
      for time in times:
         if time in results[file]:
            for res in results[file][time]:
               sys.stdout.write(res+",")
            sys.stdout.write(
                  str(
                     float(results[file][time][2]) /
                     float(results[file][time][1])
                     )
                  )
            sys.stdout.write(",,,")
         else:
            sys.stdout.write("N/A,N/A,N/A,N/A,,,")
      sys.stdout.write("\n")


#We have structure resultfile -> bliffile -> results
#we want structure bliffile -> resultfile -> results
