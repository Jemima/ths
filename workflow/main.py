#!/usr/bin/env python3
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
   parser.add_argument("-l", "--latency", type=float, help="Circuit operating frequency in seconds")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated file")
   parser.add_argument("-c", "--count", type=int, help="Maximum number of inputs to test")
   parser.add_argument("-b", "--breadthfirst", action="store_true", help="Partitioner should use worse breadth first traversal")
   parser.add_argument("-r", "--recoverytime", type=str, help="Maximum per partition recovery time")
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
      critical = params.latency
      if critical == None:
         critical = 0.00000001
      recoverytime = params.recoverytime
      if recoverytime == None:
         recoverytime = "1e-2" 
      bfs = params.breadthfirst
      pars = ["./partitioner", "-q", "-r", str(recoverytime), "-f", params.infile, "-o", dirSplit, '-p', str(critical)]
      if bfs != None and bfs == True:
         pars.append("-b")
      try:
         output = subprocess.check_output(pars).decode(sys.stdout.encoding).strip().split('\n')
      except subprocess.CalledProcessError as e:
         sys.stderr.write("Partitioner error: "+str(e.output, 'UTF-8')+"\n")
         raise
      inputs = output[0] #The original inputs and outputs. We save these, since in the process of splitting loops, creating partitions, etc, we create a lot of extra inputs and outputs,
      outputs = output[1] #and we aren't able to tell which are supposed to be present in the end result otherwise
      print(params.infile+"\t"+output[2],end='\t')
      partitions = output[3:]
      step1 = time.clock()
      
      sys.stderr.write("Triplicating...\n")
      threads = params.numthreads
      if threads == None:
         pool = Pool(4)
      else:
         pool = Pool(threads);
      TMRArgs = [["python3", "blifTMR.py", "voter.blif", dirSplit+file, dirTMR+file] for file in os.listdir(dirSplit)]
      pool.map(subprocess.check_call, TMRArgs, 4)
      step2 = time.clock()
      sys.stderr.write("Merging...\n")
      header = open(dir+"header.blif", 'w')
      header.write(".model main\n.inputs ")
      header.write(inputs)
      header.write("\n.outputs ")
      header.write(outputs)
      header.close()
      subprocess.check_call(["python3", "blifJoin.py", dir+"file.blif", dir+"header.blif", "-f", dirTMR+"*.blif"])
      
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
                  exit(27)
               s.close()
            else:
               s.close()
               sys.stderr.write("Testing...\n")
               output = str(subprocess.check_output(["./abc", "-c", "cec "+params.infile+" "+params.outfile]), "UTF-8")
               sys.stderr.write(output)
               if not "are equivalent" in output:
                  exit(5)
               #subprocess.check_call(["python3", "test.py", params.infile, params.outfile])
                  exit(27)
               #subprocess.check_call(["python", "test.py", params.infile, params.outfile])
      step4 = time.clock()
      print(step1-start, end='\t')
      print(step2-step1, end='\t')
      print(step3-step2, end='\t')
      print(step4-step3, end='\t')
      print(time.clock()-start, end='\t')
      print(len(TMRArgs), end='\n')
      for s in partitions:
         print(s)
      shutil.rmtree(dir)
   except subprocess.CalledProcessError as e:
      sys.stderr.write("Subprocess error: "+str(e.output))
      sys.stderr.write("\n")
      exit(e.returncode)
   print("\n")
