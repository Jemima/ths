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
   parser.add_argument("-p", "--criticalpath", type=float, help="Critical path length")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated file")
   parser.add_argument("-c", "--count", type=int, help="Maximum number of inputs to test")
   parser.add_argument("-a", "--area", type=int, help="Maximum area to pass to partitioner")
   parser.add_argument("-n", "--numthreads", type=int, help="Maximum number of threads to use")

   params = parser.parse_args()
   dir = tempfile.mkdtemp(dir=os.getcwd())+"/"
   step1=0
   step2=0
   step3=0
   step4=0
   try:
      dirTMR = dir+"TMR/"
      dirSplit = dir+"Split/"
      os.mkdir(dirTMR)
      os.mkdir(dirSplit)
      sys.stderr.write("Partitioning...\n")
      sys.stderr.write(params.outfile)
      critical = params.criticalpath
      if critical == None:
         critical = 0.00000001
      area = params.area
      if area == None:
         area = 1000
      output = subprocess.check_output(["./partitioner", "-q", "-a", str(area), "-r", "1", "-f", params.infile, "-o", dirSplit]).decode(sys.stdout.encoding).strip().split('\n')
      inputs = output[0] #The original inputs and outputs. We save these, since in the process of splitting loops, creating partitions, etc, we create a lot of extra inputs and outputs,
      outputs = output[1] #and we aren't able to tell which are supposed to be present in the end result otherwise
      print(params.infile+"\t"+output[2],end='\t')
      step1 = time.clock()
      
      sys.stderr.write("Triplicating...\n")
      threads = params.numthreads
      if threads == None:
         pool = Pool(4)
      else:
         pool = Pool(threads);
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
      subprocess.check_call(["python", "blifJoin.py", dir+"file.blif", dir+"header.blif", "-f", dirTMR+"*.blif"])
      
      step3 = time.clock()
      sys.stderr.write("Flattening...\n");
      subprocess.check_output(["./abc", "-o", params.outfile, "-c", "echo", dir+"file.blif"])
      #ABC has a few assorted bugs. 1. It strips clock information from latches, resulting in invalid blif files. Assume there is only one clock for all latches (true for MCNC) and sed to fix it up
      try:
         latch = str(subprocess.check_output(["grep", "-m", "1", "\.latch", params.infile]), 'UTF-8').split()
         subprocess.check_call(["sed", "-ri", "s/(\\.latch.+)(2)/\\1 "+latch[3]+" "+latch[4]+" 2/", params.outfile])
      except subprocess.CalledProcessError: #Ignore these errors. Grep returns 1 when no match found, which causes an exception
         pass
      if params.test:
         if params.count and params.count < inputs.count(' '):
            sys.stderr.write("Skipping test due to too many inputs\n")
         else:
            import mmap
            f = open(params.outfile)
            s = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
            if s.find(bytes('.latch', 'UTF-8')) != -1:
               sys.stderr.write("Latches present.\n")
               output = str(subprocess.check_output(["./abc", "-c", "dsec "+params.infile+" "+params.outfile]), "UTF-8")
               sys.stderr.write(output)
               if not "are equivalent" in output:
                  exit(5)
               s.close()
            else:
               s.close()
               sys.stderr.write("Testing...\n")
               output = str(subprocess.check_output(["./abc", "-c", "cec "+params.infile+" "+params.outfile]), "UTF-8")
               sys.stderr.write(output)
               if not "are equivalent" in output:
                  exit(5)
               #subprocess.check_call(["python", "test.py", params.infile, params.outfile])
      step4 = time.clock()
   finally:
      shutil.rmtree(dir)
      print(step1-start, end='\t')
      print(step2-step1, end='\t')
      print(step3-step2, end='\t')
      print(step4-step3, end='\t')
      print(time.clock()-start, end='\t')
      print(len(TMRArgs), end='\n')
