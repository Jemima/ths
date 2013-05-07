Algorithm
=========

 - Read blif file into memory as DFG (Directed Flow Graph)
 - Open a new partition
 - Traverse graph visiting each node exactly once.
 - If partition+current node exceeds limits finalise partition, open new partition
   - Cut cycles
   - Write to file
 - Add node to current partition
 - Finalise final partition
 - Triplicate each partition
 - Add voter to each partition
 - Rejoin loops
 - Connect TMR partitions together

Points -> pseudocode/flowchart.
Learn to pseudocode. Find good example and copy.
Number lines, then explain what each line is doing
Describe data structures used in code e.g. Model is object containing set of nodes, map of signal name->Signal, etc.
Signal contains ...
Node contains ...

         BEGIN
            files <- _Partition(file)_
            FOREACH(file in files)
               file <- _Triplicate(file)
            model <- _Join(files)_
            model <- _Flatten(model)_
         END

         "Read the contents of a file as a string, create three copies of the model in the file,
          then insert a voter to join the three models together,
          and connect them to the primary outputs"
         BEGIN Triplicate(file)
            voter = _ReadVoterFromFile()_
            partition = _ReadModelFromFile()_
            model = voter
            model.AddSubCircuit(partition)
            model.AddSubCircuit(partition)
            model.AddSubCircuit(partition)
            model.ConnectSignals()
         END Triplicate

         "Given a list of blif files, adds each of them as a submodel of a master which
          just joins them all together"
         BEGIN Join(files)
            model = EmptyModel
            FOREACH(file in files)
               model.AddSubCircuit(file)
            model.ConnectSignals()
         END Join

         BEGIN Partition(file)
            model = _ReadModel(file)_
            partition = _CreateEmptyPartition()_
            queue = CreateEmptyQueue()
            visited = 
            FOREACH(signal in output)
               queue.push_back(signal->source)

            WHILE(queue.size() > 0)
               node = _PopFront(queue)_
               IF(visited[node] == true)
                  continue
               visited[node] = true
               IF(_RecoveryTime(partition, node)_ > UserSpecifiedRecoveryTime)
                  _TMR(partition)_
                  partition = _CreateEmptyPartition()_
               FOREACH(signal in node->inputs)
                  queue.push_back(signal->source)

            _TMR(partition)_
         END Partion
         queue enqueue/dequeue
         Consistent syntax
         Aftereach block explain the lines

         BEGIN RecoveryTime(partition, node)
            return RecoveryTimeEquationHere
         END RecoveryTime

         BEGIN PopFront(q)
            item = q.front()
            q.remove_front()
            return item
         END PopFront

         BEGIN TMR(model)
            file = EmptyFile()
            _CutLoops(model)_
            WriteToFile(file, model)
         END TMR

         BEGIN CutLoops(model)
            state = EmptyMap()
            FOREACH(signal in model->outputs)
               CutLoopsRecursive(state, NULL, signal)
         END CutLoops

         BEGIN CutLoopsRecursive(state, parent, signal)
            node = signal->source
            IF(state[node] == Exploring)
               Replace(parent->inputs, signal->name, "qqrin"+signal->name)
            ELSE IF(state[node] == Finished)
               return
            ELSE
               state[node] = Exploring
               FOREACH(signal in node->inputs)
                  CutLoopsRecursive(state, node, signal)

            state[node] = Finished
         END CutLoopsRecursive

Expand Error Recovery Time, CutLoops, etc. Basic operations.

Fix error recovery time calculation in partitioner. Take into account real world architecture (reconfiguration time, etc, etc).

Communication delay. Requires estimate, partition, repartition with better estimate until correct.

Number feedback/feedforward edges.

Update by Friday.
Separate report on self cycle vs general internal cycle.


High level description interleaved with pseudocode.

A blif file is provided to our partitioner, which is read into memory and represented as a DFG (Directed Flow Graph), with each circuit element (latch, etc) making a node.

A new partition is then created, which is an empty circuit with no elements.

The DFG is then traversed in a breadth first manner from the outputs to the inputs, keeping track of whether each node has been visited before. If it has, the node is skipped ensuring that each node is visited once and only once.

As each node is reached the error recovery time of the open partition+node is tested. If it exceeds the user specified limit the partition is closed off and written out to a file, and a new empty partition is opened. The error recovery time is calculated as DetectionTime+ResynchTime+NetworkTime+ReconfigureTime=...?

The node is then added to the current partition and the process repeats until all nodes have been added to a partition.

As each partition is finalised cycles are detected using a depth first search variant based on using the at http://www.personal.kent.edu/~rmuhamma/Algorithms/MyAlgorithms/GraphAlgor/depthSearch.htm).
Starting from each output and recursively descending through the graph, a node is marked as exploring when it is first reached, then all its children are expanded, then after its children are finished, a node is marked as finished.
While executing, if a node marked as finished is reached we can return from that path early, as we have already determined there is no loop beyond that point.
If we reached a node marked as exploring, then we've found a loop and the most recent edge is cut with the edge source being promoted to a primary input, and sink being promoted to a primary output. These special primary outputs and inputs are 'marked' for a later step.

Each finalised partition is now in a separate file. Each is now treated as a black box as it gets triplicated and has voter logic inserted on all the outputs.

After voter insertion any cut cycles are then reconnected outside the voters.

And lastly, each set of triplicated partitions and voters is connected together in one large heirarchical blif file, and is then flattened using the program abc into a format VPR can read.
