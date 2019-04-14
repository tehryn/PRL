#!/usr/bin/env bash
if [ $# -ne 1 ] ; then
    echo -e "Usage: bash test.sh SEQUENCE"
    exit 1
else
    # preklad
    mpic++ --prefix /usr/local/share/OpenMPI -o vuv vuv.cpp

    procNum=$(python3 -c  "import math; print( ( 1 << ( ${#1} ).bit_length() ) * 2 - 4 if ${#1} > 1 else 1 )")

    #spusteni
    mpirun --prefix /usr/local/share/OpenMPI -np $procNum vuv $1

    #uklid
    rm -f vuv
fi