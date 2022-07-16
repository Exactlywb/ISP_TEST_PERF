#! /usr/bin/bash 

cat tmp | while read line
do
    read -ra SPLITED <<< "$line"
    if [[ ${SPLITED[0]} == "cc1plus" ]]
    then
        if [[ ${SPLITED[1]} == "PERF_RECORD_MMAP2" ]] && [[ $(basename ${SPLITED[-1]}) == "cc1plus" ]]
        then
            range=$(echo ${SPLITED[3]} | tr -d '[)' | tr '(' ' ')
            echo "MMAP $range"
        fi
    fi
done
