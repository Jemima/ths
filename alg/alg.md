Data Structures:
==
General structure:
Blif represents a BLIF file, contains 1+ Models
Model represents a BLIF model, or circuit, as a DFG.
Each node is a BlifNode.
Each BlifNode has the names of its inputs and outputs stored as strings.
The model contains a map from string->Signal.
Each Signal has a pointer to source and sinks.

signals[node->output]->sinks are the children of a node
FOREACH(signal) in inputs
   signals[signal]->source
provides the ancestors of each node.
A Model also contains the primary input and output signals.

Storing signals as names, and providing a circuit specific map allows for nodes
to be copied directly across with redirecting pointers at each step.
Additionally, it means cutting loops is a simple case of string replacement,
as the list of signals is dynamically generated from the set of nodes.

Model: Represents a BLIF Model i.e. a circuit or subcircuit within a BLIF file
as a DFG.
Fields:
   set(BlifNode) nodes - The set of all circuit elements
   map(string->Signal) - a map from signal name to signal
   string name - The name of this model
   list(Signal) inputs - The list of primary inputs for this circuit
   list(Signal) outputs - The list of primary outputs for this circuit

Methods:
   CutLoops() - described at ...
   AddNode(BlifNode) - adds a BlifNode to the current circuit. Doesn't create
   necessary signals.
   MakeSignalList() - Creates all appropriate signals
   MakeIOList() - Promotes appropriate signals to primary innputs or outputs

BlifNode: Represents a circuit element, or node within the DFG representing
our circuit
   List(string) inputs - names of input signals
   string output - name of output signal
   string clock - name of clock signal (if applicable)
   string type - ".latch" or ".names" for latch or LUT
   string contents - body of node element. Type dependent text data.
   unsigned long id - Unique id
   unsigned cost - 0 for LUT, 1 for latch. Number of clock cycles added to
   critical path by this node.
Methods:
   MakeNode(string type, list(string) params) - Creates a BlifNode from the
   text data in a BLIF file
   AddContents(string line) - Adds the provided line to the node body
   GetText() - Returns a textual representation for the node, suitable to be
   inserted into a BLIF file
   Clone() - Creates a clone of this node






Signal: Represents a signal between nodes, or a set of edges with common
source from a node in the DFG.
Fields:
   string name - name of the signal
   BlifNode source - node which drives this signal
   List(BlifNode) sinks - nodes which this signal drives

Blif: Represents a BLIF file, providing helper methods to read a file into a
model.
Fields:
   Map(string->Model) models - Map from model name to model
   List(string) masterInputs - Primary inputs for the master circuit
   List(string) masterOutputs - Primary inputs for the master circuit
   Model main - Master circuit model
Methods:
   Blif(string path) - Constructor, create Blif object from path to a BLIF
   file
   Write(string path, Model model) - Writes the specified model to a BLIF file






Algorithm:
==
Syntax
===
variable/object - begins with lower case
Function/Method/Procedure - begins with upper case
Class.Method - Static method
class->Method - instance method

      BEGIN main(input, targetRecoveryTime)
         Returns:
            Blif file containing transformed input file
         Variables:
            input - input blif file
            targetRecoveryTime - Maximum recovery time, in seconds, per partition
            output - output blif file
            files - set of paths to blif files representing one partition per file
            file - intermediate blif file
            header - string containing header (name, inputs, outputs) of a blif file

         files <- Partition(input)
         FOREACH(file in files)
            file <- Triplicate(file)
         header <- input->GetLines(3) //Read the first three lines (name, inputs, outputs)
         file <- Join(files, header)
         output <- Flatten(output)
      END main

