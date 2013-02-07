#!/usr/bin/python
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
      print("Partitioning...\n")
      sys.stderr.write("Partitioning...\n")
      output = subprocess.check_output(["./partitioner", "-f", params.infile, "-o", dirSplit]).decode(sys.stdout.encoding).strip().split('\n')
      inputs = output[0] #The original inputs and outputs. We save these, since in the process of splitting loops, creating partitions, etc, we create a lot of extra inputs and outputs,
      outputs = output[1] #and we aren't able to tell which are supposed to be present in the end result otherwise
      print(params.infile+"\t"+output[2],end='\t')
      step1 = time.clock()
      
      sys.stderr.write("Triplicating...\n")
      
      pool = Pool(8);
      TMRArgs = [["python", "blifTMR.py", "voter.blif", dirSplit+file, dirTMR+file] for file in os.listdir(dirSplit)]
      pool.map(subprocess.check_call, TMRArgs, 4)
      step2 = time.clock()
      sys.stderr.write("Merging...\n")
      header = open(dir+"header.blif", 'w')
      header.write(".model main\n.inputs ")
      header.write(inputs)
      header.write("\n.outputs ")
      header.write(outputs)
      header.close()
      subprocess.check_call(["python", "blifJoin.py", params.outfile, dir+"header.blif", "-f", dirTMR+"*.blif"])
      step3 = time.clock()
      if params.test:
         if params.count and params.count < inputs.count(' '):
            sys.stderr.write("Skipping test due to too many inputs\n")
         else:
            import mmap
            f = open(params.outfile)
            s = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
            if s.find('.latch') != -1:
               sys.stderr.write("Latches present, skipping testing...\n")
               s.close()
            else:
               s.close()
               sys.stderr.write("Testing...\n")
               subprocess.check_call(["python", "test.py", params.infile, params.outfile])
      step4 = time.clock()
   finally:
      #shutil.rmtree(dir)
      print(step1-start, end='\t')
      print(step2-step1, end='\t')
      print(step3-step2, end='\t')
      print(step4-step3, end='\t')
      print(time.clock()-start, end='\n')
