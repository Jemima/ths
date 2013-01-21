from __future__ import division
import argparse
import shutil
import re
import tempfile
import os
import itertools as iter
import subprocess
import sys
from multiprocessing import Pool
from functools import partial


def readContinuedLine(f):
    buffer = ""
    while(True):
        buffer = buffer + f.readline()
        if len(buffer) == 0 or buffer.isspace():
         continue
       # print("%s, %s"%(buffer, buffer[-2]))
        if buffer[-2] == '\\':
            buffer = buffer[:-2]
        else:
            break
    return buffer.strip()

def doCall(values, referenceFile, testFile):
   output = subprocess.check_output(["sis.exe", "-o", "NUL", "-c", "simulate "+' '.join(values), referenceFile]).strip()
   m = re.search("Outputs: ([01 ]+)", output)
   reference = m.group(1).split(' ')
   output = subprocess.check_output(["sis.exe", "-o", "NUL", "-c", "simulate "+' '.join(values), testFile]).strip()
   m = re.search("Outputs: ([01 ]+)", output)
   test = m.group(1).split(' ')
   if reference != test:
      return values
   return True
   
def handleResults(res):
   if res != True:
      print "Failed test for inputs: "+str(res)
   global counter
   counter += 1
      
   
if __name__ == '__main__':
   parser = argparse.ArgumentParser(description="Exhaustively simulate a purely combinational circuit using SIS")
   parser.add_argument("reference", type=str, help="Reference blif format circuit")
   parser.add_argument("test", type=str, help="Blif format circuit to test")
   
   args = parser.parse_args()
   failed = False
   file = open(args.reference)
   readContinuedLine(file)
   numInputs = len(readContinuedLine(file).split(' '))-1 #-1 since it includes the .inputs
   total = 2**numInputs
   pool = Pool(8);
   partialCall = partial(doCall, referenceFile=args.reference, testFile=args.test)

   for i, res in enumerate(pool.imap_unordered(partialCall, iter.product("01", repeat=numInputs), 4)):
       sys.stderr.write('\r{0:%} done'.format(i/total))
       if res != True:
         print "Failed on values: "+str(res)