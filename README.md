ths
===
The partitioner and workflow folders contain READMEs detailing how to get them built and working.

partitioner contains the C++ partitioner code.
workflow contains the assorted python scripts to handle everything else. This includes triplicating, joining, 
and helper scripts to automate running the workflow.

Specifically related to getting things working on Kraken, you will need to install a more recent version of Boost than
is available in the repos, 1.43 or later, and will need to modify the makefile in partitioner is detailed in the README.txt

workflow requires Python3 to run.
