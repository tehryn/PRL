#!/usr/bin/env bash
if [ $# -ne 1 ] ; then
    echo -e "Usage: bash test.sh N"
    echo -e "       Where N > 0"
    exit 1
else
    if [ $1 -eq 0 ] ; then
        echo -e "Usage: bash test.sh N"
        echo -e "       Where N > 0"
        exit 2
    else
        re='^[0-9]+$'
        if ! [[ $1 =~ $re ]] ; then
           echo -e "error: Not a number" >&2; exit 1
           exit 2
        else   
            # generovani nahodneho souboru
            dd if=/dev/random bs=1 count=$1 of=numbers 1>&2 2>/dev/null
            # preklad
            mpic++ --prefix /usr/local/share/OpenMPI -o bks bks.cpp
            
            #vypocet poctu procesu
            log=$(python3 -c  "import math; log=math.ceil( math.log( $1, 2 ) ); print( 1 << ( log - 1 ).bit_length() if log > 1 else 1 )")
            procNum=$(( 2 * $log - 1 ))
         
            #spusteni
            mpirun --prefix /usr/local/share/OpenMPI -np $procNum bks numbers
            
            #uklid
            rm -f bks numbers
       fi
    fi
fi
