#!/bin/bash

sbatch --time=1:00:00 --nodes=1 --ntasks-per-node=$1 --mem=4GB ./run.sh $1 $2
