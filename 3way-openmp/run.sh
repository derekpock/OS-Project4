#!/bin/bash
## Provide 2 arguments: NUM_OF_THREADS and MAX_LINES_TO_SAMPLE

time ./openmp --threads=$1 --lines=$2
