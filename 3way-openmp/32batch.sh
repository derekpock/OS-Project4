#!/bin/bash

for i in {1..10}
do
    for j in 1000 10000 100000 500000 1000000
    do
        sbatch --time=4:00:00 --nodes=1 --ntasks-per-node=32 --mem=4GB -- ./run.sh 32 $j 1
    done
done
