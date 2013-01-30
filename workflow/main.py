from __future__ import print_function
import argparse
import tempfile
import os
import subprocess
import shutil
import sys
import time
from multiprocessing import Pool

if __name__ == "__main__":
   start = time.clock()
   parser = argparse.ArgumentParser(description="Partition and TMR a circuit")
   parser.add_argument("infile", type=str, help="Input blif format circuit")
   parser.add_argument("outfile", type=str, help="Output blif format circuit")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated file")
   parser.add_argument("-c", "--count", type=int, help="Maximum number of inputs to test")

   params = parser.parse_args()
   dir = tempfile.mkdtemp(dir=os.getcwd())+"/"
   try:
      dirTMR = dir+"TMR/"
      dirSplit = dir+"Split/"
      os.mkdir(dirTMR)
      os.mkdir(dirSplit)
      sys.stderr.write("Partitioning...\n")
      output = subprocess.check_output(["partitioner", "-f", params.infile, "-o", dirSplit, "-q"]).strip().split("\n")
      inputs = output[0]
      outputs = output[1]
      print(params.infile+"\t"+output[2],)
      
      sys.stderr.write("Triplicating...\n")
      
      pool = Pool(8);
      TMRArgs = [["python", "blifTMR.py", "voter.blif", dirSplit+file, dirTMR+file] for file in os.listdir(dirSplit)]
      pool.map(subprocess.check_call, TMRArgs, 4)
      sys.stderr.write("Merging...\n")
      subprocess.check_call(["python", "blifJoin.py", params.outfile, inputs, outputs, "-f", dirTMR+"*.blif"])
      if params.test:
         if params.count and params.count < inputs.count(' '):
            sys.stderr.write("Skipping test due to too many inputs\n")
         else:
            sys.stderr.write("Testing...\n")
            subprocess.check_call(["python", "test.py", params.infile, params.outfile])
   finally:
      shutil.rmtree(dir)
   start = time.clock()
   print(time.clock()-start)