We're given a blif file as input.
In line 11 we partition the input circuit into a number of sub circuits, each in a separate file.
Then in lines 12-13 we iterate over all the partitions, and transform them into a triplicated partition with three copies and a voter circuit.
Then in line 14 we extract the original header, which provides the name, inputs and outputs of the original circuit.
We then, in line 15, join all the partitions together with the original name, inputs and outputs (in the same order), as the original circuit.

      BEGIN Partition(file, targetRecoveryTime)
         Returns:
            Set of files containing partitions
         Variables:
            file - path to input blif file
            targetRecoveryTime - time (in seconds) which is the maximum error recovery time for a partition
            blif - In-memory representation of a Blif file
            circuit - BlifModel representing a circuit
            partition - BlifModel representing the subcircuit which is our current partition
            queue - FIFO queue
            visited - map of whether a node has been visited
            signal - Signal
            node - BlifNode
            numPartitions - counter for the number of partitions
            outPath - string representing output path
            files - set of files containing partitions

         blif <- new Blif(file) //Read in our blif file
         circuit <- blif->main //We only support non-heirarchical files, so this is the only circuit i.e. BlifModel
         partition <- new BlifModel() //Empty circuit/partition/model
         queue <- new Queue() //Empty queue
         visited <- Map(BlifNode->bool, DEFAULT: false) //Empty map, returns false if value not contained

         FOREACH(signal in circuit->outputs)
            queue->Enqueue(signal->source)

         WHILE(queue->size > 0)
            node = queue->Dequeue();
            IF(visited[node] == true)
               continue
            visited[node] = true
            IF(RecoveryTime(partition, node) > targetRecoveryTime)
               partition->CutLoops()
               files <- files + partition->WriteToFile()
               numPartitions <- numPartitions+1
               partition <- new BlifModel()
            FOREACH(signal in node->inputs)
               queue.Enqueue(signal->source)

         IF(partition->Size() > 0)
            partition->CutLoops()
            files <- files+partition->WriteToFile()

         return files
      END Partition
Iterate through the nodes in a circuit, adding them to partitions such that
each partition is under a specified recovery time.
Lines 16-20 create initial data structures, including our representation of the circuit as a DFG.
Lines 22-23 enqueue the initial nodes for the queue
26-29 pops the front node off the queue, and tests whether it has been visited, or added to a partition before. If it has, skip it and move on to the next, otherwise mark it as visited and proceed.
Line 30 tests if the current partition should be finished and written to file. If it should, loops are cut, it's written to file, and a new empty circuit is created to continue wtih.
Afterwards, the node is added to the current partition, and its inputs are pushed to the queue.
Finally, if we're left with a partially full partition at the end, write it out.

      BEGIN RecoveryTime(partition, node)
         Returns:
            Calculated recovery time for a partition if a node were added.

         Variables:
            partition - a circuit
            node - BlifNode to add to circuit and calculate time for

         return function which is not currently specified
      END RecoveryTime
Fill this in once RecoveryTime calculation is implemented.

ResynchTime+CommunicationTime+DetectionTime+ReconfigureTime
Function of circuit area, number of partitions, latency in clock cycles, and clock frequency.

      BEGIN BlifModel->CutLoops()
         Variables:
            state - Map from node to whether it's explored or not
            signal - Signal
            this - this BlifModel instance
         
         state <- Map(BlifNode->int, DEFAULT: 0)
         FOREACH(signal in this->outputs)
            this->CutLoopsRecursive(state, NULL, signal)
      END BlifModel->CutLoops
Start recursing from outputs back to detect loops.
Line 8 starts the recursive traversal for each output, with no parent.

      BEGIN BlifModel->CutLoopsRecursive(state, parent, signal)
         Variables:
            state - map of whether a node is explored or not
            parent - parent node
            signal - edge into this node
            node - BlifNode
            sig - Signal
         Constants:
            NEW - 0
            EXPLORING - 1
            FINISHED - 2
         node = signal->source
         IF(state[node] == EXPLORING) //Found a cycle
            ReplaceSignalName(parent->inputs, signal->name, "qqrin"+signal->name)
         ELSEIF(state[node] == FINISHED) //Already explored this path
            return;
         ELSE
            state[node] = EXPLORING
            FOREACH(sig in node->inputs)
               this->CutLoopsRecursive(state, node, signal)

         state[node] = FINISHED
      END BlifModel->CutLoopsRecursive
Line 12 tests if we've detected a cycle, through coming across a node marked as EXPLORING, since that means it's an ancestor of this node.
If so, then in line 13 we rename the input signal to cut the loop. As the input has no source, it automatically gets promoted to primary input.
If it's not a cycle, then line 14 tests if we've already checked for loops down this path. Since if we have it's a waste of time exploring it again, so we just quit early out of this branch.
next, in like 17-19 we mark our current node as EXPLORING, and foreach input, recursively expand each.
Then finally, in line 21, after we've explored all child branches, we can mark this node as FINISHED and return back up the stack.
Eventually, every node will be marked as FINISHED, and then we have detected and cut all loops in this circuit.

      BEGIN ReplaceSignalName(signals, name, newName)
         Variables:
            signals - list of signal names
            name - string to search for
            newName - string to replace name with
            signal - Signal

         FOREACH(signal in signals)
            IF(signal->name == name)
               signal->name = newName
      END ReplaceSignalName
