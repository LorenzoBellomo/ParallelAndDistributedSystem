#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 projFolder"
	exit -1
fi

cd $1
make tseq

FILENAME=benchmark/data/tseq.out 
NW=1
echo "" > $FILENAME

for((i=0;i<9;++i)); do # from 1 to 256 increasing by power of 2
    echo "executing with NW = $NW"
	echo "3000000 items, $NW workers" >> $FILENAME
	echo "===============================================" >> $FILENAME
	for((z=0;z<5;++z)); do
		./sorter 3000000 $NW 123 >> $FILENAME
	done
	echo "===============================================" >> $FILENAME
done

echo "output written to $FILENAME"