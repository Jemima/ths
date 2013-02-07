import re
import sys
import os
import glob
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

if(len(sys.argv) < 5):
   print("Usage: blifJoin outfile headerfile infile1 infile2...\n       OR\n       blifJoin outfile headerfile -f searchpath")
   exit(1)

files = sys.argv[3:]
if(sys.argv[3] == "-f"):
   files = glob.glob(sys.argv[4])
signalsIn = set()
signalsOut = set()
subckts = ""
body = ""
counterName = 1
for path in files:
   file = open(path)
   buffer = ""
   line = 0
   name = ""
   inputs = ""
   outputs = ""
   prefix = "p"+str(counterName)

   line = readContinuedLine(file)
   m = re.search('\s*.model\s+(.+)', line)
   name = m.group(1)
   name = prefix+name
   
   line = readContinuedLine(file)
   m = re.search('\s*.inputs\s+(.+)', line)
   inputs = m.group(1).split(' ')
   signalsIn.update(inputs)
   
   line = readContinuedLine(file)
   m = re.search('\s*.outputs\s+(.+)', line)
   outputs = m.group(1).split(' ')
   signalsOut.update(outputs)




   subckts += "\n.subckt "+name
   for n in range(0,len(inputs)):
       subckts += " %s=%s" % (inputs[n], inputs[n])
   for n in range(0,len(outputs)):
      subckts += " %s=%s"%(outputs[n], outputs[n])

   file.seek(0)
   temp = file.read()
   temp = re.sub(r'.model\s+(\S+)', ".model "+prefix+r'\1', temp)
   temp = re.sub(r'.subckt\s+(\S+)', ".subckt "+prefix+r'\1', temp)
   body += temp+"\n\n"
   file.close()
   counterName+=1;

sys.stderr.write("Writing to "+sys.argv[1]+"\n")
file = open(sys.argv[1], "w")
header = open(sys.argv[2], "r")
file.write(header.read())
file.write("\n")
file.write(subckts)
file.write("\n.end\n\n")
file.write(body)
file.close()
header.close()