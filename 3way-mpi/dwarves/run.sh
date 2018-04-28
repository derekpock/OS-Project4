#!/bin/bash
## Provide 2 arguments: NUM_OF_THREADS and MAX_LINES_TO_SAMPLE

/usr/bin/time -f "%P\t%M Kb\t%e sec\t$3 nodes\t$1 cores\t$2 lines\tdwarves" -- mpirun ./openmp --lines=$2 -v
