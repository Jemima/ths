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
         lines[line[0]] = [line[4], max(float(line[11]), float(line[13])), max(float(line[12]), float(line[14])), line[21], line[22]]

   return lines
if __name__ == "__main__":
   parser = argparse.ArgumentParser(description="Collate multiple results files, and generate a summary")
   parser.add_argument("indir", type=str, help="Input directory")

   params = parser.parse_args()
   dirs = [os.path.join(params.indir, f) for f in os.listdir(params.indir)]
   info = {}
   for d in dirs:
      info[d] = {}
      files = [os.path.join(d, f) for f in os.listdir(d)]
      files.sort(key=os.path.getsize)
      files.reverse()
      for f in files:
         if fnmatch(f, "*.results"):
            bn = os.path.basename(f)
            info[d][bn] = parse(f)
   
   summary = {}
   for d in info:
      for run in info[d]:
         summary[run] = {}
         for f in info[d][run]:
            summary[run][f] = [[0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]

   for d in info:
      for run in info[d]:
         for f in info[d][run]:
            for n in range(0, len(info[d][run][f])):
               try:
                  field = info[d][run][f][n]
                  summary[run][f][n][1] += float(field)
                  summary[run][f][n][0] += 1.0
               except:
                  pass

   for run in summary:
      for f in summary[run]:
         for n in range(0, len(summary[run][f])):
            field = summary[run][f][n][1]/summary[run][f][n][0]
            summary[run][f][n] = field
         summary[run][f][2] = summary[run][f][2]/summary[run][f][1]
         summary[run][f][4] = summary[run][f][4]/summary[run][f][3]

   info = summary
   results = {}
   for outer in info:
      for inner in info[outer]:
         results[inner] = {}

   times = []
   for outer in info:
      time = outer[0:outer.rindex('.')]
      times.append(time)
      
      for inner in info[outer]:
         results[inner][time] = info[outer][inner]

   times.sort()
   times = sorted(times, key=lambda x:-float(x))
   for time in times:
      sys.stdout.write(time+",Number of Partitions, Number of BLEs (original), Increase in BLE Number, Clock Period (original) (ns), Clock Slowdown Factor,,")

   sys.stdout.write("\n")
   for file in results:
      sys.stdout.write(file)
      sys.stdout.write(",")
      for time in times:
         if time in results[file]:
            for res in results[file][time]:
               print("%.2f" % res, end=",")
               #sys.stdout.write(str(res)+",")
            sys.stdout.write(",,")
         else:
            sys.stdout.write("N/A,N/A,N/A,N/A,N/A,,,")
      sys.stdout.write("\n")


#We have structure resultfile -> bliffile -> results
#we want structure bliffile -> resultfile -> results
