import argparse
import shutil
import re
import tempfile
import os
import itertools as iter
import subprocess

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
    
parser = argparse.ArgumentParser(description="Exhaustively simulate a purely combinational circuit using SIS")
parser.add_argument("reference", type=str, help="Reference blif format circuit")
parser.add_argument("test", type=str, help="Blif format circuit to test")

args = parser.parse_args()
dir = tempfile.mkdtemp(dir=os.getcwd())+"/"
try:
   failed = False
   file = open(args.reference)
   readContinuedLine(file)
   numInputs = len(readContinuedLine(file).split(' '))-1 #-1 since it includes the .inputs
   for x in iter.product("01", repeat=numInputs):
      output = subprocess.check_output(["sis.exe", "-o", "NUL", "-c", "simulate "+' '.join(x), args.reference]).strip()
      m = re.search("Outputs: ([01 ]+)", output)
      reference = m.group(1).split(' ')
      output = subprocess.check_output(["sis.exe", "-o", "NUL", "-c", "simulate "+' '.join(x), args.test]).strip()
      m = re.search("Outputs: ([01 ]+)", output)
      test = m.group(1).split(' ')
      if reference != test:
         failed = True
         print "Failed test for inputs: "+str(x)
   if failed == False:
      print "All outputs match"
finally:
   shutil.rmtree(dir)