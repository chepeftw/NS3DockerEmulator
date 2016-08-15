#!/bin/sh

# This file basically destroy the network bridge and TAP interface
# created by the singleSetup.sh script

if [ -z "$1" ]
  then
    echo "No name supplied"
    exit 1
fi

NAME=$1

ifconfig br-$NAME down

brctl delif br-$NAME tap-$NAME

brctl delbr br-$NAME

ifconfig tap-$NAME down

tunctl -d tap-$NAME