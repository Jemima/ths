import re
import sys


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

if(len(sys.argv) != 4):
   print("Usage: blifTMR voterfile infile outfile\n")
   exit(1)
   
#infile MUST contain a .model, .inputs, .outputs
#It is possible for no inputs to be specified i.e. ".inputs " however a model name and outputs MUST be provided.
#Currently clocks are ignored
try:
   voter  = open(sys.argv[1])
   insert = open(sys.argv[2])
   output = open(sys.argv[3], 'w')

   buffer = ""
   line = 0
   name = ""
   inputs = ""
   outputs = ""

   line = readContinuedLine(insert)
   m = re.search('\s*.model\s+(.+)', line)
   name = m.group(1)

   line = readContinuedLine(insert)
   m = re.search('\s*.inputs\s+(.+)', line)
   if m is None: #no inputs
      inputs = []
   else:
      inputs = m.group(1).split(' ')

   line = readContinuedLine(insert)
   m = re.search('\s*.outputs\s+(.+)', line)
   outputs = m.group(1).split(' ')


   output.write(".model output\n.inputs")
   for n in range(0,len(inputs)):
       output.write(" %s"%inputs[n])
   output.write("\n.outputs")
   for n in range(0,len(outputs)):
       output.write(" %s"%outputs[n])



   subckt = "\n.subckt "+name
   for n in range(0,len(inputs)):
       subckt += " %s=%s" % (inputs[n], inputs[n])
   for n in range(0,len(outputs)):
      subckt += " %s=[qq%d//replace]"%(outputs[n], n)

   output.write(subckt.replace("//replace", '0'))
   output.write(subckt.replace("//replace", '1'))
   output.write(subckt.replace("//replace", '2'))
         
         
   for n in range(0, len(outputs)):
      output.write("\n.subckt voter a=[qq%d0] b=[qq%d1] c=[qq%d2] out=%s"%(n, n, n, outputs[n]))
      #output.write("\n.names [%d0] [%d1] [%d2] out%d\n11- 1\n1-1 1\n-11 1\n\n"%(n, n, n, n))

   output.write("\n.end\n")
   output.write(voter.read())
   output.write("\n")
   insert.seek(0)
   output.write(insert.read())

   insert.close()
   output.close()
except:
   print "Unexpected error:", sys.exc_info()[0]
   print sys.argv[2]
   raise