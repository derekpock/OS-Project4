#!/bin/bash

for i in {1..10}
do	
    sbatch --time=10:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=1 --mem=4GB -- ./run.sh 1 1000000 1
    sbatch --time=5:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=1 --mem=4GB -- ./run.sh 1 500000 1
    sbatch --time=1:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=1 --mem=4GB -- ./run.sh 1 100000 1
    sbatch --time=1:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=1 --mem=4GB -- ./run.sh 1 10000 1
    sbatch --time=1:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=1 --mem=4GB -- ./run.sh 1 1000 1

    sbatch --time=5:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=2 --mem=4GB -- ./run.sh 2 1000000 1
    sbatch --time=2:30:00 --constraint=dwarves --nodes=1 --ntasks-per-node=2 --mem=4GB -- ./run.sh 2 500000 1
    sbatch --time=1:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=2 --mem=4GB -- ./run.sh 2 100000 1
    sbatch --time=1:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=2 --mem=4GB -- ./run.sh 2 10000 1
    sbatch --time=1:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=2 --mem=4GB -- ./run.sh 2 1000 1
    for j in 1000 10000 100000 500000 1000000
    do
        for k in 4 8 16 32
        do
            sbatch --time=1:00:00 --constraint=dwarves --nodes=1 --ntasks-per-node=$k --mem=4GB -- ./run.sh $k $j 1
        done
        sbatch --time=1:00:00 --constraint=dwarves --nodes=2 --ntasks-per-node=4 --mem=4GB -- ./run.sh 4 $j 2
        sbatch --time=1:00:00 --constraint=dwarves --nodes=4 --ntasks-per-node=4 --mem=4GB -- ./run.sh 4 $j 4
        sbatch --time=1:00:00 --constraint=dwarves --nodes=4 --ntasks-per-node=2 --mem=4GB -- ./run.sh 2 $j 4
        sbatch --time=1:00:00 --constraint=dwarves --nodes=2 --ntasks-per-node=16 --mem=4GB -- ./run.sh 16 $j 2
        sbatch --time=1:00:00 --constraint=dwarves --nodes=16 --ntasks-per-node=2 --mem=4GB -- ./run.sh 2 $j 16
    done
done
