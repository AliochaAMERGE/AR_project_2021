#!/bin/bash

mpicc $1 -lm -ldl
mpirun -np $2 --oversubscribe  ./a.out
rm a.out