String replace, given a list of strings, a string to match, and a string to replace it with.

      BEGIN Triplicate(file)
         Variables:
            voter - string containing blif voter circuit
            file - string containing path to input file
            circuit - string containing blif circuit
            output - output circuit
            subckt - string representing subcircuit text for blif file
            inputs - array of input names
            outputs - array of output names
            name - string containing circuit name
            str - temporary string variable
            n - temporary integer counter

         voter <- "voter.blif"->GetContents()
         name <- ParseLine(file->GetNextLine())
         inputs <- ParseLine(file->GetNextLine())
         outputs <- ParseLine(file->GetNextLine())
         file->seek(0)
         circuit <- file->GetContents()
         output->write(".model output\n")
         output->write(".inputs "+inputs+"\n")
         output->write(".outputs "+outputs+"\n")

         subckt <- "\n.subckt "+name
         FOREACH(str in inputs)
             subckt <- subckt + " " + str + "=" + str
         n <- 0
         FOREACH(str in outputs)
            subckt <- subckt + " " + str + "=" + "qq" + n + "//replace"
            n <- n+1
         subckt <- subckt->replace("=qqrin", "=")

         output->write(subckt->replace("//replace", '0'))
         output->write(subckt->replace("//replace", '1'))
         output->write(subckt->replace("//replace", '2'))

         n <- 0
         FOREACH(str in outputs)
            output->write("\n.subckt voter a=qq"+n+"0 b=qq"+n+"1 c=qq"+n+"2
            out="str)
            n <- n+1

         output->write(voter)
         output->write(circuit)
         return output
      END Triplicate
file->GetNextLine reads the next line from the file. Note that lines may be
continued according to the blif specification by ending the line with a '\'
file->GetContents returns the entire contents of the file.
This probably needs to be tidied up. Sort out exactly how much detail to go
into. I suspect this would be better represented graphically.
Basically, given a voter and a circuit, embed both as subcircuits into a new
heirarchical blif file, create three instances of the circuit and one of the
voter, and connect each set of three corresponding outputs to one voter, to
the single corresponding overall output.

      BEGIN Join(files, header)
         Variables:
            
         output->write(header)
         tail <- ""
         count <- 0
         FOREACH(file in files)
            name <- ParseLine(file->GetNextLine())
            inputs <- ParseLine(file->GetNextLine())
            outputs <- ParseLine(file->GetNextLine())
            prefix <- "p"+count
            file->seek(0)
            circuit <- file->GetContents()
            subckt <- ".subckt "+prefix+name
            FOREACH(str in inputs)
               subckt <- subckt + str + "=" + str
            FOREACH(str in outputs)
               subckt <- subckt + str + "=" + str
            circuit->replace(".model ", ".model "+prefix)
            circuit->replace(".subckt ", ".subckt "+prefix)
            tail <- tail+circuit
            output->write(subckt)
            count <- count+1

         output->write(tail)

         return output
      END Join
Given a list of blif files, concatenates them all together, creates
subcircuit definitions to connect them all together, and writes them to a file

      BEGIN Flatten(file)
         Flattening is currently performed by abc (link), called with parameters:
         ./abc -o output -c echo file
         Due to bug in abc, clock information is stripped from latches, so we then
         called grep and sed to fix the output file
         latch = split(grep -m 1 '\.latch' file)
         IF(latch)
            sed -ri 's/\.latch.+)(2)/\1 '+latch[3] + ' ' + latch[4] + ' 2/' output
      END Flatten(file)
./abc is provided an input file, given the command to echo the current file,
and told to output everything to output
grep is called to search for latches, and return the latch information if
there is one. If there is, replace the faulty latch information with the
correct information.
This assumes that there is only one global clock, all latches are triggered on
the same signal (e.g. rising edge, falling etc), and all latches have initial
state don't care, which holds true for all provided benchmarks.