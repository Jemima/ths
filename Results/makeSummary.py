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
         lines[line[0]] = [line[1], line[19], line[20]]

   return lines
if __name__ == "__main__":
   parser = argparse.ArgumentParser(description="Collate multiple results files, and generate a summary")
   parser.add_argument("indir", type=str, help="Input directory")
   parser.add_argument("outfile", type=str, help="Output summary file")

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

   for outer in info:
      sys.stdout.write(outer+"\t")
      for inner in info[outer]:
         results[inner][outer] = info[outer][inner]

   pp = pprint.PrettyPrinter()
   pp.pprint(results)
   for file in results:
      sys.stdout.write(file)
      sys.stdout.write("\t")
      for run in results[file]:
         for res in results[file][run]:
            sys.stdout.write(res+"\t")
      print("\n")


#We have structure resultfile -> bliffile -> results
#we want structure bliffile -> resultfile -> results
