#!/bin/bash
## Provide 3 arguments: NUM_OF_THREADS and MAX_LINES_TO_SAMPLE and NUM_OF_NODES

/usr/bin/time -f "%P\t%M Kb\t%e sec\t$3 nodes\t$1 cores\t$2 lines\tdwarves" -- ./openmp --threads=$(( $1 * $3 )) --lines=$2 -v
