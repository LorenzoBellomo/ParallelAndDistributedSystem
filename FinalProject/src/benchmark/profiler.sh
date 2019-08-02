#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 projFolder"
	exit -1
fi

cd $1
make profile

FILENAME=benchmark/data/profiler.out 

echo "executing program with 1 worker"
echo "3000000 items, 1 worker" > $FILENAME
echo "===============================================" >> $FILENAME
LD_PRELOAD=benchmark/gprof-helper.so ./sorter 3000000 1 123 >> $FILENAME
gprof sorter >> $FILENAME
echo "===============================================" >> $FILENAME

echo "executing program with 2 workers"
echo "3000000 items, 2 workers" >> $FILENAME
echo "===============================================" >> $FILENAME
LD_PRELOAD=benchmark/gprof-helper.so ./sorter 3000000 2 123 >> $FILENAME
gprof sorter >> $FILENAME
echo "===============================================" >> $FILENAME

echo "executing program with 8 workers"
echo "3000000 items, 8 workers" >> $FILENAME
echo "===============================================" >> $FILENAME
LD_PRELOAD=benchmark/gprof-helper.so ./sorter 3000000 8 123 >> $FILENAME
gprof sorter >> $FILENAME
echo "===============================================" >> $FILENAME

rm -f gmon.out
echo "output written to $FILENAME"

