#!/bin/bash

## First found this … http://blog.oddbit.com/2014/08/11/four-ways-to-connect-a-docker/
## then found https://docs.docker.com/v1.7/articles/networking/

if [ -z "$1" ]
  then
    echo "No number supplied"
    exit 1
fi

if [ -z "$2" ]
  then
    echo "No time supplied"
    exit 1
fi

NUMBER=$1
NUMBERL=20
TIME=$2

COUNTER=0
while [  $COUNTER -lt $NUMBERL ]; do
	
	echo ""
	echo "python main.py -n $NUMBER -t $TIME --timeout 800 -o full"
	echo ""
	python main.py -n $NUMBER -t $TIME --timeout 800 -o full
	DATENOW=$(date +"%y_%m_%d_%H_%M")

	mkdir -p var/archive/$DATENOW
	mv var/log/* var/archive/$DATENOW/

	let COUNTER=COUNTER+1 
done
