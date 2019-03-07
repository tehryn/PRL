#!/usr/bin/env bash
if [ $# -ne 1 ]
then
    echo -e "Usage: bash test.sh N"
else
    re='^[0-9]+$'
    if ! [[ $1 =~ $re ]] ; then
       echo "error: Not a number" >&2; exit 1
    fi
    
    dd if=/dev/random bs=1 count=$1 of=numbers 1>&2 2>/dev/null
    mpic++ --prefix /usr/local/share/OpenMPI -o bks bks.cpp
    log=$(python3 -c  "import math; log=math.ceil( math.log( $1, 2 ) ); print( 1 << ( log - 1 ).bit_length() )")
    procNum=$(( 2 * $log - 1 ))
    #echo "Log2($1) = $log"
    #echo "ProcNum  = $procNum"
    
    mpirun --prefix /usr/local/share/OpenMPI -np $procNum bks numbers
    rm -f bks numbers
fi
