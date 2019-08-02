#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 projFolder"
	exit -1
fi

cd $1
make all
NW=1
N=1048576

for((i=0;i<9;++i)); do # from 1 to 256 increasing by power of 2
    echo "executing with NW = $NW"
    FILENAME=benchmark/data/nw_${NW}.txt
    echo "============================================" >> $FILENAME > $FILENAME
    for((j=0;j<10;++j)); do # from 2^20 to 2^30 increasing by power of 2
        echo "N = $N"
        TMP=$((j+20))
        echo "N = $N (2^$TMP)" >> $FILENAME
        for((z=0;z<3;++z)); do
            ./sorter $N $NW 123 >> $FILENAME
        done
        N=$((N * 2))
        echo "============================================" >> $FILENAME
    done
    N=1048576
    NW=$((NW * 2))
done

echo "output writter to benchmark folder"
