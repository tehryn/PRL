RED='\033[0;31m'
GREEN='\033[0;32m'
YEL='\033[0;33m'
NC='\033[0m'
total=0
ok=0
mpic++ --prefix /usr/local/share/OpenMPI -o bks bks.cpp
for file in tests/*.in
do
    name=$(basename "${file}")
    num=${name%.in}
    echo $num
    log=$(python3 -c  "import math; log=math.ceil( math.log( $num, 2 ) ); print( 1 << ( log - 1 ).bit_length() )")
    procNum=$(( 2 * $log - 1 ))
    mpirun --prefix /usr/local/share/OpenMPI -np $procNum bks $file >"tests/${num}.bks"
    diff "tests/${num}.bks" "tests/${num}.out"
    if [ "$?" == "0" ]
    then
        echo -e "${GREEN}[TEST $total] mpirun --prefix /usr/local/share/OpenMPI -np $procNum bks $file ${NC}"
        ((ok++))
    else
        echo -e "${RED}[TEST $total] mpirun --prefix /usr/local/share/OpenMPI -np $procNum bks $file ${NC}"
    fi
    ((total++))
    sleep 5
done

if [ "$ok" == "$total" ]
  then
    echo -e "[${GREEN}SUCCESSFULL${NC}]"
   else
    echo -e "[${RED}FAILED${NC}]"
fi