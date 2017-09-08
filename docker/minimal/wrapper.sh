#!/usr/bin/env sh

ETH0=$(ip a | grep eth0 | wc -l) # This is for ALPINE

while [ $ETH0 -eq 0 ]
do
  echo "waiting ... "
  sleep 2
  ETH0=$(ip a | grep eth0 | wc -l)
done

mkdir -p /var/log/golang
echo "START" > /var/log/golang/wrapper.log

echo "starting ... "
echo "---------------------------------------------"


# Start the first process
echo "Starting Beacon ... " >> /var/log/golang/wrapper.log
echo "/beacon /beacon_conf/conf1.yml &" >> /var/log/golang/wrapper.log

/beacon /beacon_conf/conf1.yml &
status=$?
if [ $status -ne 0 ]; then
  echo "Failed to start beacon process: $status"
  exit $status
fi

# Naive check runs checks once a minute to see if either of the processes exited.
# This illustrates part of the heavy lifting you need to do if you want to run
# more than one service in a container. The container will exit with an error
# if it detects that either of the processes has exited.
# Otherwise it will loop forever, waking up every 60 seconds

while /bin/true; do

  ps aux | grep beacon | grep -v grep
  P1_STATUS=$?

  echo "PROCESS1 STATUS = $P1_STATUS " >> /var/log/golang/wrapper.log

  # If the greps above find anything, they will exit with 0 status
  # If they are not both 0, then something is wrong
  if [ $P1_STATUS -ne 0 ]; then
    echo "One of the processes has already exited."
    exit -1
  fi
  sleep 60
done