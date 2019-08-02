#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 projFolder"
	exit -1
fi

cd $1
make tseq

FILENAME=benchmark/data/tseq.out 

echo "executing 3000000 items, 1 worker"
echo "3000000 items, 1 worker" > $FILENAME
echo "===============================================" >> $FILENAME
./sorter 3000000 1 123 >> $FILENAME
echo "===============================================" >> $FILENAME

echo "executing 3000000 items, 2 workers"
echo "3000000 items, 2 workers" >> $FILENAME
echo "===============================================" >> $FILENAME
./sorter 3000000 2 123 >> $FILENAME
echo "===============================================" >> $FILENAME

echo "executing 3000000 items, 8 workers"
echo "3000000 items, 8 workers" >> $FILENAME
echo "===============================================" >> $FILENAME
./sorter 3000000 8 123 >> $FILENAME
echo "===============================================" >> $FILENAME

echo "executing 3000000 items, 256 workers"
echo "3000000 items, 256 workers" >> $FILENAME
echo "===============================================" >> $FILENAME
./sorter 3000000 256 123 >> $FILENAME

echo "output written to $FILENAME"