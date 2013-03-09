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
from multiprocessing import Pool
from fnmatch import fnmatch

if __name__ == "__main__":
   start = time.clock()
   parser = argparse.ArgumentParser(description="TMRs a whole bunch of circuits, then runs them through VPR, and collects results")
   parser.add_argument("indir", type=str, help="Input directory containing blif files")
   parser.add_argument("outdir", type=str, help="Output directory")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated files")
   parser.add_argument("-l", "--log", type=str, help="Location of log file")

   params = parser.parse_args()
   if params.test:
      test = "-t"
   else:
      test = ""
   try:
      dir = tempfile.mkdtemp(dir=os.getcwd())+"/"
      for f in os.listdir(params.indir):
         if fnmatch(f, "*.blif"):
            ipath = os.path.abspath(params.indir)+"/"+f
            opath = os.path.abspath(params.outdir)+"/"+f
            output = subprocess.check_output(["./main.py", test, ipath, opath])
            print(output)
            output2 = subprocess.check_output(["./vpr",  "--full_stats", "--route_file", "temp", "--net_file", "temp2", "--place_file", "temp3", "arch.xml", opath]) 
            print(output2)

   finally:
      shutil.rmtree(dir)
