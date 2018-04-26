#!/bin/bash

for i in {1..10}
do
    for j in 1000 10000 100000 500000 1000000
    do
        for k in 1 2 4 8 16 32
        do
            ./schedule.sh $k $j 1
        done
        ./schedule.sh 4 $j 2
        ./schedule.sh 4 $j 4
        ./schedule.sh 2 $j 4
        ./schedule.sh 16 $j 2
        ./schedule.sh 2 $j 16
    done
done