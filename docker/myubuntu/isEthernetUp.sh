#!/bin/bash

ETH0=$(ifconfig -a | grep eth0 | wc -l)

while [ $ETH0 -eq 0 ]
do
  sleep 5
  ETH0=$(ifconfig -a | grep eth0 | wc -l)
done

sleep 3
olsrd -i eth0