#!/usr/bin/env python3
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
import random
from multiprocessing import Pool
from fnmatch import fnmatch


def parseOutput(s):
   out = {}
   if "unrouteable" in s:
      return {'error': 'unrouteable'}
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
   try:
      ipath = args[0]
      opath = args[1]
      mainParams = args[2]
      novpr = args[3]
      logPath = args[4]
      channelWidth = args[5]
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
      seed = str(random.randint(0, 30000))
      path = "0.00000001"
      vprParams = ["../vpr", "--route_file", "file.route", "--place_file", "file.place", "--net_file", "file.net", "--full_stats", "--seed", seed]
      if channelWidth != None:
         vprParams.append("--route_chan_width")
         vprParams.append(str(channelWidth))

      vprParams.append("../arch.xml")
      if novpr == False:
         try:
            os.chdir(dir)
            vprParams.append(ipath)
            output2 = str(subprocess.check_output(vprParams, stderr=subprocess.STDOUT), 'UTF-8')
         except subprocess.CalledProcessError as e:
            log.write(str(e.output, 'UTF-8'))
            sys.stderr.write(str(e))
            os.chdir("..")
            pass
            return {"error": vprParams}
         reto2 = parseOutput(output2)
         if "error" in reto2:
            return reto2
         path = re.findall('Final critical path: (.+) ns', output2)[0]
         path = float(path)*1E-9*1.8
         par.append('-l')
         par.append(str(path))
         os.chdir("..")

      sys.stdout.flush()

      discard = open(os.devnull, 'w')
      try:
         ret = str(subprocess.check_output(par, stderr=log), 'UTF-8')
      except subprocess.CalledProcessError as e:
         if e.returncode == 203:
            return {"error": {"message": "Unable to partition for given parameters", "par": par}}
         elif e.returncode == 27:
            return {"error": {"message": "Failed equivalence validation", "par": par}}
         else:
            raise
      ret = ret.split("\n")
      res = ret[1:]
      for n in range(0, len(res)):
         res[n] = res[n].split("\t")
      ret = {"times": ret[0].split("\t"), "partitions": res, "out2": None, "out3": None}
      if novpr == False:
         try:
            os.chdir(dir)
            try:
               os.remove("file.net")
               os.remove("file.place")
               os.remove("file.route")
               os.remove("file.sdc")
            except:
               pass
            vprParams.pop()
            vprParams.append(opath)
            output3 = str(subprocess.check_output(vprParams, stderr=subprocess.STDOUT), 'UTF-8')
         except subprocess.CalledProcessError as e:
            try:
               #Try again. Sometimes VPR crashes just after packing, so use that packed netlist to place and route.
               vprParams.insert(0, "--place")
               vprParams.insert(0, "--route")
               output3 = str(subprocess.check_output(vprParams, stderr=subprocess.STDOUT), 'UTF-8')
            except:
               pass
            log.write(str(e.output, 'UTF-8'))
            sys.stderr.write(str(e))
            os.chdir("..")
            pass
            return {"error": vprParams}
         ret["out2"] = parseOutput(output2)
         ret["out3"] = parseOutput(output3)
         os.chdir("..")
      sys.stdout.flush()

      shutil.rmtree(dir)
      log.write(str(ret))
      log.write("\n")
      log.close()
      return ret
   except Exception as e:
      print(e)
      import traceback
      traceback.print_exc()
      return {"error": e}

def formatResults(res):
   times = res['times']
   partitions = res['partitions']
   base = res['out2']
   tmr = res['out3']
   fields = ['channelWidth', 'type input', 'type output', 'type names', 'type latch', 'Duration', 'NetDelay', 'LogicDelay', 'Period']
   sRet = "{0}\t{1}\t{2}\t{3}\t{4}\t".format(times[0], times[1], times[2], times[3], times[9].strip())
   if base != None and tmr != None:
      if 'error' in base:
         sRet += str(base)
      elif 'error' in tmr:
         sRet += str(tmr)
      else:
         for field in fields:
            sRet += "{0}\t{1}\t".format(base[field], tmr[field])
   else:
      sys.stderr.write("Base: ")
      sys.stderr.write(str(base))
      sys.stderr.write("TMR: ")
      sys.stderr.write(str(tmr))

   sRet +="\n"
   for line in partitions:
      for value in line:
         sRet += "\t"+value
      sRet += "\n"
   return sRet+"\n"


