import re
import sys

def readContinuedLine(f):
    buffer = ""
    while(True):
        buffer = buffer + f.readline()
       # print("%s, %s"%(buffer, buffer[-2]))
        if buffer[-2] == '\\':
            buffer = buffer[:-2]
        else:
            break
    return buffer

voter  = open('voter.blif')
insert = open(sys.argv[1])
output = open(sys.argv[2], 'w')

buffer = ""
line = 0
name = ""
inputs = ""
outputs = ""

line = readContinuedLine(insert)
print(line)
m = re.search('\s*.model\s+(.+)', line)
name = m.group(1)

line = readContinuedLine(insert)
print(line)
m = re.search('\s*.inputs\s+(.+)', line)
inputs = m.group(1).split(' ')

line = readContinuedLine(insert)
print(line)
m = re.search('\s*.outputs\s+(.+)', line)
outputs = m.group(1).split(' ')

print(name, inputs, outputs)

output.write(".model output\n.inputs")
for n in range(0,len(inputs)):
    output.write(" %s"%inputs[n])
output.write("\n.outputs")
for n in range(0,len(outputs)):
    output.write(" %s"%outputs[n])



subckt = "\n.subckt "+name
for n in range(0,len(inputs)):
    subckt += " %s=%s" % (inputs[n], inputs[n])
print(n)
for n in range(0,len(outputs)):
   subckt += " %s=[%d//replace]"%(outputs[n], n)

output.write(subckt.replace("//replace", '0'))
output.write(subckt.replace("//replace", '1'))
output.write(subckt.replace("//replace", '2'))
      
      
for n in range(0, len(outputs)):
   output.write("\n.subckt voter a=[%d0] b=[%d1] c=[%d2] out=%s"%(n, n, n, outputs[n]))
   #output.write("\n.names [%d0] [%d1] [%d2] out%d\n11- 1\n1-1 1\n-11 1\n\n"%(n, n, n, n))

output.write("\n.end\n")
output.write(voter.read())
output.write("\n")
insert.seek(0)
output.write(insert.read())

insert.close()
output.close()