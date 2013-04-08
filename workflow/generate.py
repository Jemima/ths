#!/usr/bin/python
from __future__ import print_function
import argparse
import tempfile
import os
import subprocess
import shutil
import sys
import time
import glob
import re
from multiprocessing import Pool
from fnmatch import fnmatch


def parseOutput(s):
   out = ''
   stats = re.findall('(\d+ LUTs of size \d+)', s)
   for st in stats:
      out += str(st)+','
   stats = re.findall('(\d+ of type .+)', s)
   for st in stats:
      out += str(st)+','
   out += 'size: '+re.findall('mapped into a (\d+)', s)[0]+','
   out += 'chWidth: '+re.findall('channel width factor of (\d+)', s)[0]+','
   out += 'Area: '+re.findall('Total used logic block area: (.+)', s)[0]+','
   out += 'CritPathNets: '+re.findall('Nets on critical path: (.+)\.', s)[0]+','
   out += 'Latency: '+re.findall('Final critical path: (.+)', s)[0]+','
   out += 'Time: '+re.findall('VPR took (\d+)', s)[0]+','
   return out


if __name__ == "__main__":
   start = time.clock()
   parser = argparse.ArgumentParser(description="TMRs a whole bunch of circuits, then runs them through VPR, and collects results")
   parser.add_argument("indir", type=str, help="Input directory containing blif files")
   parser.add_argument("outdir", type=str, help="Output directory")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated files")
   parser.add_argument("-l", "--log", type=str, help="Location of log file")
   parser.add_argument("-v", "--novpr", action="store_true", help="Skip running VPR")
   parser.add_argument("-a", "--area", type=int, help="Max partition area")

   params = parser.parse_args()
   if params.test:
      test = "-t"
   else:
      test = ""
   try:
      dir = tempfile.mkdtemp(dir=os.getcwd())+"/"
      for f in os.listdir(params.indir):
         if fnmatch(f, "*.blif"):
            sys.stderr.write(f)
            ipath = os.path.abspath(params.indir)+"/"+f
            opath = os.path.abspath(params.outdir)+"/"+f
            try:
               path = "0.00000001"
               if params.novpr == False:
                  output2 = str(subprocess.check_output(["./vpr",  "--full_stats", "--route_file", "temp", "--net_file", "temp2", "--place_file", "temp3", "arch.xml", ipath], stderr=sys.stdout), 'UTF-8')
                  path = re.findall('Final critical path: (.+) ns', output2)[0]
                  path = float(path)*1E-9
               if params.test:
                  sys.stdout.flush()
                  output = str(subprocess.check_output(["./main.py", '-t', '-a', str(params.area), ipath, opath, '-p', str(path)], stderr=sys.stdout), 'UTF-8')
               else:
                  sys.stdout.flush()
                  output = str(subprocess.check_output(["./main.py", '-a', str(params.area), ipath, opath], stderr=sys.stdout), 'UTF-8')
               print(output)
               if params.novpr == False:
                  output3 = str(subprocess.check_output(["./vpr",  "--full_stats", "--route_file", "temp", "--net_file", "temp2", "--place_file", "temp3", "arch.xml", opath], stderr=sys.stdout), 'UTF-8')
                  print(parseOutput(output2))
                  print(parseOutput(output3))
               print("\n\n******************************\n\n")
               sys.stdout.flush()
            except Exception as e:
               print(e)
               pass
   finally:
      shutil.rmtree(dir)
