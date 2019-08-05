#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 projFolder"
	exit -1
fi

cd $1
mkdir -p benchmark/data
make benchmark
NW=1
N=1048576

for((i=0;i<9;++i)); do # from 1 to 256 increasing by power of 2
    echo "executing with NW = $NW"
    FILENAME=benchmark/data/nw_${NW}.out
    echo "============================================" > $FILENAME
    for((j=0;j<7;++j)); do # from 2^20 to 2^25 increasing by power of 2
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

N=1048576
FILENAME=benchmark/data/std_sort.out
echo "computing some std::sort times now for the used input sizes"
make all
echo "" > $FILENAME
for((j=0;j<7;++j)); do # from 2^20 to 2^25 increasing by power of 2
    echo "============================================" >> $FILENAME
    echo "N = $N"
    TMP=$((j+20))
    echo "N = $N (2^$TMP)" >> $FILENAME
    for((z=0;z<3;++z)); do
        ./sorter $N 64 123 >> $FILENAME
    done
    N=$((N * 2))
    echo "============================================" >> $FILENAME
done

echo "output writter to benchmark folder"
