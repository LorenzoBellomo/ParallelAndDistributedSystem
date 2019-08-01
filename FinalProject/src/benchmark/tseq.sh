#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 projFolder"
	exit -1
fi

cd $1
make tseq

FILENAME=benchmark/dump/tseq.out 

./sorter 300000 1 123 > $FILENAME

echo "output written to $FILENAME"