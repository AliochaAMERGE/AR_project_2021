#!/bin/bash

mpicc $1
mpirun -np $2 --oversubscribe  ./a.out
rm a.out
