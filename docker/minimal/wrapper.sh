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
echo "Starting Router ... " >> /var/log/golang/wrapper.log
echo "/router /btc_conf/conf.yml &" >> /var/log/golang/wrapper.log

/router /btc_conf/conf.yml &
status=$?
if [ $status -ne 0 ]; then
  echo "Failed to start router process: $status"
  exit $status
fi

# Start the second process
echo "Starting Miner ... " >> /var/log/golang/wrapper.log
echo "/miner /btc_conf/conf.yml &" >> /var/log/golang/wrapper.log

/miner /btc_conf/conf.yml &
status=$?
if [ $status -ne 0 ]; then
  echo "Failed to start miner process: $status"
  exit $status
fi

# Start the third process
echo "Starting Blockchain (Monitor) ... " >> /var/log/golang/wrapper.log
echo "/blockchain /btc_conf/conf.yml &" >> /var/log/golang/wrapper.log

/blockchain /btc_conf/conf.yml &
status=$?
if [ $status -ne 0 ]; then
  echo "Failed to start blockchain process: $status"
  exit $status
fi

# Naive check runs checks once a minute to see if either of the processes exited.
# This illustrates part of the heavy lifting you need to do if you want to run
# more than one service in a container. The container will exit with an error
# if it detects that either of the processes has exited.
# Otherwise it will loop forever, waking up every 60 seconds

sleep 2

while /bin/true; do

  ps aux | grep router | grep -v grep
  P1_STATUS=$?

  ps aux | grep miner | grep -v grep
  P2_STATUS=$?

  ps aux | grep blockchain | grep -v grep
  P3_STATUS=$?

  echo "PROCESS1 STATUS = $P1_STATUS |"
  echo "PROCESS2 STATUS = $P2_STATUS |"
  echo "PROCESS3 STATUS = $P3_STATUS |"

  echo "PROCESS1 STATUS = $P1_STATUS " >> /var/log/golang/wrapper.log
  echo "PROCESS2 STATUS = $P2_STATUS " >> /var/log/golang/wrapper.log
  echo "PROCESS3 STATUS = $P3_STATUS " >> /var/log/golang/wrapper.log

  # If the greps above find anything, they will exit with 0 status
  # If they are not both 0, then something is wrong
  if [ $P1_STATUS -ne 0 ]; then
    echo "Router has already exited."
    exit -1
  fi
  if [ $P2_STATUS -ne 0 ]; then
    echo "Miner has already exited."
    exit -1
  fi
  if [ $P3_STATUS -ne 0 ]; then
    echo "Blockchain has already exited."
    exit -1
  fi

  sleep 60
done