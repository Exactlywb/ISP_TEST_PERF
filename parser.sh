#! /usr/bin/bash 

cat $1 | while read line
#perf script -F comm,event,brstack --show-mmap-events | while read line
do
    read -ra SPLITED <<< "$line"
    if [[ ${SPLITED[0]} == "cc1plus" ]]
    then
        if [[ ${SPLITED[1]} == "PERF_RECORD_MMAP2" ]] && [[ $(basename ${SPLITED[-1]}) == "cc1plus" ]]
        then
            range=$(echo ${SPLITED[3]} | tr -d '[)' | tr '(' ' ')
            echo "MMAP $range ${SPLITED[5]}"
        fi
        if [[ ${SPLITED[1]} == "iTLB-load-misses:" ]]
        then
            len=${#SPLITED[@]}
            for (( j=2; j < $len; j++));
            do
                IFS='/' read -ra TRACE <<< ${SPLITED[$j]}
                echo "${TRACE[0]} ${TRACE[1]}"
            done
        fi
    fi
done
