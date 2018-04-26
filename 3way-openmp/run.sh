#!/bin/bash
## Provide 2 arguments: NUM_OF_THREADS and MAX_LINES_TO_SAMPLE

/usr/bin/time -f "%P\t%MKb\t%E" sleep 2 ./openmp --threads=$1 --lines=$2
