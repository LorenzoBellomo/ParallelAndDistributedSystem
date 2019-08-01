#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 projFolder"
	exit -1
fi

cd $1
make profile

FILENAME=benchmark/dump/profiler.out 

LD_PRELOAD=benchmark/gprof-helper.so ./sorter 300000 1 123
gprof sorter > $FILENAME
rm -f gmon.out