if __name__ == "__main__":
   start = time.clock()
   parser = argparse.ArgumentParser(description="TMRs a whole bunch of circuits, then runs them through VPR, and collects results")
   parser.add_argument("indir", type=str, help="Input directory containing blif files")
   parser.add_argument("outdir", type=str, help="Output directory")
   parser.add_argument("-t", "--test", action="store_true", help="Test generated files")
   parser.add_argument("-R", "--results", type=str, help="Location of results file", required=True)
   parser.add_argument("-l", "--log", type=str, help="Location of log file")
   parser.add_argument("-v", "--novpr", action="store_true", help="Skip running VPR")
   parser.add_argument("-r", "--recoverytime", type=float, help="Max recovery time")
   parser.add_argument("-n", "--numthreads", type=int, help="Maximum number of threads to use. Defaults to 4")
   parser.add_argument("-w", "--channelwidth", type=int, help="Channel width for VPR to use")

   params = parser.parse_args()
   results = open(params.results, 'w')
   if params.test:
      test = "-t"
   else:
      test = ""
   threads = params.numthreads
   recoverytime = params.recoverytime
   test = params.test
   novpr = params.novpr
   if novpr == None:
      novpr = False

   pars = ["-n", "1"]
   if recoverytime != None:
      pars.extend(["-r", str(recoverytime)])
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
         runArgs.append([ipath, opath, pars, novpr, params.log, params.channelwidth])
         #doRun([ipath, opath, pars, novpr, params.log, params.channelwidth])
   #exit()
   if threads != None:
      pool = Pool(processes=threads)
   else:
      pool = Pool(4)
   errored = []
   total = len(runArgs)
   sys.stderr.write("Running, 0/"+str(total)+"\n")
   results.write("File\tUnused\tNumber of Nodes\tEstimated Latency\tPartitions\tChannel Width Base\tChannel Width TMR\tNumber of Inputs Base\tNumber of Inputs TMR\tNumber of Outputs Base\tNumber of Outputs TMR\tNumber of LUTs Base\tNumber of LUTs TMR\tNumber of Latches Base\tNumber of Latches TMR\tVPR Duration Base\tVPR Duration TMR\tNetDelay Base (s)\tNetDelay TMR (s)\tLogicDelay Base (s)\tLogicDelay TMR (s)\tPeriod Base (ns)\tPeriod TMR (ns)\n")
   results.write("Per partition values\tRecovery Time\tNumber of Outputs\tNumber of Inputs\tNumber of cut loops\tNumber of latches\tNumber of LUTs\tCritical Path Length\n")
   for i, res in enumerate(pool.imap(doRun, runArgs)):
      #results.write(str(res)+"\n\n")
      #name, numPartitions, channelWidth, type output, type output, type latch, type names
      if 'error' in res:
         sys.stderr.write(str(res)+"\n")
         results.write(str(res)+"\n")
         errored.append(i)
      else:
         results.write(formatResults(res))
         results.flush()
         sys.stderr.write("Completed "+str(i+1)+"/"+str(total)+". Finished "+res["times"][0]+"\n")
   #if len(errored) > 0:
      #sys.stderr.write("Some errors occured. Retrying those files single threaded\n")
      #for n in errored:
         #res = doRun(runArgs[n])
         #if 'error' in res:
            #sys.stderr.write(str(res)+"\n")
         #else:
            #results.write(formatResults(res))
            #results.flush()
            #sys.stderr.write("Completed "+str(i+1)+"/"+str(total)+". Finished "+res["times"][0]+"\n")
   sys.stderr.write("\n")
   print("\n")
   results.close()
