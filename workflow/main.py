#!/usr/bin/python
import argparse
import tempfile
import os
import subprocess
import shutil
from multiprocessing import Pool

if __name__ == "__main__":
   parser = argparse.ArgumentParser(description="Partition and TMR a circuit")
   parser.add_argument("infile", type=str, help="Input blif format circuit")
   parser.add_argument("outfile", type=str, help="Output blif format circuit")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated file")

   params = parser.parse_args()
   dir = tempfile.mkdtemp(dir=os.getcwd())+"/"
   try:
      dirTMR = dir+"TMR/"
      dirSplit = dir+"Split/"
      os.mkdir(dirTMR)
      os.mkdir(dirSplit)
      print("Partitioning...\n")
      output = str(subprocess.check_output(["./partitioner", "-f", params.infile, "-o", dirSplit])).strip().split("\n")
      inputs = output[0]
      outputs = output[1]
      
      print("Triplicating...\n")
      
      pool = Pool(8);
      TMRArgs = [["python", "blifTMR.py", "voter.blif", dirSplit+file, dirTMR+file] for file in os.listdir(dirSplit)]
      pool.map(subprocess.check_call, TMRArgs, 4)
      print("Merging...\n")
      subprocess.check_call(["python", "blifJoin.py", params.outfile, inputs, outputs, "-f", dirTMR+"*.blif"])
      if params.test:
         print("Testing...\n")
         subprocess.check_call(["python", "test.py", params.infile, params.outfile])
   finally:
      shutil.rmtree(dir)
