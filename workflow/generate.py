#!/usr/bin/python -u
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
   out = {}
   stats = re.findall('(\d+) (LUTs of size \d+)', s)
   for st in stats:
      out[st[1]] = st[0]
   stats = re.findall('(\d+) of (type .+)', s)
   for st in stats:
      out[st[1]] = st[0]
   out["PlacementWidth"] = re.findall('mapped into a (\d+)', s)[0]
   out["channelWidth"] = re.findall('channel width factor of (\d+)', s)[0]
   out["UsedArea"] = re.findall('Total used logic block area: (.+)', s)[0]
   out["Nets"] = re.findall('Nets on critical path: (.+)\.', s)[0]
   out["LogicDelay"] = re.findall('Total logic delay: (\S+)', s)[0]
   out["NetDelay"] = re.findall('total net delay: (\S+)', s)[0]
   out["Period"] = re.findall('Final critical path: (\S+)', s)[0]
   out["Duration"] = re.findall('VPR took ([\d.+-,]+)', s)[0]
   return out

def doRun(args): 
   ipath = args[0]
   opath = args[1]
   mainParams = args[2]
   novpr = args[3]
   logPath = args[4]
   if logPath == None:
      log = open(os.devnull, 'w')
   else:
      log = open(logPath, 'a')
   par = ["./main.py"]
   par.extend(mainParams)
   par.append(ipath)
   par.append(opath)
   dir = tempfile.mkdtemp(dir=os.getcwd())+"/"
   ret = []
   try:
      path = "0.00000001"
      if novpr == False:
         try:
            output2 = str(subprocess.check_output(["./vpr", "--route_file", dir+"file.route", "--place_file", dir+"file.place", "--net_file", dir+"file.net", "--full_stats", "arch.xml", ipath], stderr=subprocess.STDOUT), 'UTF-8')
         except subprocess.CalledProcessError as e:
            log.write(str(e.output, 'UTF-8'))
            sys.stderr.write(str(e))
            pass
            return {"error": ["./vpr", "--route_file", dir+"file.route", "--place_file", dir+"file.place", "--net_file", dir+"file.net", "--full_stats", "arch.xml", ipath]}
         path = re.findall('Final critical path: (.+) ns', output2)[0]
         path = float(path)*1E-9
      sys.stdout.flush()

      discard = open(os.devnull, 'w')
      ret = str(subprocess.check_output(par, stderr=log), 'UTF-8')
      ret = {"times": ret.split("\t")}
      if novpr == False:
         try:
            output3 = str(subprocess.check_output(["./vpr", "--route_file", dir+"file.route", "--place_file", dir+"file.place", "--net_file", dir+"file.net", "--full_stats", "arch.xml", opath], stderr=subprocess.STDOUT), 'UTF-8')
         except subprocess.CalledProcessError as e:
            log.write(str(e.output, 'UTF-8'))
            sys.stderr.write(str(e))
            pass
            return {"error": ["./vpr", "--route_file", dir+"file.route", "--place_file", dir+"file.place", "--net_file", dir+"file.net", "--full_stats", "arch.xml", ipath]}
         ret["out2"] = parseOutput(output2)
         ret["out3"] = parseOutput(output3)
      sys.stdout.flush()
   finally:
      shutil.rmtree(dir)
   log.write(str(ret))
   log.write("\n")
   log.close()
   return ret

def formatResults(res):
   times = res['times']
   base = res['out2']
   tmr = res['out3']
   fields = ['channelWidth', 'type input', 'type output', 'type names', 'type latch', 'NetDelay', 'LogicDelay', 'Period']
   sRet = "{0}\t{1}\t".format(times[0], times[9].strip())
   for field in fields:
      sRet += "{0}\t{1}\t".format(base[field], tmr[field])

   return sRet+"\n"


if __name__ == "__main__":
   start = time.clock()
   parser = argparse.ArgumentParser(description="TMRs a whole bunch of circuits, then runs them through VPR, and collects results")
   parser.add_argument("indir", type=str, help="Input directory containing blif files")
   parser.add_argument("outdir", type=str, help="Output directory")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated files")
   parser.add_argument("-r", "--results", type=str, help="Location of results file", required=True)
   parser.add_argument("-l", "--log", type=str, help="Location of log file")
   parser.add_argument("-v", "--novpr", action="store_true", help="Skip running VPR")
   parser.add_argument("-a", "--area", type=int, help="Max partition area")
   parser.add_argument("-n", "--numthreads", type=int, help="Maximum number of threads to use. Defaults to 4")

   params = parser.parse_args()
   results = open(params.results, 'w')
   if params.test:
      test = "-t"
   else:
      test = ""
   threads = params.numthreads
   area = params.area
   test = params.test
   novpr = params.novpr
   if novpr == None:
      novpr = False

   pars = ["-n", "1"]
   if area != None:
      pars.extend(["-a", str(area)])
   if test != None:
      pars.append("-t")

   runArgs = []
   files = [os.path.join(params.indir, f) for f in os.listdir(params.indir)]
   files.sort(key=os.path.getsize)
   files.reverse()
   for f in files:
      if fnmatch(f, "*.blif"):
         ipath = os.path.abspath(f)
         opath = os.path.abspath(params.outdir)+'/'+os.path.basename(f)
         runArgs.append([ipath, opath, pars, novpr, params.log])
         #doRun([ipath, opath, pars, novpr, log])
   #exit()
   if threads != None:
      pool = Pool(processes=threads)
   else:
      pool = Pool(4)
   errored = []
   total = len(runArgs)
   sys.stderr.write("Running, 0/"+str(total)+"\n")
   results.write("File\tPartitions\tChannel Width Base\tChannel Width TMR\tNumber of Inputs Base\tNumber of Inputs TMR\tNumber of Outputs Base\tNumber of Outputs TMR\tNumber of LUTs Base\tNumber of LUTs TMR\tNumber of Latches Base\tNumber of Latches TMR\tNetDelay Base (s)\tNetDelay TMR (s)\tLogicDelay Base (s)\tLogicDelay TMR (s)\tPeriod Base (ns)\tPeriod TMR (ns)\n")
   for i, res in enumerate(pool.imap_unordered(doRun, runArgs)):
      #results.write(str(res)+"\n\n")
      #name, numPartitions, channelWidth, type output, type output, type latch, type names
      if 'error' in res:
         sys.stderr.write(str(res)+"\n")
         errored.append(i)
      else:
         results.write(formatResults(res))
         results.flush()
         sys.stderr.write("Completed "+str(i+1)+"/"+str(total)+". Finished "+res["times"][0]+"\n")
   if errored.len() > 0:
      sys.stderr.write("Some errors occured. Retrying those files single threaded\n")
      for n in errored:
         res = doRun(runArgs[n])
         if 'error' in res:
            sys.stderr.write(str(res)+"\n")
            errored.append(i)
         else:
            results.write(formatResults(res))
            results.flush()
            sys.stderr.write("Completed "+str(i+1)+"/"+str(total)+". Finished "+res["times"][0]+"\n")
   log.close()
   results.close()
