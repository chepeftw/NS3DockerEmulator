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
TIME=$2

COUNTER=0
while [  $COUNTER -lt $NUMBER ]; do
	
	let SEGMENT3=(COUNTER/250)
	let SEGMENT4=(COUNTER%250)+1
	
	echo ""
	echo ""
	echo "python main.py -n $NUMBER -t $TIME -o full -r 10.12.$SEGMENT3.$SEGMENT4"
	echo ""
	echo ""
	python main.py -n $NUMBER -t $TIME -o full -r 10.12.$SEGMENT3.$SEGMENT4

	let COUNTER=COUNTER+1 
done